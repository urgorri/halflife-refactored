# Implementation Plan: Codebase-Wide Code Smell Remediation

## Project Boundaries

### Must Have
- **100% Elimination of all 236 identified code smells** across the active codebase:
  - 11 Deep Control Flow Nesting sites (flattened to < 4 indentation levels using guard clauses/sub-methods).
  - 57 Monster & Ally AI smells (`dlls/monsters/`, `dlls/ai/`).
  - 36 Gameplay, World & Interactive Systems smells (`dlls/gameplay/`, `dlls/world/`, `dlls/systems/`).
  - 39 Core Server & Weapons smells (`dlls/core/`, `dlls/weapons/`).
  - 44 Client DLL smells (`cl_dll/`).
  - 49 Shared Movement, Performance Counters & Bot Navigation smells (`pm_shared/`, `game_shared/`).
- 100% preservation of gameplay behavior, network protocols, save/restore tables (`TYPEDESCRIPTION`), and binary layout.
- Clean compilation with 0 errors and 0 warnings on MSBuild toolset `v145` (Win32 Release).
- Green CI checks on GitHub Actions (Linux x86 DLLs and Windows x86 DLLs).

### Nice to Have
- Improved algorithmic readability and consistent naming style across legacy files.
- Reduction of cognitive complexity in deeply branched state machines.

### Out of Scope
- Unused legacy mod directories (`dmc/`, `ricochet/`, `utils/`, `external/`).
- Modern C++20 language migrations that break GoldSrc engine ABI compatibility.
- Unsafe string functions (`sprintf`, `strcpy`) which are tracked in a dedicated security sprint.

### Technical Constraints
- Operating System: Windows (local builds), Linux & Windows (CI builds).
- Compiler: MSBuild toolset `v145` (Visual Studio 2022) / GCC/Clang on Linux.
- Language Standard: C++98 / C++03 compliant syntax.
- Branch Policy: Never push directly to `master` or `main`. All work conducted in topic branch `refactor/code-smell-elimination-all`.

---

## Phased Implementation Plan

- [x] 1. Phase 1: Planning, Scoping & Baseline Verification [COMPLETED]
  - [x] 1.1 Run tech debt inventory scan across core codebase and catalogue code smell hotspots
    - _Requirements: [REQ-6]_
    - _Components: All_
  - [x] 1.2 Create and checkout topic branch `refactor/code-smell-mitigation`
    - _Requirements: [REQ-6]_
  - [x] 1.3 Author requirements (`REQUIREMENTS.md`), architecture design (`PLAN_DESIGN.md`), and implementation plan (`PLAN.md`)
    - _Requirements: [REQ-1, REQ-2, REQ-3, REQ-4, REQ-5, REQ-6]_
    - _Components: [COMP-1, COMP-2, COMP-3, COMP-4, COMP-5]_

- [x] 2. Phase 2: Player Combat & Damage Mitigation (`dlls/core/player_combat.cpp`) [COMPLETED]
  - [x] 2.1 Remove dead `#if 0` blocks (`ThrowGib`, `ThrowHead`, commented audio) from `dlls/core/player_combat.cpp`
    - _Requirements: [REQ-1.1]_
    - _Components: [COMP-1]_
  - [x] 2.2 Encapsulate armor damage absorption calculation into a dedicated static helper function
    - Extract `CalculateArmorAbsorption` preserving exact `DMG_FALL`, `DMG_DROWN`, `DMG_BLAST`, and multiplayer bonus logic.
    - _Requirements: [REQ-1.2, REQ-1.3]_
    - _Components: [COMP-1]_
  - [x] 2.3 De-nest and simplify suit diagnosis logic in `TakeDamage`
    - Replace deep while-loop condition nesting with structured bit-clearing helpers.
    - _Requirements: [REQ-1.4, REQ-1.5]_
    - _Components: [COMP-1]_
  - [x] 2.4 Compile and verify `hldll.vcxproj` with MSBuild (0 errors, 0 warnings)
    - _Requirements: [REQ-6.1]_

