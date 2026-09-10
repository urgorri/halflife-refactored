# Implementation Plan: Code Smell Mitigation

## Project Boundaries

### Must Have
- Elimination of high-leverage code smells in primary hotspot files:
  - `dlls/core/player_combat.cpp`: dead `#if 0` code, un-encapsulated armor absorption, deep nesting in suit diagnosis.
  - `dlls/core/client_commands.cpp`: repeated player pointer casting, monolithic if/else command ladders.
  - `dlls/core/player_input.cpp`: repetitive unrolled impulse 101 item provisioning, obsolete pre-alpha comments.
  - `cl_dll/vgui/vgui_ScorePanel.cpp`: magic resolution conditionals, dead commented blocks.
  - `dlls/ai/monster_sensors.cpp`: redundant successive `ClearConditions` calls, commented sound pool code.
- 100% preservation of gameplay behavior, network protocols, save/restore tables, and binary layout.
- Clean compilation with 0 errors and 0 warnings using MSBuild toolset `v145`.

### Nice to Have
- Reduction of code smell count by >= 25 items across target files.
- Improved code formatting and alignment adhering to GoldSrc refactoring standard.

### Out of Scope
- Unused legacy mod directories (`dmc/`, `ricochet/`, `utils/`, `external/`).
- Full rewriting or modernization to modern C++20 standard libraries (breaks compatibility with GoldSrc runtime/compilers).
- Security audit of unsafe string functions (`sprintf`, `strcpy`), which is tracked as a separate debt sprint.

### Technical Constraints
- Operating System: Windows.
- Compiler: MSBuild toolset `v145` (Visual Studio 2022).
- Language Standard: C++98 / C++03 compliant syntax.
- Branch Policy: Never push directly to `master` or `main`. All work conducted in `refactor/code-smell-mitigation`.

---

## Phased Implementation Plan

- [x] 1. Phase 1: Planning, Scoping & Baseline Verification
  - [x] 1.1 Run tech debt inventory scan across core codebase and catalogue code smell hotspots
    - _Requirements: [REQ-6]_
    - _Components: All_
  - [x] 1.2 Create and checkout topic branch `refactor/code-smell-mitigation`
    - _Requirements: [REQ-6]_
  - [x] 1.3 Author requirements (`REQUIREMENTS.md`), architecture design (`PLAN_DESIGN.md`), and implementation plan (`PLAN.md`)
    - _Requirements: [REQ-1, REQ-2, REQ-3, REQ-4, REQ-5, REQ-6]_
    - _Components: [COMP-1, COMP-2, COMP-3, COMP-4, COMP-5]_

- [x] 2. Phase 2: Player Combat & Damage Mitigation (`dlls/core/player_combat.cpp`)
  - [x] 2.1 Remove dead `#if 0` blocks (`ThrowGib`, `ThrowHead`, commented audio) from `dlls/core/player_combat.cpp`
    - _Requirements: [REQ-1.1]_
    - _Components: [COMP-1]_
    - _Dependencies: Phase 1_
  - [x] 2.2 Encapsulate armor damage absorption calculation into a dedicated static helper function
    - Extract `CalculateArmorAbsorption` preserving exact `DMG_FALL`, `DMG_DROWN`, `DMG_BLAST`, and multiplayer bonus logic.
    - _Requirements: [REQ-1.2, REQ-1.3]_
    - _Components: [COMP-1]_
    - _Dependencies: Task 2.1_
  - [x] 2.3 De-nest and simplify suit diagnosis logic in `TakeDamage`
    - Replace deep while-loop condition nesting with structured bit-clearing helpers.
    - _Requirements: [REQ-1.4, REQ-1.5]_
    - _Components: [COMP-1]_
    - _Dependencies: Task 2.2_
  - [x] 2.4 Compile and verify `hldll.vcxproj` with MSBuild
    - Ensure 0 errors, 0 warnings.
    - _Requirements: [REQ-6.1]_
    - _Dependencies: Task 2.3_

