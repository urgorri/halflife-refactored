# Requirements Document: Code Smell Mitigation

## Introduction

This project defines the requirements for systematically mitigating high-priority **code smells** across the core Half-Life GoldSrc DLL codebase (dlls/, cl_dll/, pm_shared/, game_shared/). The primary objective is to eliminate obsolete legacy code, deep control-flow nesting, copy-paste logic duplication, and magic constants while strictly preserving 100% binary compatibility, network protocol parity, and runtime gameplay behavior.

Target subsystems identified as primary code smell hotspots:
1. dlls/core/player_combat.cpp: Legacy dead code blocks (#if 0), monolithic armor absorption arithmetic, and repetitive suit diagnosis conditionals.
2. dlls/core/client_commands.cpp: Monolithic if/else if command cascades, repetitive entity class casts, and embedded unicode decoding routines.
3. dlls/core/player_input.cpp: Unrolled linear equipment provisioning for impulse 101, obsolete pre-alpha comments and temporary variables.
4. cl_dll/vgui/vgui_ScorePanel.cpp: Resolution hack conditionals, magic layout offsets, and commented-out legacy rendering code.
5. dlls/ai/monster_sensors.cpp: Redundant successive ClearConditions calls, commented sound pool code, and un-encapsulated enemy tracking arrays.

## Glossary

- **GoldSrc DLL**: The core game dynamic link library set comprising the server library (hl.dll) and the client library (client.dll).
- **Code Smell**: Surface structures in code that indicate deeper maintenance, readability, or architectural problems without necessarily being functional bugs.
- **Technical Debt**: The implied cost of additional rework caused by choosing an easy or antiquated solution instead of a clean, maintainable approach.
- **Edict (dict_t)**: The engine-level container representing an active entity slot in the world.
- **Save/Restore Table (TYPEDESCRIPTION)**: Valve serialization reflection table that defines which fields are stored in saved games. Modifying field offsets or orders breaks savegame compatibility.
- **Impulse Command**: Out-of-band numeric command sent from client to server (e.g., impulse 100 for flashlight, impulse 101 for full weapons).
- **Hitgroup**: Anatomical region index (head, chest, stomach, limbs) used in damage calculations to multiply base weapon damage.
- **RFC 2119**: The normative specification standard using uppercase keywords: SHALL, SHALL NOT, SHOULD, MAY.

---

## Requirements

### Requirement 1: Player Combat & Armor Absorption Refactoring

**User Story:** As an engine programmer and maintainer, I want player combat and armor damage absorption logic to be modular, cleanly encapsulated, and free of dead legacy code, so that damage calculations are readable, maintainable, and verifiable without altering combat math.

#### Acceptance Criteria

1. THE Player Combat Subsystem SHALL remove all dead #if 0 blocks (specifically ThrowGib, ThrowHead, and commented-out water death routines) from dlls/core/player_combat.cpp.
2. WHEN CBasePlayer::TakeDamage executes armor absorption, THE Player Combat Subsystem SHALL encapsulate the armor reduction and damage attenuation calculations into a dedicated helper method ApplyArmorAbsorption(float flDamage, int bitsDamageType, float &flDamageRemaining, float &flArmorDrained).
3. THE ApplyArmorAbsorption method SHALL produce mathematical results bit-identical to the original calculation across all damage types (DMG_BLAST, DMG_FALL, DMG_DROWN, standard attacks) and multiplayer modes.
4. WHEN updating suit medical warnings in TakeDamage, THE Player Combat Subsystem SHALL streamline the bitwise diagnosis loop using structured damage-to-sentence mapping to reduce control flow nesting from >= 6 levels to < 3 levels.
5. THE Player Combat Subsystem SHALL retain all network event broadcasts (SVC_DIRECTOR, DRC_CMD_EVENT) and player punchangle offsets without modification.

---

### Requirement 2: Client Command Dispatcher Refactoring

**User Story:** As a server developer, I want client console command parsing to be cleanly structured with centralized player resolution and helper utilities, so that adding, auditing, or maintaining commands is straightforward and avoids duplicated boilerplate.

#### Acceptance Criteria

1. WHEN ClientCommand is invoked, THE Client Command Subsystem SHALL resolve the player pointer CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pev) once at the function entry point instead of repeating class casts across branches.
2. THE Client Command Subsystem SHALL extract UTF-8 validation and decoding routines (Q_IsValidUChar32, Q_UTF8ToUChar32) from client_commands.cpp into a dedicated shared header/utility to improve cohesion.
3. THE Client Command Subsystem SHALL replace repetitive string comparison ladders with structured command dispatch or grouped switch-friendly handlers for core commands (say, say_team, ullupdate, give, drop, ov).
4. WHERE cheats are requested (give, ov), THE Client Command Subsystem SHALL maintain exact sv_cheats check semantics and permission boundaries.
5. THE Client Command Subsystem SHALL preserve all teamplay chat logging formats (UTIL_LogPrintf) and text message network sequences (gmsgSayText).

