#!/usr/bin/env python3
"""Verify gameplay-critical constants against a golden snapshot.

This script parses #define macros, enum values, struct field orders, DLL exports,
and entity classname bindings from Half-Life SDK source and header files,
comparing them against a checked-in golden JSON file. It exits with a non-zero
status if any constant value has changed, been removed, or if an expected section
is missing.

Usage:
    python verify_constants.py [--golden PATH] [--repo-root PATH]

To update the golden file after an intentional constant change:
    1. Make the change in the source header.
    2. Run: python verify_constants.py --update
    3. Review the diff in tests/golden/constants.json
    4. Commit both the header change and the updated golden file.
"""

import argparse
import json
import os
import re
import sys
from pathlib import Path


def find_repo_root(start: Path) -> Path:
    """Walk up from *start* until we find a directory containing AGENTS.md."""
    current = start.resolve()
    while current != current.parent:
        if (current / "AGENTS.md").exists():
            return current
        current = current.parent
    # Fallback: assume script is in tests/
    return start.parent


def parse_defines(filepath: Path) -> dict[str, int | float | list[float]]:
    """Extract #define NAME VALUE pairs from a C/C++ header file.

    Handles integer literals, hex, bit-shift expressions like (1 << N),
    negative values, float literals, Vector(...) constructors, and
    references to other defines within the same file.
    """
    if not filepath.exists():
        print(f"  WARNING: File not found: {filepath}", file=sys.stderr)
        return {}

    defines: dict[str, int | float | list[float]] = {}
    content = filepath.read_text(encoding="utf-8", errors="replace")

    # Match lines like: #define NAME VALUE (avoiding multiline newline capture)
    pattern = re.compile(
        r"^[ \t]*#[ \t]*define[ \t]+([a-zA-Z_]\w*)[ \t]+([^\r\n]+)$", re.MULTILINE
    )

    for match in pattern.finditer(content):
        name = match.group(1)
        raw_value = match.group(2).strip()

        # Strip trailing C/C++ comments: // ... or /* ... */
        raw_value = re.sub(r"\s*//.*$", "", raw_value).strip()
        raw_value = re.sub(r"\s*/\*.*?\*/", "", raw_value).strip()

        value = _try_evaluate(raw_value, defines)
        if value is not None:
            defines[name] = value

    return defines


def parse_enum_values(filepath: Path) -> dict[str, int]:
    """Extract enum member values from C-style enum blocks."""
    if not filepath.exists():
        return {}

    content = filepath.read_text(encoding="utf-8", errors="replace")

    results: dict[str, int] = {}
    enum_pattern = re.compile(
        r"(?:typedef\s+)?enum(?:\s+\w+)?\s*\{([^}]+)\}", re.DOTALL
    )

    for enum_match in enum_pattern.finditer(content):
        body = enum_match.group(1)
        current_value = 0
        for line in body.split("\n"):
            # Strip comments first
            line = re.sub(r"//.*$", "", line)
            line = re.sub(r"/\*.*?\*/", "", line)
            line = line.strip().rstrip(",").strip()
            if not line:
                continue

            if "=" in line:
                parts = line.split("=", 1)
                name = parts[0].strip()
                val_str = parts[1].strip()
                val = _try_evaluate(val_str, results)
                if val is not None:
                    int_val = int(val)
                    results[name] = int_val
                    current_value = int_val + 1
            else:
                name = line.strip()
                if name and name.isidentifier():
                    results[name] = current_value
                    current_value += 1

    return results


def parse_entity_classnames(repo_root: Path) -> dict[str, str]:
    """Extract all LINK_ENTITY_TO_CLASS(map_name, dll_class) mappings."""
    dlls_dir = repo_root / "dlls"
    if not dlls_dir.exists():
        return {}

    entity_pattern = re.compile(
        r"LINK_ENTITY_TO_CLASS\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)"
    )
    entities: dict[str, str] = {}

    for p in sorted(dlls_dir.rglob("*.cpp")):
        content = p.read_text(encoding="utf-8", errors="replace")
        for match in entity_pattern.finditer(content):
            entities[match.group(1)] = match.group(2)

    return dict(sorted(entities.items()))