- [x] 3. Phase 3: Client Commands & Player Input Optimization (`client_commands.cpp`, `player_input.cpp`)
  - [x] 3.1 Extract common player pointer extraction in `ClientCommand`
    - Replace 15+ duplicated `GetClassPtr((CBasePlayer *)pev)` casts with single resolution.
    - _Requirements: [REQ-2.1]_
    - _Components: [COMP-2]_
    - _Dependencies: Phase 1_
  - [x] 3.2 Refactor console command routing in `client_commands.cpp`
    - Group command handling into modular helper functions (`Cmd_Say`, `Cmd_Give`, `Cmd_Drop`, `Cmd_Fov`).
    - _Requirements: [REQ-2.2, REQ-2.3, REQ-2.4, REQ-2.5]_
    - _Components: [COMP-2]_
    - _Dependencies: Task 3.1_
  - [x] 3.3 Deduplicate impulse 101 item provisioning in `player_input.cpp`
    - Replace 30+ unrolled `GiveNamedItem` calls with traversal over a static constant array `s_szImpulse101Items`.
    - _Requirements: [REQ-3.1, REQ-3.2]_
    - _Components: [COMP-3]_
    - _Dependencies: Phase 1_
  - [x] 3.4 Clean obsolete pre-alpha comments and dead variables in `player_input.cpp`
    - Remove unused `TraceResult tr; // UNDONE: kill me!` and dead comments.
    - _Requirements: [REQ-3.3, REQ-3.4]_
    - _Components: [COMP-3]_
    - _Dependencies: Task 3.3_
  - [x] 3.5 Compile and verify `hldll.vcxproj` with MSBuild
    - Ensure 0 errors, 0 warnings.
    - _Requirements: [REQ-6.1]_
    - _Dependencies: Task 3.2, Task 3.4_

- [x] 4. Phase 4: AI Monster Sensors & VGUI Scoreboard Remediations (`monster_sensors.cpp`, `vgui_ScorePanel.cpp`)
  - [x] 4.1 Consolidate duplicated `ClearConditions` calls in `CBaseMonster::Listen`
    - Remove second redundant `ClearConditions` call and dead comments.
    - _Requirements: [REQ-5.1]_
    - _Components: [COMP-5]_
    - _Dependencies: Phase 1_
  - [x] 4.2 Purge commented-out sound pool references in `monster_sensors.cpp`
    - Clean legacy commented `g_pSoundEnt->m_SoundPool` lines.
    - _Requirements: [REQ-5.2, REQ-5.3, REQ-5.4]_
    - _Components: [COMP-5]_
    - _Dependencies: Task 4.1_
  - [x] 4.3 Encapsulate magic resolution literals and remove dead code in `vgui_ScorePanel.cpp`
    - Introduce named resolution constants (`RES_LOW_WIDTH`, `RES_DEFAULT_WIDTH`) and remove dead tracker icon code.
    - _Requirements: [REQ-4.1, REQ-4.2, REQ-4.3, REQ-4.4]_
    - _Components: [COMP-4]_
    - _Dependencies: Phase 1_
  - [x] 4.4 Compile and verify `hl_cdll.vcxproj` with MSBuild
    - Ensure 0 errors, 0 warnings.
    - _Requirements: [REQ-6.1]_
    - _Dependencies: Task 4.2, Task 4.3_

- [x] 5. Phase 5: Verification, Tech Debt Re-Scan & PR Packaging
  - [x] 5.1 Run `debt_scanner.py` and verify measurable reduction in code smell count
    - Confirm decrease in debt inventory without introducing regressions.
    - _Requirements: [REQ-6.4]_
    - _Dependencies: Phases 2, 3, 4_
  - [x] 5.2 Perform full solution rebuild across all target configurations
    - Build `hldll` and `hl_cdll` in Release configuration.
    - _Requirements: [REQ-6.1]_
    - _Dependencies: Task 5.1_
  - [x] 5.3 Commit all changes to `refactor/code-smell-mitigation`
    - Format commit messages cleanly in English with clear descriptions.
    - _Requirements: [REQ-6.2, REQ-6.3]_
    - _Dependencies: Task 5.2_
