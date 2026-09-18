# Behavioral Equivalence Verification

This test suite implements automated behavioral equivalence verification for `halflife-refactored`, ensuring that internal refactoring maintains exact gameplay behavior, binary ABI compatibility, and savegame/level-transition serialization.

## Overview of Verification Layers

### Layer 1 — Gameplay Constants Audit (`#102`)
- **Script**: [`tests/verify_constants.py`](file:///c:/code/halflife-refactored/tests/verify_constants.py)
- **Golden Baseline**: [`tests/golden/constants.json`](file:///c:/code/halflife-refactored/tests/golden/constants.json)
- **Scope**:
  - Weapon definitions (IDs, auto-switch weights, clip sizes, ammo capacities, default ammo given, ammo pickup quantities, item flags, bullet enum ordinals)
  - Dispersion cone vectors (`VECTOR_CONE_*DEGREES`)
  - HUD definitions & limits (`cdll_dll.h`)
  - Skill levels and `skilldata_t` struct field ordering (`skill.h`)
  - Damage types and duration constants (`damage_defs.h`)
  - Observer modes (`pm_shared.h`)
  - DLL exports (`hl.def`)
  - Entity factory string bindings (`LINK_ENTITY_TO_CLASS` across `dlls/`)
- **How to run**:
  ```bash
  python tests/verify_constants.py --golden tests/golden/constants.json
  ```
- **How to update**:
  ```bash
  python tests/verify_constants.py --update
  ```

---

### Layer 2A — ABI Struct Layout Verification (`#103`)
- **Implementation**: [`tests/test_abi_layout.cpp`](file:///c:/code/halflife-refactored/tests/test_abi_layout.cpp)
- **Scope**:
  - Validates exact byte sizes (`sizeof`) and field offsets (`offsetof`) for all structures crossing the engine/DLL and client/server boundary:
    - `entvars_t`, `edict_t`, `globalvars_t`, `enginefuncs_t`, `DLL_FUNCTIONS`, `NEW_DLL_FUNCTIONS`
    - `playermove_t`, `usercmd_t`, `ItemInfo`, `TraceResult`, `KeyValueData`
    - `physent_t`, `pmtrace_t`, `netadr_t`, `clientdata_t`, `weapon_data_t`, `entity_state_t`, `movevars_t`
- **Execution**:
  - Evaluated entirely at compile-time via `static_assert`.
  - Built automatically as part of normal Windows MSBuild and Linux Makefile targets.
  - Zero runtime overhead.

---

### Layer 2B — Save/Restore Tables Verification (`#101`)
- **Script**: [`tests/verify_saverestore.py`](file:///c:/code/halflife-refactored/tests/verify_saverestore.py)
- **Golden Baseline**: [`tests/golden/saverestore_baseline.json`](file:///c:/code/halflife-refactored/tests/golden/saverestore_baseline.json)
- **Compile-Time Checks**: [`tests/test_saverestore.cpp`](file:///c:/code/halflife-refactored/tests/test_saverestore.cpp)
- **Scope**:
  - Verifies `TYPEDESCRIPTION` tables for player (`CBasePlayer::m_playerSaveData`), weapons (`CBasePlayerItem`, `CBasePlayerWeapon`, `CGauss`, `CEgon`, `CRpg`, `CSatchel`, `CShotgun`, `CWeaponBox`), projectiles (`CHornet`, `CSqueakGrenade`, `CRpgRocket`, `CTripmineGrenade`), and entity base classes (`CBaseEntity`, `CBaseDelay`, `CBaseAnimating`, `CBaseToggle`, `gEntvarsDescription`).
  - Ensures field count, names, serialization types, and array dimensions match expected baselines.
  - Compile-time assertions check exact member memory offsets.
- **How to run**:
  ```bash
  python tests/verify_saverestore.py --golden tests/golden/saverestore_baseline.json
  ```
- **How to update**:
  ```bash
  python tests/verify_saverestore.py --update
  ```