def _try_evaluate(
    raw_value: str, known: dict[str, int | float | list[float]]
) -> int | float | list[float] | None:
    """Try to evaluate a C preprocessor constant expression."""
    raw_value = raw_value.strip()

    # Remove wrapping parentheses if balanced
    while raw_value.startswith("(") and raw_value.endswith(")"):
        inner = raw_value[1:-1]
        depth = 0
        balanced = True
        for ch in inner:
            if ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
            if depth < 0:
                balanced = False
                break
        if balanced and depth == 0:
            raw_value = inner.strip()
        else:
            break

    # Integer literal
    if re.fullmatch(r"-?\d+", raw_value):
        return int(raw_value)

    # Hex literal
    if re.fullmatch(r"0[xX][0-9a-fA-F]+", raw_value):
        return int(raw_value, 16)

    # Float literal
    if re.fullmatch(r"-?\d+\.\d+f?", raw_value):
        return float(raw_value.rstrip("f"))

    # Bit shift: (1 << N) or 1 << N
    shift_match = re.fullmatch(r"(\d+)\s*<<\s*(\d+)", raw_value)
    if shift_match:
        return int(shift_match.group(1)) << int(shift_match.group(2))

    # Vector(x, y, z)
    vec_match = re.fullmatch(
        r"Vector\s*\(\s*([-\d.eE]+)f?\s*,\s*([-\d.eE]+)f?\s*,\s*([-\d.eE]+)f?\s*\)",
        raw_value,
    )
    if vec_match:
        return [
            float(vec_match.group(1)),
            float(vec_match.group(2)),
            float(vec_match.group(3)),
        ]

    # Reference to another define
    if raw_value in known:
        return known[raw_value]

    # Negative reference
    neg_match = re.fullmatch(r"-\s*(\w+)", raw_value)
    if neg_match and neg_match.group(1) in known:
        val = known[neg_match.group(1)]
        if isinstance(val, (int, float)):
            return -val

    # Bitwise NOT with hex: ~(0xff003fff) or ~( 1 << WEAPON_SUIT )
    not_match = re.fullmatch(r"~\s*\(\s*(0[xX][0-9a-fA-F]+)\s*\)", raw_value)
    if not_match:
        val = int(not_match.group(1), 16)
        return (~val) & 0xFFFFFFFF

    not_shift = re.fullmatch(r"~\s*\(\s*1\s*<<\s*(\w+)\s*\)", raw_value)
    if not_shift:
        shift_name = not_shift.group(1)
        if shift_name in known and isinstance(known[shift_name], int):
            shift_amt = known[shift_name]
            return (~(1 << shift_amt)) & 0xFFFFFFFF

    return None


def verify_section(section_name: str, golden: dict, actual: dict) -> list[str]:
    """Compare a golden section dict against actual parsed values."""
    errors: list[str] = []

    for const_name, expected_value in golden.items():
        if const_name.startswith("_"):
            continue  # Skip metadata keys

        if const_name not in actual:
            errors.append(
                f"  MISSING: {const_name} (expected {expected_value}) not found in parsed output"
            )
            continue

        actual_value = actual[const_name]

        # Vector comparison
        if isinstance(expected_value, list):
            if not isinstance(actual_value, list) or len(expected_value) != len(actual_value):
                errors.append(
                    f"  CHANGED: {const_name} = {actual_value} (expected {expected_value})"
                )
            else:
                mismatch = any(
                    abs(float(e) - float(a)) > 1e-5
                    for e, a in zip(expected_value, actual_value)
                )
                if mismatch:
                    errors.append(
                        f"  CHANGED: {const_name} = {actual_value} (expected {expected_value})"
                    )
        # Float comparison
        elif isinstance(expected_value, float) or isinstance(actual_value, float):
            if abs(float(expected_value) - float(actual_value)) > 1e-6:
                errors.append(
                    f"  CHANGED: {const_name} = {actual_value} (expected {expected_value})"
                )
        # String comparison
        elif isinstance(expected_value, str):
            if str(expected_value) != str(actual_value):
                errors.append(
                    f"  CHANGED: {const_name} = {actual_value} (expected {expected_value})"
                )
        # Integer comparison
        else:
            if int(expected_value) != int(actual_value):
                errors.append(
                    f"  CHANGED: {const_name} = {actual_value} (expected {expected_value})"
                )

    return errors