- [x] 3. Phase 3: Client Commands & Player Input Optimization (`client_commands.cpp`, `player_input.cpp`) [COMPLETED]
  - [x] 3.1 Extract common player pointer extraction in `ClientCommand`
    - _Requirements: [REQ-2.1]_
    - _Components: [COMP-2]_
  - [x] 3.2 Refactor console command routing in `client_commands.cpp`
    - _Requirements: [REQ-2.2, REQ-2.3, REQ-2.4, REQ-2.5]_
    - _Components: [COMP-2]_
  - [x] 3.3 Deduplicate impulse 101 item provisioning in `player_input.cpp`
    - _Requirements: [REQ-3.1, REQ-3.2]_
    - _Components: [COMP-3]_
  - [x] 3.4 Clean obsolete pre-alpha comments and dead variables in `player_input.cpp`
    - _Requirements: [REQ-3.3, REQ-3.4]_
    - _Components: [COMP-3]_
  - [x] 3.5 Compile and verify `hldll.vcxproj` with MSBuild (0 errors, 0 warnings)
    - _Requirements: [REQ-6.1]_

- [x] 4. Phase 4: AI Monster Sensors & VGUI Scoreboard Remediations (`monster_sensors.cpp`, `vgui_ScorePanel.cpp`) [COMPLETED]
  - [x] 4.1 Consolidate duplicated `ClearConditions` calls in `CBaseMonster::Listen`
    - _Requirements: [REQ-5.1]_
    - _Components: [COMP-5]_
  - [x] 4.2 Purge commented-out sound pool references in `monster_sensors.cpp`
    - _Requirements: [REQ-5.2, REQ-5.3, REQ-5.4]_
    - _Components: [COMP-5]_
  - [x] 4.3 Encapsulate magic resolution literals and remove dead code in `vgui_ScorePanel.cpp`
    - _Requirements: [REQ-4.1, REQ-4.2, REQ-4.3, REQ-4.4]_
    - _Components: [COMP-4]_
  - [x] 4.4 Compile and verify `hl_cdll.vcxproj` with MSBuild (0 errors, 0 warnings)
    - _Requirements: [REQ-6.1]_

- [x] 5. Phase 5: Verification, Tech Debt Re-Scan & PR Packaging [COMPLETED]
  - [x] 5.1 Run `debt_scanner.py` and verify measurable reduction in code smell count
    - _Requirements: [REQ-6.4]_
  - [x] 5.2 Perform full solution rebuild across all target configurations
    - _Requirements: [REQ-6.1]_
  - [x] 5.3 Merge PR #74 into `master`
    - _Requirements: [REQ-6.2, REQ-6.3]_

- [x] 6. Phase 6: Full 100% Remediation of Initial Hotspot Code Smells [COMPLETED]
  - [x] 6.1 Eliminate remaining 6 code smells in `dlls/core/player_combat.cpp`
    - _Requirements: [REQ-7.1, REQ-7.5]_
  - [x] 6.2 Eliminate remaining 3 code smells in `dlls/core/player_input.cpp`
    - _Requirements: [REQ-7.3, REQ-7.5]_
  - [x] 6.3 Eliminate remaining 5 code smells in `dlls/ai/monster_sensors.cpp`
    - _Requirements: [REQ-7.4, REQ-7.5]_
  - [x] 6.4 Compile with MSBuild v145 and re-run `debt_scanner.py` (0 smells across all 5 files)
    - _Requirements: [REQ-6.1, REQ-7.5]_
  - [x] 6.5 Merged into `master` via commit `f58fd56`

---

