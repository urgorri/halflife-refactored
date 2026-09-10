# Requirements Document: Codebase-Wide Code Smell Remediation

## Introduction

This project defines the comprehensive requirements for systematically eliminating **all 236 remaining code smells** across the active Half-Life GoldSrc DLL codebase (`dlls/`, `cl_dll/`, `pm_shared/`, and `game_shared/`). The overarching objective is to achieve a 100% clean code smell baseline according to the automated technical debt scanner while strictly preserving binary compatibility, save/restore tables, network protocols, and runtime game behavior.

### Target Subsystems and Scope

The active scope encompasses 117 files across 7 major architectural domains:
1. **High-Leverage Deep Nesting Sites**: 11 critical routines with >= 6 levels of control nesting across AI, player core, rendering, client weapons, and bot management.
2. **Monster & Ally AI Subsystem** (`dlls/monsters/`, `dlls/ai/`): 57 smells across 26 files (Barney, Scientist, SquadMonster, MonsterScheduler, Schedule, PathCorner, etc.).
3. **Gameplay, World, & Interactive Systems** (`dlls/gameplay/`, `dlls/world/`, `dlls/systems/`): 36 smells across 15 files (`scripted.cpp`, `world.cpp`, `trains.cpp`, `sound_sentences.cpp`, `func_break.cpp`, `func_tank.cpp`, etc.).
4. **Core Server & Weapon Subsystems** (`dlls/core/`, `dlls/weapons/`): 39 smells across 21 files (`player_physics.cpp`, `util.cpp`, `animation.cpp`, `cbase.cpp`, `combat_damage.cpp`, `player.cpp`, `player_inventory.cpp`, `weapon_base.cpp`, weapons, etc.).
5. **Client DLL Subsystems** (`cl_dll/`): 44 smells across 24 files (`inputw32.cpp`, `menu.cpp`, `hl_weapons.cpp`, `studio_render_bones.cpp`, `health.cpp`, `in_camera.cpp`, `view.cpp`, `status_icons.cpp`, etc.).
6. **Shared Movement, Performance Counters & Bot Navigation** (`pm_shared/`, `game_shared/`): 49 smells across 20 files (`pm_move_water.c`, `pm_math.c`, `pm_step.c`, `perf_counter.h`, `bot_util.cpp`, `nav_area_connect.cpp`, `nav_area_tactical.cpp`, `nav_area.h`, etc.).

---

## Glossary

- **GoldSrc DLL**: The core game dynamic link library set comprising the server library (`hl.dll`) and the client library (`client.dll`).
- **Code Smell**: Surface indicators in source code that signal maintainability, readability, or structural decay (such as deep control flow nesting, dead legacy comments, obsolete temporary tags, and copy-paste duplication).
- **Control Flow Nesting**: The depth of hierarchical conditional (`if`, `else`), iterative (`for`, `while`, `do`), and selection (`switch`, `case`) statements. A depth >= 6 severely degrades cognitive readability and testing branch coverage.
- **Guard Clause**: A conditional check at the beginning of a function or loop that immediately returns, breaks, or continues if preconditions are not met, preventing excessive indentation of the main execution path.
- **Edict (`edict_t`)**: The engine-level container representing an active entity slot in the world.
- **Save/Restore Table (`TYPEDESCRIPTION`)**: Valve serialization reflection table that defines which fields are stored in saved games. Modifying field offsets or orders breaks savegame compatibility.
- **RFC 2119**: The normative specification standard using uppercase keywords: SHALL, SHALL NOT, SHOULD, MAY.

---

## Completed Baseline Requirements (Phases 1-6)

### Requirement 1: Player Combat & Armor Absorption Refactoring [COMPLETED]
- Purged dead `#if 0` blocks (`ThrowGib`, `ThrowHead`).
- Encapsulated armor damage absorption math into `CalculateArmorAbsorption`.
- Streamlined HEV suit diagnosis conditionals.

### Requirement 2: Client Command Dispatcher Refactoring [COMPLETED]
- Unified `CBasePlayer` pointer resolution at `ClientCommand` entry.
- Modernized command variable naming (`temp` -> `pszSayCmd`).

### Requirement 3: Impulse & Cheat Input Handler Deduplication [COMPLETED]
- Replaced 30+ unrolled `GiveNamedItem` calls in `impulse 101` with static table iteration.
- Scoped decal trace variables and cleaned pre-alpha comments.