def verify_field_order(section_name: str, golden_order: list[str], filepath: Path) -> list[str]:
    """Verify struct field ordering in a header file."""
    errors: list[str] = []

    if not filepath.exists():
        errors.append(f"  File not found: {filepath}")
        return errors

    content = filepath.read_text(encoding="utf-8", errors="replace")

    field_pattern = re.compile(r"^\s+(?:float|int)\s+(\w+)\s*;", re.MULTILINE)
    actual_fields = [m.group(1) for m in field_pattern.finditer(content)]

    if not actual_fields:
        errors.append(f"  Could not parse struct fields from {filepath}")
        return errors

    if actual_fields != golden_order:
        for i, expected_field in enumerate(golden_order):
            if i >= len(actual_fields):
                errors.append(
                    f"  MISSING FIELD at position {i}: expected '{expected_field}'"
                )
            elif actual_fields[i] != expected_field:
                errors.append(
                    f"  FIELD ORDER CHANGED at position {i}: found '{actual_fields[i]}', expected '{expected_field}'"
                )

        if len(actual_fields) > len(golden_order):
            for i in range(len(golden_order), len(actual_fields)):
                errors.append(
                    f"  EXTRA FIELD at position {i}: '{actual_fields[i]}'"
                )

    return errors


def verify_exports(golden_exports: dict, filepath: Path) -> list[str]:
    """Verify DLL export definitions."""
    errors: list[str] = []

    if not filepath.exists():
        errors.append(f"  File not found: {filepath}")
        return errors

    content = filepath.read_text(encoding="utf-8", errors="replace")

    for export_name, ordinal in golden_exports.items():
        pattern = rf"\b{re.escape(export_name)}\b.*@{ordinal}\b"
        if not re.search(pattern, content):
            errors.append(
                f"  MISSING EXPORT: {export_name} at ordinal @{ordinal}"
            )

    return errors