- [x] 7. Phase 7: High-Leverage Deep Control Flow Nesting Elimination (11 Critical Sites) [COMPLETED]
  - [x] 7.1 Restructure guard conditions in `dlls/monsters/barney.cpp` (L129)
    - Invert precondition checks to flatten companion follow & combat state transitions.
    - _Requirements: [REQ-8.1]_
    - _Components: [COMP-6, COMP-7]_
  - [x] 7.2 Flatten panic and flee handling in `dlls/monsters/scientist.cpp` (L51)
    - Decompose nested state cascades using guard clauses and early exits.
    - _Requirements: [REQ-8.2]_
    - _Components: [COMP-6, COMP-7]_
  - [x] 7.3 Restructure follower tracking in `dlls/ai/talkmonster.cpp` (L169)
    - Invert conversational state checks to reduce nesting depth to < 4.
    - _Requirements: [REQ-8.3]_
    - _Components: [COMP-6, COMP-7]_
  - [x] 7.4 Flatten player ladder and impulse conditions in `dlls/core/player.cpp` (L781)
    - Apply early return guards in player impulse dispatcher.
    - _Requirements: [REQ-8.4]_
    - _Components: [COMP-6, COMP-9]_
  - [x] 7.5 Extract sentence lookup sub-routine in `dlls/systems/sound_sentences.cpp` (L31)
    - Encapsulate nested sentence search loops into dedicated helper method.
    - _Requirements: [REQ-8.5]_
    - _Components: [COMP-6, COMP-8]_
  - [x] 7.6 Guard client weapon prediction early in `cl_dll/hl/com_weapons.cpp` (L131)
    - Guard against invalid player and null weapon pointers at routine entry.
    - _Requirements: [REQ-8.6]_
    - _Components: [COMP-6, COMP-10]_
  - [x] 7.7 Simplify view bob calculations in `cl_dll/render/view_bob.cpp` (L192)
    - Flatten ground velocity and camera tilt evaluations with guard returns.
    - _Requirements: [REQ-8.7]_
    - _Components: [COMP-6, COMP-10]_
  - [x] 7.8 Streamline input bitwise unpacking in `cl_dll/input/tf_defs.h` (L1179)
    - Modernize nested bitwise checks into linear mask evaluations.
    - _Requirements: [REQ-8.8]_
    - _Components: [COMP-6, COMP-10]_
  - [x] 7.9 Simplify network voice stream checks in `game_shared/voice_gamemgr.cpp` (L21)
    - Invert client index validation checks.
    - _Requirements: [REQ-8.9]_
    - _Components: [COMP-6, COMP-11]_
  - [x] 7.10 Extract bot manager and traversal loops in `game_shared/bot/bot_manager.cpp` (L3) & `bot_util.h` (L247)
    - Isolate inner node scanning into focused traversal routines.
    - _Requirements: [REQ-8.10]_
    - _Components: [COMP-6, COMP-11]_
  - [x] 7.11 Compile and verify with MSBuild toolset `v145`
    - Build `hldll.vcxproj` and `hl_cdll.vcxproj` with 0 errors and 0 warnings.
    - _Requirements: [REQ-14.2]_
    - _Dependencies: Tasks 7.1 - 7.10_

- [ ] 8. Phase 8: Monster & Ally AI Subsystem Smell Remediation (`dlls/monsters/`, `dlls/ai/`)
  - [ ] 8.1 Modernize legacy tags and comments in monster entity files (35 smells in `dlls/monsters/`)
    - Remediate `barney.cpp`, `scientist.cpp`, `apache.cpp`, `controller.cpp`, `gargantua.cpp`, `osprey.cpp`, `zombie.cpp`, `agrunt.cpp`, `bigmomma.cpp`, `hgrunt.cpp`, `hgrunt_repel.cpp`, `ichthyosaur.cpp`, `leech.cpp`, `roach.cpp`.
    - _Requirements: [REQ-9.1]_
    - _Components: [COMP-7]_
  - [ ] 8.2 Modernize legacy tags and comments in AI core scheduling files (22 smells in `dlls/ai/`)
    - Remediate `monsters.cpp`, `schedule.cpp`, `h_ai.cpp`, `monster_scheduler.cpp`, `talkmonster.cpp`, `nodes.h`, `flyingmonster.cpp`, `nodes_links.cpp`, `pathcorner.cpp`, `squadmonster.cpp`.
    - _Requirements: [REQ-9.2]_
    - _Components: [COMP-7]_
  - [ ] 8.3 Encapsulate bounding hull magic constants in `monsters.cpp`
    - _Requirements: [REQ-9.3, REQ-9.4]_
    - _Components: [COMP-7]_
  - [ ] 8.4 Compile and verify `hldll.vcxproj` with MSBuild (0 errors, 0 warnings)
    - _Requirements: [REQ-14.2]_
    - _Dependencies: Tasks 8.1 - 8.3_

