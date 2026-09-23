# Prompt: Comprehensive End-to-End Behavioral Equivalence Audit

> **Role & Objective**: You are a Principal Systems Engineer and GoldSrc Engine Specialist. Your objective is to conduct an end-to-end audit of the entire `halflife-refactored` codebase to identify, document, and remediate behavioral regressions, logic drift, and memory corruption introduced during earlier refactoring phases, comparing strictly against canonical Valve GoldSrc SDK source code (`upstream/master` from `https://github.com/ValveSoftware/halflife`).

---

## 1. Background & Context

In `halflife-refactored`, several refactoring iterations modularized, decomposed, and consolidated legacy single-file subsystems (such as `StudioModelRenderer.cpp`, `triggers.cpp`, `sound.cpp`, `weapons.cpp`, and `gamerules.cpp`).

While static CI layers (Layer 1 Constants, Layer 2 SaveData Offsets, and Layer 4 Exported Symbols) passed cleanly, severe runtime bugs were discovered during real in-game testing:
1. **Studio Model Renderer Regressions** (Investigated & Fixed in PR #124):
   - Fragmenting `CStudioModelRenderer` across multiple files introduced an uncanonical `curstate.scale` matrix multiplication, omitted span pointer advancement in bone interpolation (`StudioCalcBonePosition`), overwritten `StudioMergeBones` with duplicated `StudioSetupBones` code, and caused negative index reads (`pbones[-1]`).
   - These bugs caused game stuttering, massive CPU load from SEH exceptions, and grotesque model distortion.
2. **Level Transition / Changelevel Regression** (Active Problem):
   - In single-player Half-Life, transitioning between maps via `trigger_changelevel` fails: the player spawns in random or default positions (often `info_player_start` at map coordinates `(0, 0, 0)`) rather than properly continuing relative to the landmark entity.
   - Entities and player state are not being placed or carried over correctly across level transitions.
3. **Suspicion of Wider Drift**:
   - Other consolidated or refactored files (such as `triggers_brush.cpp`, `triggers_point.cpp`, `chargers.cpp`, `doors.cpp`, `turrets.cpp`, `sound_*.cpp`, `pm_shared/`) may contain subtle logic deletions, altered math formulas, broken conditionals, or missing save/restore table entries.

---

## 2. Target Scope & Priority Areas

Perform an exhaustive, function-by-function comparison between the current codebase and `upstream/master`:

### Priority 1: Level Transitions, Landmarks & Player Spawning
- **Files**:
  - `dlls/systems/triggers_brush.cpp` (specifically `CChangeLevel`, `ChangeList`, `AddTransitionToList`, `InTransitionVolume`, `FindLandmark`, `TouchChangeLevel`, `UseChangeLevel`, `ExecuteChangeLevel`).
  - `dlls/core/cbase.cpp` & `dlls/core/saverestore.cpp` (entity transition lists, landmark coordinate offsetting, `FIELD_POSITION_VECTOR` transformation during level change).
  - `dlls/core/player.cpp` (`ClientPutInServer`, `CBasePlayer::Spawn`, `EntSelectSpawnPoint`, handling of `SF_CHANGELEVEL_*` flags and landmark adjustment).
  - `dlls/gameplay/gamerules.cpp` (`GetPlayerSpawnSpot`, transition spawn point selection).
- **Core Questions to Answer**:
  - Why is the player landmark delta calculation failing? Is `m_szLandmarkName` correctly read and passed?
  - Is `ChangeList` populating `pLevelList` with the exact upstream entity list and landmark positions?
  - Is the engine level change command (`CHANGELEVEL` or `CHANGELEVEL2`) called with the correct parameters?
  - Are landmark relative positions (`origin - landmark_origin`) being altered or corrupted during level transition restore?

### Priority 2: Consolidated Entity & Trigger Subsystems
- **Files**:
  - `dlls/systems/triggers_brush.cpp` (consolidated from `trigger_multiple`, `trigger_once`, `trigger_hurt`, `trigger_push`, `trigger_teleport`, `trigger_changelevel`, `trigger_ladder`, `trigger_gravity`).
  - `dlls/systems/triggers_point.cpp` (consolidated from `trigger_relay`, `trigger_auto`, `trigger_changetarget`, `trigger_counter`, `trigger_multi_manager`, `trigger_save`, `trigger_camera`).
  - `dlls/systems/doors.cpp` (consolidated from `door_rotating`, `door_momentary`, `doors`).
  - `dlls/systems/chargers.cpp` (consolidated from `charger_health`, `charger_suit`, `charger_base`).
  - `dlls/systems/turrets.cpp` (consolidated from `sentry`, `miniturret`, `turret_base`).
- **Focus**:
  - Check for omitted think callbacks, missing flags (`spawnflags`), reset timers, touch/use dispatch mismatches, or altered formulas.
  - Verify every `TYPEDESCRIPTION` table matches upstream exactly (missing fields break save/restore).

### Priority 3: Sound & Audio Processing Subsystems
- **Files**:
  - `dlls/systems/sound.cpp`, `dlls/systems/sound_ambient.cpp`, `dlls/systems/sound_dsp.cpp`, `dlls/systems/sound_sentences.cpp`, `dlls/systems/sound_speaker.cpp`.
- **Focus**:
  - Sentence parsing, room types, dynamic pitch/attenuation calculations, speaker queuing.

### Priority 4: Player Movement & Physics Prediction
- **Files**:
  - `pm_shared/pm_shared.c`, `pm_shared/pm_math.c`, `pm_shared/pm_step.c`, `pm_shared/pm_move_*.c`.
- **Focus**:
  - Step sound categorization, ladder climbing collision, ducking hull transitions, friction and air movement acceleration. Ensure no constants or boundary conditions were modified.

### Priority 5: Weapons, Prediction & Ammo Registry
- **Files**:
  - `cl_dll/hl/client_weapon_manager.cpp`, `dlls/weapons/weapon_registry.cpp`, `cl_dll/events/event_registry.cpp`.
- **Focus**:
  - Event dispatch timing, weapon animation sequence prediction, ammo deduction, secondary attack state latching.

---

## 3. Audit Methodology & Step-by-Step Execution Plan

### Step 1: Automated & AST Diff Analysis
1. Use `git diff upstream/master...HEAD` or targeted Python scripts to inspect differences across the priority subsystems.
2. Filter out trivial cosmetic changes (e.g. clang-format whitespace, curly brace placement).
3. Focus on:
   - Deleted code blocks or missing branches.
   - Altered floating-point arithmetic or vector calculations.
   - Substituted constants (e.g. `Q_PI` vs `M_PI`).
   - Altered array indices or pointer arithmetic.
   - Missing fields in `m_SaveData` tables.

### Step 2: Root Cause Identification for the Level Transition Bug
1. Trace the execution path when a player walks into a `trigger_changelevel`:
   - `TouchChangeLevel` -> `ChangeLevelNow` -> `ExecuteChangeLevel` -> `ChangeList` -> `AddTransitionToList` -> engine `CHANGELEVEL2`.
2. Trace the loading of the destination map:
   - Landmark matching in destination map (`FindLandmark`).
   - Landmark offset application: `pEntity->pev->origin = pEntity->pev->origin - landmark_origin_source + landmark_origin_dest`.
   - Player spawn resolution: `ClientPutInServer` -> `EntSelectSpawnPoint`.
3. Compare against `upstream/master:dlls/triggers.cpp` line by line. Pinpoint the exact defect causing the random spawn.

### Step 3: Classification of Findings
Document all identified defects in a structured format:
```markdown
### [BUG-ID]: [Brief Title]
- **Component / File**: `path/to/file.cpp:line`
- **Upstream Implementation**: (Provide code snippet from Valve SDK)
- **Current Refactored Code**: (Provide code snippet from current codebase)
- **Observed Failure / Risk**: (Explain the runtime failure, e.g. landmark offset dropped, SEH crash, etc.)
- **Severity**: [Critical / High / Medium / Low]
- **Prescribed Remediation**: (Exact code changes needed to restore canonical equivalence)
```

### Step 4: Behavior-Preserving Remediation
1. Apply fixes prioritizing correctness and canonical fidelity over stylistic preferences.
2. Adhere strictly to `AGENTS.md`:
   - Do not over-modularize if it hurts readability or runtime stability.
   - Do not change public API interfaces or protocol structs.
   - Preserve existing save/restore formats.

---

## 4. Verification Requirements

After implementing fixes for identified defects:
1. Run local test binaries:
   - `projects/vs2019/Release/hl_tests/hl_tests.exe`
   - `projects/vs2019/Release/smoke_test_client/smoke_test_client.exe`
2. Run baseline verification scripts:
   - `python tests/verify_constants.py --golden tests/golden/constants.json`
   - `python tests/verify_saverestore.py --golden tests/golden/saverestore_baseline.json`
   - `python tests/verify_symbols.py --golden tests/golden/symbols_client_windows.txt --binary projects/vs2019/Release/hl_cdll/client.dll`
   - `python tests/verify_symbols.py --golden tests/golden/symbols_server_windows.txt --binary projects/vs2019/Release/hldll/hl.dll`
3. Add new regression test cases (in `hl_tests`) verifying the specific math/logic fixed (e.g. landmark offset math).

---

## 5. Deliverables

1. A comprehensive markdown report (`AUDIT_FINDINGS.md`) cataloging all audited subsystems, discrepancies found, and severity ratings.
2. Working, verified patches fixing the level transition bug and all critical/high regressions.
3. Clean topic branch and PR ready for review and merge.