### Requirement 4: VGUI Scoreboard Geometry & Cleanliness Remediation [COMPLETED]
- Encapsulated resolution thresholds (`SBOARD_RES_*`) and added `GetScaledColumnWidth`.
- Cleaned legacy resize and tracker comments.

### Requirement 5: AI Monster Sensory Redundancy Elimination [COMPLETED]
- Consolidated duplicate `ClearConditions` calls in `CBaseMonster::Listen`.
- Purged commented soundpool references and cleaned route checks.

### Requirement 6: Strict Functional & Binary Compatibility [COMPLETED]
- Validated 0 compiler warnings/errors on MSBuild `v145` and green CI checks on Linux and Windows.

### Requirement 7: 100% Code Smell Elimination in Core Hotspots [COMPLETED]
- Verified exactly 0 code smells remaining across the initial 5 hotspot files.

---

## Active Requirements: 100% Codebase-Wide Remediation

### Requirement 8: Deep Control Flow Nesting Elimination (11 Critical Sites)

**User Story:** As an engine maintainer, I want deeply nested control blocks (>= 6 levels) across the 11 identified critical sites flattened using guard clauses, early returns, or helper sub-methods, so that core algorithms are easily readable, testable, and cognitively manageable without altering execution logic.

#### Acceptance Criteria

1. WHEN evaluating guard decisions in `dlls/monsters/barney.cpp` (Line 129), THE Monster Subsystem SHALL invert preconditions using early returns/guard checks to reduce nesting depth to < 4 levels.
2. WHEN processing scientist panic and flee states in `dlls/monsters/scientist.cpp` (Line 51), THE Scientist Subsystem SHALL extract condition handling into focused helper routines to eliminate nesting >= 6 levels.
3. WHEN handling companion speech and following logic in `dlls/ai/talkmonster.cpp` (Line 169), THE Ally AI Subsystem SHALL restructure follower tracking conditionals to not exceed 4 nesting levels.
4. WHEN parsing player impulse states and active weapon interactions in `dlls/core/player.cpp` (Line 781), THE Player Core Subsystem SHALL flatten ladder conditions with early return guards.
5. WHEN searching sound sentences in `dlls/systems/sound_sentences.cpp` (Line 31), THE Audio Subsystem SHALL extract sentence lookup into a clean sub-routine to eliminate deep loops.
6. WHEN computing client predicted weapon firing in `cl_dll/hl/com_weapons.cpp` (Line 131), THE Weapon Prediction Subsystem SHALL guard against invalid weapon indices early to keep execution nesting < 4 levels.
7. WHEN calculating camera view bob in `cl_dll/render/view_bob.cpp` (Line 192), THE View Render Subsystem SHALL simplify velocity and ground contact checks with guard clauses.
8. WHEN unpacking input bitfields in `cl_dll/input/tf_defs.h` (Line 1179), THE Input Subsystem SHALL streamline bitwise masking conditionals.
9. WHEN managing network voice stream distribution in `game_shared/voice_gamemgr.cpp` (Line 21), THE Voice Subsystem SHALL invert player validity checks to reduce nesting.
10. WHEN managing bot tactical assignments in `game_shared/bot/bot_manager.cpp` (Line 3) and `game_shared/bot/bot_util.h` (Line 247), THE Bot Navigation Subsystem SHALL extract inner traversal loops into helper routines to ensure maximum nesting is < 4 levels.

---

### Requirement 9: Monster & Ally AI Subsystem Code Smell Remediation (`dlls/monsters/`, `dlls/ai/`)

**User Story:** As an AI programmer, I want monster entities, schedulers, and node navigation files to be purged of obsolete Valve pre-release annotations (`UNDONE`, `HACK`, `FIXME`, `BUG`), so that AI state machines and navigation code reflect clean, professional documentation.

#### Acceptance Criteria