- [ ] 9. Phase 9: Gameplay, World, & Interactive Systems Remediation (`dlls/gameplay/`, `dlls/world/`, `dlls/systems/`)
  - [ ] 9.1 Modernize cinematic and game rules debt in `dlls/gameplay/` (14 smells)
    - Remediate `scripted.cpp`, `gamerules_scoring.cpp`, `spectator.cpp`, `gamerules_spawn.cpp`, `h_cine.cpp`, `multiplay_gamerules.cpp`.
    - _Requirements: [REQ-10.1]_
    - _Components: [COMP-8]_
  - [ ] 9.2 Modernize world entity and train movement debt in `dlls/world/` (8 smells)
    - Remediate `world.cpp`, `trains.cpp`, `trackchange.cpp`.
    - _Requirements: [REQ-10.2]_
    - _Components: [COMP-8]_
  - [ ] 9.3 Remediate interactive system entities and sanitize `sound_sentences.cpp` (14 smells in `dlls/systems/`)
    - Rename `temp` variables in `sound_sentences.cpp` to descriptive identifiers (`sentenceIndex`, `lruItem`).
    - Remediate `func_break.cpp`, `func_tank.cpp`, `vehicle.cpp`, `airtank.cpp`, `buttons.cpp`, `h_cycler.cpp`.
    - _Requirements: [REQ-10.3, REQ-10.4]_
    - _Components: [COMP-8]_
  - [ ] 9.4 Compile and verify `hldll.vcxproj` with MSBuild (0 errors, 0 warnings)
    - _Requirements: [REQ-14.2]_
    - _Dependencies: Tasks 9.1 - 9.3_

- [ ] 10. Phase 10: Core Server & Weapons Subsystems Remediation (`dlls/core/`, `dlls/weapons/`)
  - [ ] 10.1 Modernize server core mechanics and helper files (26 smells in `dlls/core/`)
    - Remediate `player_physics.cpp`, `animation.cpp`, `cbase.cpp`, `combat_damage.cpp`, `player.cpp`, `player_inventory.cpp`, `client_networking.cpp`, `subs.cpp`, `client.cpp`, `animating.cpp`, `combat_gib.cpp`, `player_cheats.cpp`, `util_saverestore.cpp`.
    - _Requirements: [REQ-11.1]_
    - _Components: [COMP-9]_
  - [ ] 10.2 Replace macro swap temporary variables in `dlls/core/util.cpp` with inline template `SwapValues`
    - _Requirements: [REQ-11.2]_
    - _Components: [COMP-9]_
  - [ ] 10.3 Modernize weapon base and firearm entity files (13 smells in `dlls/weapons/`)
    - Remediate `weapon_base.cpp`, `player_item_base.cpp`, `projectile_grenade.cpp`, `projectile_hornet.cpp`, `weapon_crowbar.cpp`, `weapon_gauss.cpp`, `weapon_glock.cpp`, `weapon_mp5.cpp`, `weapon_python.cpp`, `weapon_snark.cpp`.
    - _Requirements: [REQ-11.3, REQ-11.4]_
    - _Components: [COMP-9]_
  - [ ] 10.4 Compile and verify `hldll.vcxproj` with MSBuild (0 errors, 0 warnings)
    - _Requirements: [REQ-14.2]_
    - _Dependencies: Tasks 10.1 - 10.3_