def build_golden_snapshot(repo_root: Path) -> dict:
    """Collect all current constants from the repository to generate the golden file."""
    # weapon_defs.h
    weapon_defs_path = repo_root / "dlls" / "weapons" / "weapon_defs.h"
    wdefs_defines = parse_defines(weapon_defs_path)
    wdefs_enums = parse_enum_values(weapon_defs_path)
    wdefs_all = {**wdefs_defines, **wdefs_enums}

    # cdll_dll.h
    cdll_path = repo_root / "dlls" / "core" / "cdll_dll.h"
    cdll_defines = parse_defines(cdll_path)

    # skill.h
    skill_path = repo_root / "dlls" / "core" / "skill.h"
    skill_defines = parse_defines(skill_path)
    skill_content = skill_path.read_text(encoding="utf-8", errors="replace")
    skill_fields = [
        m.group(1)
        for m in re.finditer(r"^\s+(?:float|int)\s+(\w+)\s*;", skill_content, re.MULTILINE)
    ]

    # damage_defs.h
    damage_path = repo_root / "game_shared" / "damage_defs.h"
    damage_defines = parse_defines(damage_path)

    # pm_shared.h
    pm_shared_path = repo_root / "pm_shared" / "pm_shared.h"
    pm_defines = parse_defines(pm_shared_path)

    # entities
    entities = parse_entity_classnames(repo_root)

    # Filter into categorized dicts
    golden = {
        "_description": "Golden snapshot of gameplay-critical constants. Any change to these values breaks gameplay or protocol compatibility.",
        "_version": 1,
        "weapon_defs.h": {
            "_file": "dlls/weapons/weapon_defs.h",
            "item_constants": {
                k: wdefs_all[k] for k in ["ITEM_HEALTHKIT", "ITEM_ANTIDOTE", "ITEM_SECURITY", "ITEM_BATTERY"] if k in wdefs_all
            },
            "weapon_ids": {
                k: wdefs_all[k] for k in [
                    "WEAPON_NONE", "WEAPON_CROWBAR", "WEAPON_GLOCK", "WEAPON_PYTHON", "WEAPON_MP5",
                    "WEAPON_CHAINGUN", "WEAPON_CROSSBOW", "WEAPON_SHOTGUN", "WEAPON_RPG", "WEAPON_GAUSS",
                    "WEAPON_EGON", "WEAPON_HORNETGUN", "WEAPON_HANDGRENADE", "WEAPON_TRIPMINE",
                    "WEAPON_SATCHEL", "WEAPON_SNARK", "WEAPON_SUIT", "MAX_WEAPONS"
                ] if k in wdefs_all
            },
            "weapon_weights": {
                k: wdefs_all[k] for k in [
                    "CROWBAR_WEIGHT", "GLOCK_WEIGHT", "PYTHON_WEIGHT", "MP5_WEIGHT", "SHOTGUN_WEIGHT",
                    "CROSSBOW_WEIGHT", "RPG_WEIGHT", "GAUSS_WEIGHT", "EGON_WEIGHT", "HORNETGUN_WEIGHT",
                    "HANDGRENADE_WEIGHT", "SNARK_WEIGHT", "SATCHEL_WEIGHT", "TRIPMINE_WEIGHT"
                ] if k in wdefs_all
            },
            "ammo_max_carry": {
                k: wdefs_all[k] for k in [
                    "URANIUM_MAX_CARRY", "_9MM_MAX_CARRY", "_357_MAX_CARRY", "BUCKSHOT_MAX_CARRY",
                    "BOLT_MAX_CARRY", "ROCKET_MAX_CARRY", "HANDGRENADE_MAX_CARRY", "SATCHEL_MAX_CARRY",
                    "TRIPMINE_MAX_CARRY", "SNARK_MAX_CARRY", "HORNET_MAX_CARRY", "M203_GRENADE_MAX_CARRY"
                ] if k in wdefs_all
            },
            "weapon_clips": {
                k: wdefs_all[k] for k in [
                    "WEAPON_NOCLIP", "GLOCK_MAX_CLIP", "PYTHON_MAX_CLIP", "MP5_MAX_CLIP",
                    "MP5_DEFAULT_AMMO", "SHOTGUN_MAX_CLIP", "CROSSBOW_MAX_CLIP", "RPG_MAX_CLIP",
                    "GAUSS_MAX_CLIP", "EGON_MAX_CLIP", "HORNETGUN_MAX_CLIP", "HANDGRENADE_MAX_CLIP",
                    "SATCHEL_MAX_CLIP", "TRIPMINE_MAX_CLIP", "SNARK_MAX_CLIP"
                ] if k in wdefs_all
            },
            "weapon_default_give": {
                k: wdefs_all[k] for k in [
                    "GLOCK_DEFAULT_GIVE", "PYTHON_DEFAULT_GIVE", "MP5_DEFAULT_GIVE",
                    "MP5_M203_DEFAULT_GIVE", "SHOTGUN_DEFAULT_GIVE", "CROSSBOW_DEFAULT_GIVE",
                    "RPG_DEFAULT_GIVE", "GAUSS_DEFAULT_GIVE", "EGON_DEFAULT_GIVE",
                    "HANDGRENADE_DEFAULT_GIVE", "SATCHEL_DEFAULT_GIVE", "TRIPMINE_DEFAULT_GIVE",
                    "SNARK_DEFAULT_GIVE", "HIVEHAND_DEFAULT_GIVE"
                ] if k in wdefs_all
            },
            "ammo_pickup_give": {
                k: wdefs_all[k] for k in [
                    "AMMO_URANIUMBOX_GIVE", "AMMO_GLOCKCLIP_GIVE", "AMMO_357BOX_GIVE",
                    "AMMO_MP5CLIP_GIVE", "AMMO_CHAINBOX_GIVE", "AMMO_M203BOX_GIVE",
                    "AMMO_BUCKSHOTBOX_GIVE", "AMMO_CROSSBOWCLIP_GIVE", "AMMO_RPGCLIP_GIVE",
                    "AMMO_SNARKBOX_GIVE"
                ] if k in wdefs_all
            },
            "bullet_types": {
                k: wdefs_all[k] for k in [
                    "BULLET_NONE", "BULLET_PLAYER_9MM", "BULLET_PLAYER_MP5", "BULLET_PLAYER_357",
                    "BULLET_PLAYER_BUCKSHOT", "BULLET_PLAYER_CROWBAR", "BULLET_MONSTER_9MM",
                    "BULLET_MONSTER_MP5", "BULLET_MONSTER_12MM"
                ] if k in wdefs_all
            },
            "item_flags": {
                k: wdefs_all[k] for k in [
                    "ITEM_FLAG_SELECTONEMPTY", "ITEM_FLAG_NOAUTORELOAD",
                    "ITEM_FLAG_NOAUTOSWITCHEMPTY", "ITEM_FLAG_LIMITINWORLD",
                    "ITEM_FLAG_EXHAUSTIBLE", "ITEM_FLAG_NOAUTOSWITCHTO"
                ] if k in wdefs_all
            },
            "sound_constants": {
                k: wdefs_all[k] for k in [
                    "LOUD_GUN_VOLUME", "NORMAL_GUN_VOLUME", "QUIET_GUN_VOLUME",
                    "BRIGHT_GUN_FLASH", "NORMAL_GUN_FLASH", "DIM_GUN_FLASH",
                    "BIG_EXPLOSION_VOLUME", "NORMAL_EXPLOSION_VOLUME",
                    "SMALL_EXPLOSION_VOLUME", "WEAPON_ACTIVITY_VOLUME"
                ] if k in wdefs_all
            },
            "dispersion_cones": {
                k: wdefs_all[k] for k in [
                    "VECTOR_CONE_1DEGREES", "VECTOR_CONE_2DEGREES", "VECTOR_CONE_3DEGREES",
                    "VECTOR_CONE_4DEGREES", "VECTOR_CONE_5DEGREES", "VECTOR_CONE_6DEGREES",
                    "VECTOR_CONE_7DEGREES", "VECTOR_CONE_8DEGREES", "VECTOR_CONE_9DEGREES",
                    "VECTOR_CONE_10DEGREES", "VECTOR_CONE_15DEGREES", "VECTOR_CONE_20DEGREES"
                ] if k in wdefs_all
            },
            "weapon_misc": {
                k: wdefs_all[k] for k in ["WEAPON_IS_ONTARGET", "MAX_NORMAL_BATTERY"] if k in wdefs_all
            }
        },
        "cdll_dll.h": {
            "_file": "dlls/core/cdll_dll.h",
            "hud_constants": {
                k: cdll_defines[k] for k in [
                    "MAX_WEAPONS", "MAX_WEAPON_SLOTS", "MAX_ITEM_TYPES", "MAX_ITEMS",
                    "HIDEHUD_WEAPONS", "HIDEHUD_FLASHLIGHT", "HIDEHUD_ALL", "HIDEHUD_HEALTH",
                    "MAX_AMMO_TYPES", "MAX_AMMO_SLOTS", "HUD_PRINTNOTIFY", "HUD_PRINTCONSOLE",
                    "HUD_PRINTTALK", "HUD_PRINTCENTER", "WEAPON_SUIT"
                ] if k in cdll_defines
            }
        },
        "skill.h": {
            "_file": "dlls/core/skill.h",
            "skill_levels": {
                k: skill_defines[k] for k in ["SKILL_EASY", "SKILL_MEDIUM", "SKILL_HARD"] if k in skill_defines
            },
            "skilldata_t_field_order": skill_fields
        },
        "damage_defs.h": {
            "_file": "game_shared/damage_defs.h",
            "damage_types": {
                k: damage_defines[k] for k in [
                    "DMG_GENERIC", "DMG_CRUSH", "DMG_BULLET", "DMG_SLASH", "DMG_BURN",
                    "DMG_FREEZE", "DMG_FALL", "DMG_BLAST", "DMG_CLUB", "DMG_SHOCK",
                    "DMG_SONIC", "DMG_ENERGYBEAM", "DMG_NEVERGIB", "DMG_ALWAYSGIB",
                    "DMG_DROWN", "DMG_PARALYZE", "DMG_NERVEGAS", "DMG_POISON",
                    "DMG_RADIATION", "DMG_DROWNRECOVER", "DMG_ACID", "DMG_SLOWBURN",
                    "DMG_SLOWFREEZE", "DMG_MORTAR"
                ] if k in damage_defines
            },
            "damage_durations": {
                k: damage_defines[k] for k in [
                    "PARALYZE_DURATION", "PARALYZE_DAMAGE", "NERVEGAS_DURATION",
                    "NERVEGAS_DAMAGE", "POISON_DURATION", "POISON_DAMAGE",
                    "RADIATION_DURATION", "RADIATION_DAMAGE", "ACID_DURATION",
                    "ACID_DAMAGE", "SLOWBURN_DURATION", "SLOWBURN_DAMAGE",
                    "SLOWFREEZE_DURATION", "SLOWFREEZE_DAMAGE"
                ] if k in damage_defines
            }
        },
        "pm_shared.h": {
            "_file": "pm_shared/pm_shared.h",
            "observer_modes": {
                k: pm_defines[k] for k in [
                    "OBS_NONE", "OBS_CHASE_LOCKED", "OBS_CHASE_FREE", "OBS_ROAMING",
                    "OBS_IN_EYE", "OBS_MAP_FREE", "OBS_MAP_CHASE"
                ] if k in pm_defines
            }
        },
        "hl.def": {
            "_file": "dlls/hl.def",
            "exports": {
                "GiveFnptrsToDll": 1
            }
        },
        "entity_classnames": {
            "_description": "All LINK_ENTITY_TO_CLASS registrations across dlls/",
            "classes": entities
        }
    }
    return golden


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Verify gameplay constants against golden snapshot"
    )
    parser.add_argument(
        "--golden",
        default=None,
        help="Path to golden constants JSON file",
    )
    parser.add_argument(
        "--repo-root",
        default=None,
        help="Path to repository root",
    )
    parser.add_argument(
        "--update",
        action="store_true",
        help="Update the golden file with current values (use after intentional changes)",
    )
    args = parser.parse_args()

    # Determine paths
    script_dir = Path(__file__).parent
    repo_root = Path(args.repo_root) if args.repo_root else find_repo_root(script_dir)
    golden_path = (
        Path(args.golden)
        if args.golden
        else (repo_root / "tests" / "golden" / "constants.json")
    )

    if args.update:
        print(f"Updating golden constants file at: {golden_path}")
        golden_data = build_golden_snapshot(repo_root)
        golden_path.parent.mkdir(parents=True, exist_ok=True)
        with open(golden_path, "w", encoding="utf-8") as f:
            json.dump(golden_data, f, indent=2)
            f.write("\n")
        print(f"Successfully wrote updated golden constants to {golden_path}")
        return 0

    if not golden_path.exists():
        print(f"ERROR: Golden file not found: {golden_path}", file=sys.stderr)
        return 1

    with open(golden_path, "r", encoding="utf-8") as f:
        golden = json.load(f)

    all_errors: list[str] = []
    sections_checked = 0

    # --- weapon_defs.h ---
    weapon_defs_path = repo_root / "dlls" / "weapons" / "weapon_defs.h"
    if "weapon_defs.h" in golden:
        print(f"Checking weapon_defs.h ...")
        parsed_defines = parse_defines(weapon_defs_path)
        parsed_enums = parse_enum_values(weapon_defs_path)
        all_parsed = {**parsed_defines, **parsed_enums}

        wdefs = golden["weapon_defs.h"]
        for section_name, section_data in wdefs.items():
            if section_name.startswith("_"):
                continue
            if not isinstance(section_data, dict):
                continue
            errors = verify_section(section_name, section_data, all_parsed)
            if errors:
                all_errors.append(f"weapon_defs.h / {section_name}:")
                all_errors.extend(errors)
            sections_checked += 1

    # --- cdll_dll.h ---
    cdll_path = repo_root / "dlls" / "core" / "cdll_dll.h"
    if "cdll_dll.h" in golden:
        print(f"Checking cdll_dll.h ...")
        parsed = parse_defines(cdll_path)

        for section_name, section_data in golden["cdll_dll.h"].items():
            if section_name.startswith("_"):
                continue
            if not isinstance(section_data, dict):
                continue
            errors = verify_section(section_name, section_data, parsed)
            if errors:
                all_errors.append(f"cdll_dll.h / {section_name}:")
                all_errors.extend(errors)
            sections_checked += 1

    # --- skill.h ---
    skill_path = repo_root / "dlls" / "core" / "skill.h"
    if "skill.h" in golden:
        print(f"Checking skill.h ...")
        parsed = parse_defines(skill_path)
        skill_golden = golden["skill.h"]

        if "skill_levels" in skill_golden:
            errors = verify_section("skill_levels", skill_golden["skill_levels"], parsed)
            if errors:
                all_errors.append("skill.h / skill_levels:")
                all_errors.extend(errors)
            sections_checked += 1

        if "skilldata_t_field_order" in skill_golden:
            errors = verify_field_order(
                "skilldata_t", skill_golden["skilldata_t_field_order"], skill_path
            )
            if errors:
                all_errors.append("skill.h / skilldata_t field order:")
                all_errors.extend(errors)
            sections_checked += 1

    # --- damage_defs.h ---
    damage_path = repo_root / "game_shared" / "damage_defs.h"
    if "damage_defs.h" in golden:
        print(f"Checking damage_defs.h ...")
        parsed = parse_defines(damage_path)

        for section_name, section_data in golden["damage_defs.h"].items():
            if section_name.startswith("_"):
                continue
            if not isinstance(section_data, dict):
                continue
            errors = verify_section(section_name, section_data, parsed)
            if errors:
                all_errors.append(f"damage_defs.h / {section_name}:")
                all_errors.extend(errors)
            sections_checked += 1

    # --- pm_shared.h ---
    pm_shared_path = repo_root / "pm_shared" / "pm_shared.h"
    if "pm_shared.h" in golden:
        print(f"Checking pm_shared.h ...")
        parsed = parse_defines(pm_shared_path)

        for section_name, section_data in golden["pm_shared.h"].items():
            if section_name.startswith("_"):
                continue
            if not isinstance(section_data, dict):
                continue
            errors = verify_section(section_name, section_data, parsed)
            if errors:
                all_errors.append(f"pm_shared.h / {section_name}:")
                all_errors.extend(errors)
            sections_checked += 1

    # --- hl.def ---
    hldef_path = repo_root / "dlls" / "hl.def"
    if "hl.def" in golden:
        print(f"Checking hl.def ...")
        errors = verify_exports(golden["hl.def"]["exports"], hldef_path)
        if errors:
            all_errors.append("hl.def / exports:")
            all_errors.extend(errors)
        sections_checked += 1

    # --- entity_classnames ---
    if "entity_classnames" in golden:
        print(f"Checking entity_classnames ...")
        parsed_entities = parse_entity_classnames(repo_root)
        errors = verify_section("entity_classnames", golden["entity_classnames"]["classes"], parsed_entities)
        if errors:
            all_errors.append("entity_classnames:")
            all_errors.extend(errors)
        sections_checked += 1

    # --- Summary ---
    print(f"\nChecked {sections_checked} sections.")

    if all_errors:
        print(f"\nFAILED: {len(all_errors)} error(s) found:\n")
        for error in all_errors:
            print(error)
        print(
            "\nIf these changes are intentional, update the golden file:\n"
            "  python tests/verify_constants.py --update"
        )
        return 1
    else:
        print("PASSED: All gameplay constants match golden snapshot.")
        return 0


if __name__ == "__main__":
    sys.exit(main())