1. THE Monster Subsystem SHALL modernize all 35 code smell annotations across `dlls/monsters/` (`barney.cpp`, `scientist.cpp`, `apache.cpp`, `controller.cpp`, `gargantua.cpp`, `osprey.cpp`, `zombie.cpp`, `agrunt.cpp`, `bigmomma.cpp`, `hgrunt.cpp`, `hgrunt_repel.cpp`, `ichthyosaur.cpp`, `leech.cpp`, `roach.cpp`), replacing obsolete tags with factual technical documentation.
2. THE Monster AI Subsystem SHALL remediate all 22 code smell items across `dlls/ai/` (`monsters.cpp`, `schedule.cpp`, `h_ai.cpp`, `monster_scheduler.cpp`, `talkmonster.cpp`, `nodes.h`, `flyingmonster.cpp`, `nodes_links.cpp`, `pathcorner.cpp`, `squadmonster.cpp`).
3. THE Monster AI Subsystem SHALL encapsulate magic bounding dimensions (such as `64.0f` height offsets in `monsters.cpp`) into self-descriptive named constants.
4. THE Monster AI Subsystem SHALL preserve exact schedule IDs, task definitions, condition bitmasks, and AI animation sequences.

---

### Requirement 10: Gameplay, World, & Systems Subsystems Code Smell Remediation (`dlls/gameplay/`, `dlls/world/`, `dlls/systems/`)

**User Story:** As a gameplay and level systems engineer, I want cinematic sequence controllers, triggers, trains, and interactive world entities to be cleaned of legacy workarounds, so that level interactivity and environmental entities are reliable and readable.

#### Acceptance Criteria

1. THE Gameplay Subsystem SHALL modernize all 14 code smell annotations in `dlls/gameplay/` (`scripted.cpp`, `gamerules_scoring.cpp`, `spectator.cpp`, `gamerules_spawn.cpp`, `h_cine.cpp`, `multiplay_gamerules.cpp`), replacing `UNDONE:` notes with accurate descriptions of cinematic behavior.
2. THE World Entity Subsystem SHALL remediate all 8 code smell annotations in `dlls/world/` (`world.cpp`, `trains.cpp`, `trackchange.cpp`).
3. THE Interactive Systems Subsystem SHALL remediate all 14 code smell annotations and variable naming smells in `dlls/systems/` (`sound_sentences.cpp`, `func_break.cpp`, `func_tank.cpp`, `vehicle.cpp`, `airtank.cpp`, `buttons.cpp`, `h_cycler.cpp`).
4. THE Systems Subsystem SHALL rename misleading local temporary variables named `temp` in `sound_sentences.cpp` to semantic identifiers (e.g., `sentenceIndex`, `lruItem`) to eliminate false-positive debt scanner smell detections.

---

### Requirement 11: Core Server & Weapons Subsystems Code Smell Remediation (`dlls/core/`, `dlls/weapons/`)

**User Story:** As a core engine developer, I want player core mechanics, physics helpers, and weapon base classes to be free of legacy hack tags and macro naming conflicts, so that server core operations maintain highest readability.

#### Acceptance Criteria

1. THE Core Server Subsystem SHALL remediate all 26 remaining code smell items in `dlls/core/` (`player_physics.cpp`, `util.cpp`, `animation.cpp`, `cbase.cpp`, `combat_damage.cpp`, `player.cpp`, `player_inventory.cpp`, `client_networking.cpp`, `subs.cpp`, `client.cpp`, `animating.cpp`, `combat_gib.cpp`, `player_cheats.cpp`, `util_saverestore.cpp`).
2. THE Core Server Subsystem SHALL replace macro swap temporary identifiers in `dlls/core/util.cpp` with typed template or inline swap utilities to avoid `temp` debt flags.
3. THE Weapon Subsystem SHALL remediate all 13 code smell items across `dlls/weapons/` (`weapon_base.cpp`, `player_item_base.cpp`, `projectile_grenade.cpp`, `projectile_hornet.cpp`, `weapon_crowbar.cpp`, `weapon_gauss.cpp`, `weapon_glock.cpp`, `weapon_mp5.cpp`, `weapon_python.cpp`, `weapon_snark.cpp`).
4. THE Core and Weapon Subsystems SHALL preserve 100% of weapon damage formulas, clip handling, recoil punch angles, and projectile ballistic physics.

---

### Requirement 12: Client DLL Subsystems Code Smell Remediation (`cl_dll/`)

**User Story:** As a client UI and rendering developer, I want client HUD panels, input handlers, and view rendering modules to be free of legacy Win32 bug comments and temporary menu variables, so that client architecture is uniform and pristine.

#### Acceptance Criteria