- [ ] 11. Phase 11: Client DLL Subsystems Remediation (`cl_dll/`)
  - [ ] 11.1 Remediate client input and camera processing (9 smells in `cl_dll/input/`)
    - Modernize `inputw32.cpp`, `in_camera.cpp`, `input.cpp`, `tf_defs.h`.
    - _Requirements: [REQ-12.1]_
    - _Components: [COMP-10]_
  - [ ] 11.2 Remediate client HUD and menu pointer identifiers (9 smells in `cl_dll/hud/`)
    - Modernize `menu.cpp` (rename `temp` -> `pszMenuText`), `health.cpp`, `hud_benchtrace.cpp`, `hud_msg.cpp`, `status_icons.cpp`.
    - _Requirements: [REQ-12.2]_
    - _Components: [COMP-10]_
  - [ ] 11.3 Remediate client view, rendering, and studio skeletal bone processing (14 smells in `cl_dll/render/`, `cl_dll/hl/`, `cl_dll/studio/`)
    - Modernize `view.cpp`, `view_camera.cpp`, `overview.cpp`, `com_weapons.cpp`, `hl_weapons.cpp`, `studio_render_bones.cpp`, `studio_util.cpp`.
    - _Requirements: [REQ-12.3, REQ-12.4]_
    - _Components: [COMP-10]_
  - [ ] 11.4 Remediate client VGUI, entity, and utility files (4 smells in `cl_dll/vgui/`, `cl_dll/entities/`, `cl_dll/core/`, `cl_dll/systems/`)
    - Modernize `vgui_viewport_menus.cpp`, `vgui_TeamFortressViewport.cpp`, `entity.cpp`, `util.cpp`, `cdll_int.cpp`, `demo.cpp`.
    - _Requirements: [REQ-12.5]_
    - _Components: [COMP-10]_
  - [ ] 11.5 Compile and verify `hl_cdll.vcxproj` with MSBuild (0 errors, 0 warnings)
    - _Requirements: [REQ-14.2]_
    - _Dependencies: Tasks 11.1 - 11.4_

- [ ] 12. Phase 12: Shared Movement, Performance Counters & Bot Navigation (`pm_shared/`, `game_shared/`)
  - [ ] 12.1 Sanitize shared player movement physics variables (12 smells in `pm_shared/`)
    - Rename `temp` variables in `pm_move_water.c` to `vecWaterVelocity` and modernize tags in `pm_math.c`, `pm_step.c`, `pm_duck.c`, `pm_move_air.c`.
    - _Requirements: [REQ-13.1]_
    - _Components: [COMP-11]_
  - [ ] 12.2 Modernize performance counter benchmark comments (8 smells in `game_shared/perf_counter.h`)
    - _Requirements: [REQ-13.2]_
    - _Components: [COMP-11]_
  - [ ] 12.3 Modernize bot navigation and tactical mesh routines (18 smells in `game_shared/bot/`)
    - Modernize `bot_util.cpp`, `nav_area_connect.cpp`, `nav_area_tactical.cpp`, `nav_area.h`, `bot_manager.cpp`, `nav_area.cpp`, `nav_path.cpp`, `bot_profile.cpp`, `nav_file.cpp`, `nav_path.h`, `bot.cpp`, `bot_manager.h`, `bot_util.h`, `nav_node.cpp`, `nav_node.h`.
    - _Requirements: [REQ-13.3]_
    - _Components: [COMP-11]_
  - [ ] 12.4 Modernize network voice manager files (2 smells in `game_shared/voice_common.h`, `game_shared/voice_gamemgr.cpp`)
    - _Requirements: [REQ-13.4]_
    - _Components: [COMP-11]_
  - [ ] 12.5 Compile and verify both `hldll.vcxproj` and `hl_cdll.vcxproj` with MSBuild (0 errors, 0 warnings)
    - _Requirements: [REQ-14.2]_
    - _Dependencies: Tasks 12.1 - 12.4_

- [ ] 13. Phase 13: Full Codebase Audit, Verification & Final Delivery
  - [ ] 13.1 Run comprehensive technical debt inventory scan with `debt_scanner.py`
    - Verify that all active directories (`dlls/`, `cl_dll/`, `pm_shared/`, `game_shared/`) report **0 code smells remaining**.
    - _Requirements: [REQ-14.1]_
  - [ ] 13.2 Perform full clean rebuild of both DLLs with MSBuild `v145` Win32 Release
    - _Requirements: [REQ-14.2]_
  - [ ] 13.3 Commit all changes to topic branch `refactor/code-smell-elimination-all`
    - _Requirements: [REQ-14.3]_
  - [ ] 13.4 Push topic branch to remote `origin/refactor/code-smell-elimination-all` and open Pull Request
    - _Requirements: [REQ-14.3]_
  - [ ] 13.5 Verify GitHub Actions CI pipelines (Linux x86 DLLs and Windows x86 DLLs) pass with 100% success
    - _Requirements: [REQ-14.4]_
