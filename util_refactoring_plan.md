# Project Blueprint: GoldSrc Utilities Modularization (util.cpp Decomposition)

**Role:** System Architect and Planning Specialist  
**Project:** Refactoring and Modularization of `dlls/core/util.cpp` and its headers.  
**Objective:** Decompose the monolithic utilities catch-all file into three focused, low-coupling modules (Math, Entities, and Debug/Logging) without modifying game behavior or public API contracts.

---

# Part 1: Requirements Document

## Introduction
The current `dlls/core/util.cpp` is a monolithic file (~64 KB) containing helper functions ranging from mathematical operations (vector algebra, random numbers) to entity-related logic (traces, finding players, allocation) and debugging/alerts. 

This project decomposes these utilities into three separate components:
1. **`util_math`** (pure mathematical helpers)
2. **`util_entity`** (entity interaction and lookup helpers)
3. **`util_debug`** (alerting, debugging, and printing helpers)

## Glossary
- **util.cpp**: The original monolithic utilities helper file.
- **Strict Aliasing**: Compiler assumption that pointers of different types do not point to the same memory.
- **Engine Callback**: Function pointers provided by the GoldSrc engine wrapper (`gEngfuncs` / `enginefuncs_t`).

## Requirements

### Requirement 1 (REQ-1): Low Coupling
**User Story:** As a developer, I want utilities to be grouped into distinct files, so that I don't have to look through a monolithic 2000-line file to find mathematical or entity helpers.
#### Acceptance Criteria
1. THE system SHALL move all pure mathematical functions (vector math, random number generation, angle calculations) to `dlls/core/util_math.cpp` and `dlls/core/util_math.h`.
2. THE system SHALL move all entity lookup and world interaction helpers (traces, entity allocation, targetname lookups, client message writes) to `dlls/core/util_entity.cpp` and `dlls/core/util_entity.h`.
3. THE system SHALL move all engine alerting and console print helpers (alerts, debug formats, logging) to `dlls/core/util_debug.cpp` and `dlls/core/util_debug.h`.

### Requirement 2 (REQ-2): Behavior Preservation
**User Story:** As a player, I want all utility functions to produce the exact same numerical and logical outcomes, so that game behavior is completely unchanged.
#### Acceptance Criteria
1. The mathematical formulas and random number generators SHALL remain identical down to the bitwise level.
2. The logic for entity lookup and tracing SHALL match the original engine execution paths exactly.
3. Compilation outputs for both Windows and Linux DLLs SHALL be functionally equivalent to the original DLLs.

### Requirement 3 (REQ-3): Build System Compatibility
**User Story:** As a compiler, I want the build systems to recognize the new files, so that they compile cleanly on all target platforms.
#### Acceptance Criteria
1. The MSBuild files (`hldll.vcxproj` and `.filters`) SHALL include compilation rules for `util_math.cpp`, `util_entity.cpp`, and `util_debug.cpp`.
2. The Linux Makefile (`Makefile.hldll`) SHALL list `util_math.o`, `util_entity.o`, and `util_debug.o` under object targets.

---

# Part 2: System Design Document

## System Architecture

### Component Map

| Component ID | Name | Type | Responsibility | Interfaces With |
|:---|:---|:---|:---|:---|
| **COMP-MATH** | `util_math` | Module | Math utility functions (randoms, vectors, angles) | Engine random callbacks |
| **COMP-ENT** | `util_entity` | Module | Entity querying, tracing, and instantiation | Engine entity callbacks, game entities |
| **COMP-DBG** | `util_debug` | Module | Print formatting, developer alerts, logs | Engine alert callbacks |

### High-Level Architecture Diagram
```
              [ Game Entities / Modules ]
                           |
         +-----------------+-----------------+
         |                 |                 |
         v                 v                 v
   [ util_math ]    [ util_entity ]    [ util_debug ]
         |                 |                 |
         +-----------------+-----------------+
                           |
                           v
              [ GoldSrc Engine Callbacks ]
```