---

### Requirement 3: Impulse & Cheat Input Handler Deduplication

**User Story:** As a gameplay programmer, I want ImpulseCommands and CheatImpulseCommands to use structured data arrays instead of linear repetitive code, so that impulse logic is compact, testable, and free of obsolete legacy comments.

#### Acceptance Criteria

1. WHEN cheat impulse 101 is executed, THE Player Input Subsystem SHALL distribute items and ammunition using an iterative data-driven table of item classnames rather than 30+ unrolled GiveNamedItem statements.
2. THE Player Input Subsystem SHALL deliver the exact same sequence of weapons, ammunition, suit, and batteries in identical order as the original code.
3. THE Player Input Subsystem SHALL remove obsolete dead comments and unused local variables (such as TraceResult tr; // UNDONE: kill me! This is temporary for PreAlpha CDs in ImpulseCommands).
4. THE Player Input Subsystem SHALL maintain exact timing and cooldown logic for spray decals (m_flNextDecalTime, decalfrequency) and flashlight toggles.

---

### Requirement 4: VGUI Scoreboard Geometry & Cleanliness Remediation

**User Story:** As a client UI developer, I want the VGUI scoreboard panel to encapsulate resolution checks and eliminate legacy hack comments, so that scoreboard layout logic is readable and resilient across display configurations.

#### Acceptance Criteria

1. THE VGUI Scoreboard Subsystem SHALL encapsulate resolution-dependent coordinate adjustments (such as 400x300 and 640x480 width thresholds) into well-named layout constants or helper methods.
2. THE VGUI Scoreboard Subsystem SHALL remove dead commented-out blocks (including obsolete tracker icon loads and temporary debug test directives).
3. THE VGUI Scoreboard Subsystem SHALL preserve exact column alignment, width calculation formulas, line borders, and font scheme bindings across all resolutions.
4. THE VGUI Scoreboard Subsystem SHALL maintain exact spectator, teamplay, and deathmatch score presentation logic.

---

### Requirement 5: AI Monster Sensory Redundancy Elimination

**User Story:** As an AI systems engineer, I want monster sensing and condition management to eliminate duplicate state operations and commented sound pool relics, so that AI sensory update loops run with maximum efficiency and clarity.

#### Acceptance Criteria

1. WHEN CBaseMonster::Listen runs, THE Monster Sensor Subsystem SHALL execute condition clearing (its_COND_HEAR_SOUND | bits_COND_SMELL | bits_COND_SMELL_FOOD) exactly once per cycle, eliminating redundant duplicate calls.
2. THE Monster Sensor Subsystem SHALL purge commented-out legacy sound pool references (g_pSoundEnt->m_SoundPool) in favor of active CSoundEnt static accessor methods.
3. THE Monster Sensor Subsystem SHALL maintain exact enemy memory stack behavior in PushEnemy and PopEnemy up to MAX_OLD_ENEMIES (4 slots) without altering monster tracking state.
4. THE Monster Sensor Subsystem SHALL preserve identical hearing sensitivity thresholds and distance falloff checks.

---

### Requirement 6: Strict Functional & Binary Compatibility

**User Story:** As a Half-Life community player and modder, I want the refactored DLLs to run with 100% functional equivalence to original GoldSrc, so that existing savegames, network packets, and gameplay timings remain completely unaffected.

#### Acceptance Criteria

1. THE refactored code SHALL compile cleanly on MSBuild toolset 145 (Visual Studio 2022) with 0 errors and 0 new warnings across both hldll.vcxproj and hl_cdll.vcxproj.
2. THE refactored code SHALL NOT alter any struct member order, alignment, or size in classes with TYPEDESCRIPTION save/restore tables.
3. THE refactored code SHALL NOT change any network message IDs, argument counts, or serialization formats.
4. THE refactored code SHALL demonstrate a measurable reduction in identified code smell items as reported by the debt_scanner.py tool.

---

## Non-Functional Requirements

- **NFR-1: Zero Runtime Overhead**: Refactored helper methods and table lookups must be inlined or compile to equal or fewer CPU instructions than original code.
- **NFR-2: 100% Savegame Compatibility**: No changes to save/restore serialization layouts or entity member variables.
- **NFR-3: Strict Behavior Preservation**: Zero alterations to damage multipliers, sound timings, command syntax, or UI rendering output.
- **NFR-4: Clean Build Baseline**: Zero compiler warnings or lint errors introduced during refactoring.