1. THE Client Input Subsystem SHALL remediate all 9 code smell items in `cl_dll/input/` (`inputw32.cpp`, `in_camera.cpp`, `input.cpp`, `tf_defs.h`), documenting legacy Win32 lookspring and mouse processing clearly.
2. THE Client HUD Subsystem SHALL remediate all 9 code smell items in `cl_dll/hud/` (`menu.cpp`, `health.cpp`, `hud_benchtrace.cpp`, `hud_msg.cpp`, `status_icons.cpp`), replacing `temp` pointers with typed descriptors (e.g., `pActiveMenu`).
3. THE Client Render Subsystem SHALL remediate all 6 code smell items in `cl_dll/render/` (`view.cpp`, `view_camera.cpp`, `overview.cpp`, `view_bob.cpp`).
4. THE Client Studio and Weapon Subsystems SHALL remediate all 8 code smell items in `cl_dll/hl/` (`com_weapons.cpp`, `hl_weapons.cpp`) and `cl_dll/studio/` (`studio_render_bones.cpp`, `studio_util.cpp`).
5. THE Client VGUI and System Subsystems SHALL remediate all 4 remaining items in `cl_dll/vgui/` (`vgui_viewport_menus.cpp`, `vgui_TeamFortressViewport.cpp`), `cl_dll/entities/entity.cpp`, `cl_dll/core/` (`util.cpp`, `cdll_int.cpp`), and `cl_dll/systems/demo.cpp`.

---

### Requirement 13: Shared Physics, Performance Counters & Bot Navigation Remediation (`pm_shared/`, `game_shared/`)

**User Story:** As a multiplayer network programmer, I want player movement physics and shared bot navigation code to be clean and accurately documented, so that prediction matching between client and server remains exact.

#### Acceptance Criteria

1. THE Shared Player Movement Subsystem SHALL remediate all 12 code smell items in `pm_shared/` (`pm_move_water.c`, `pm_math.c`, `pm_step.c`, `pm_duck.c`, `pm_move_air.c`), sanitizing `temp` variables into descriptive math vectors/floats without modifying movement calculations.
2. THE Shared Performance Subsystem SHALL remediate all 8 `TEMP` code smell tags in `game_shared/perf_counter.h`, replacing temporary benchmarking comments with clear performance tracking documentation.
3. THE Bot Navigation Subsystem SHALL remediate all 18 code smell items in `game_shared/bot/` (`bot_util.cpp`, `nav_area_connect.cpp`, `nav_area_tactical.cpp`, `nav_area.h`, `bot_manager.cpp`, `nav_area.cpp`, `nav_path.cpp`, `bot_profile.cpp`, `nav_file.cpp`, `nav_path.h`, `bot.cpp`, `bot_manager.h`, `bot_util.h`, `nav_node.cpp`, `nav_node.h`).
4. THE Shared Voice Subsystem SHALL remediate all 2 code smell items in `game_shared/voice_common.h` and `game_shared/voice_gamemgr.cpp`.

---

### Requirement 14: Comprehensive Verification, Zero-Smell Target & Continuous Quality Gate

**User Story:** As the project lead, I want the entire active codebase to report exactly 0 code smells in automated debt scans and compile with 0 warnings on MSBuild and GitHub Actions CI, so that the refactoring delivers absolute code cleanliness without technical debt or regressions.

#### Acceptance Criteria

1. UPON completion of all remediation phases, THE Technical Debt Scanner (`debt_scanner.py`) SHALL report **0 code smells remaining** across all active directories (`dlls/`, `cl_dll/`, `pm_shared/`, and `game_shared/`).
2. THE entire solution SHALL build cleanly with 0 errors and 0 warnings on MSBuild toolset `v145` (Win32 Release configuration) for both `hldll.vcxproj` and `hl_cdll.vcxproj`.
3. ALL GitHub Actions CI workflows (Linux x86 DLLs and Windows x86 DLLs) SHALL pass without failure.
4. ALL entity save/restore tables (`TYPEDESCRIPTION`) and network serialization protocols SHALL remain 100% binary identical.

---

## Non-Functional Requirements

- **NFR-1: Zero Runtime Overhead**: Flattened control flows and sanitized variables must introduce zero performance penalties and maintain or improve branch predictability.
- **NFR-2: 100% Savegame Compatibility**: No modifications to entity struct sizes, alignments, or member variable order in classes serialized via `TYPEDESCRIPTION`.
- **NFR-3: Strict Behavior Preservation**: Zero alterations to gameplay mechanics, monster scheduling, player movement physics, weapon firing rates, or client UI layout.
- **NFR-4: Clean Build Baseline**: Zero compiler warnings or lint errors introduced across MSBuild and GCC/Clang pipelines.