## Data Flow Specifications

### Ingestion and Trace Flow
```
1. Game Entity → util_entity: Call UTIL_TraceLine(vecStart, vecEnd, ignoreMonsters, pentIgnore, &tr)
2. util_entity → Engine: Invoke gEngfuncs.pfnTraceLine(...)
3. Engine → util_entity: Populate trace results (pmtrace_t)
4. util_entity → Game Entity: Return trace reference
```

## Components and Interfaces

### 1. COMP-MATH (`util_math`)
**Responsibility:** Pure mathematical operations and random distribution.
**Key Functions:**
- `UTIL_RandomLong(low, high)`: Generates a pseudo-random long.
- `UTIL_RandomFloat(low, high)`: Generates a pseudo-random float.
- `VecToAngles(vec)`: Converts a 3D direction vector to pitch/yaw/roll angles.

### 2. COMP-ENT (`util_entity`)
**Responsibility:** Entity and world interaction.
**Key Functions:**
- `UTIL_TraceLine(...)`: Traces a line in the collision world.
- `UTIL_FindEntityByTargetname(...)`: Finds an entity by its string targetname.
- `UTIL_EntitiesInBox(...)`: List entities overlapping a 3D bounding box.

### 3. COMP-DBG (`util_debug`)
**Responsibility:** Debug console prints and alerts.
**Key Functions:**
- `ALERT(...)`: Sends console logs or runtime errors to the engine console/log files.
- `DBG_Print(...)`: Developer-only prints.

---

# Part 3: Implementation Plan

- [ ] **Phase 1: Code Decomposition**
  - [ ] **Task 1.1: Extract Math Utilities**
    - Create `dlls/core/util_math.cpp` and `dlls/core/util_math.h`.
    - Move `UTIL_RandomLong`, `UTIL_RandomFloat`, `VecToAngles`, and related constants.
    - _Requirements: [REQ-1, REQ-2]_
  - [ ] **Task 1.2: Extract Entity Utilities**
    - Create `dlls/core/util_entity.cpp` and `dlls/core/util_entity.h`.
    - Move `UTIL_TraceLine`, `UTIL_TraceHull`, `UTIL_FindEntityByTargetname`, etc.
    - _Requirements: [REQ-1, REQ-2]_
  - [ ] **Task 1.3: Extract Debug/Logging Utilities**
    - Create `dlls/core/util_debug.cpp` and `dlls/core/util_debug.h`.
    - Move `ALERT`, `DBG_Print`, and related logging functions.
    - _Requirements: [REQ-1, REQ-2]_

- [ ] **Phase 2: Build Integration**
  - [ ] **Task 2.1: Update MSBuild Project Files**
    - Add files to `projects/vs2019/hldll.vcxproj` and `hldll.vcxproj.filters`.
    - Update `AdditionalIncludeDirectories` if necessary.
    - _Requirements: [REQ-3]_
    - _Dependencies: Phase 1_
  - [ ] **Task 2.2: Update Linux Makefile**
    - Add object compilation targets to `linux/Makefile.hldll`.
    - _Requirements: [REQ-3]_
    - _Dependencies: Phase 1_

- [ ] **Phase 3: Validation and Verification**
  - [ ] **Task 3.1: Windows Compilation Check**
    - Compile `hldll` using MSBuild in Release mode. Verify no compile or link errors.
    - _Requirements: [REQ-2]_
    - _Dependencies: Phase 2_
  - [ ] **Task 3.2: Linux Compilation Check**
    - Run `make hl` in the `linux/` directory. Verify zero errors.
    - _Requirements: [REQ-2]_
    - _Dependencies: Phase 2_
  - [ ] **Task 3.3: Linter (Cppcheck) Run**
    - Run the local Cppcheck command. Verify code matches rules.
    - _Requirements: [REQ-2]_
    - _Dependencies: Phase 2_
