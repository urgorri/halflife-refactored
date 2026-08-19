# Project Blueprint: Monster Base Class Decomposition (monsters.cpp)

**Role:** System Architect and Planning Specialist  
**Project:** Refactoring and Modularization of `dlls/ai/monsters.cpp`.  
**Objective:** Decompose the monolithic `CBaseMonster` class into cohesive helper components (Sensors, Scheduler, and Combat) to improve structure and readability.

---

# Part 1: Requirements Document

## Introduction
The base monster AI class `CBaseMonster` in `dlls/ai/monsters.cpp` defines shared characteristics for all NPCs. This project modularizes its code:
1. **`CMonsterSensors`** (manages seeing, hearing, and threat scoring)
2. **`CMonsterScheduler`** (manages tasks, schedules, and state machine changes)
3. **`CMonsterCombat`** (manages default weapon attacks, firing rates, and damage calculations)

## Requirements

### Requirement 1 (REQ-MN-1): Component Extraction
**User Story:** As an AI programmer, I want monster sensing code to be separate from movement scheduling, so that I can debug perception errors without looking at movement tasks.
#### Acceptance Criteria
1. THE system SHALL move monster vision checks and hearing detection logic to `dlls/ai/monster_sensors.cpp` / `.h`.
2. THE system SHALL move scheduler updates, task lists, and AI state machine logic to `dlls/ai/monster_scheduler.cpp` / `.h`.
3. THE system SHALL move NPC default attacks, range calculations, and damage helpers to `dlls/ai/monster_combat.cpp` / `.h`.

---

# Part 2: System Design Document

## System Architecture

### Component Map

| Component ID | Name | Type | Responsibility | Interfaces With |
|:---|:---|:---|:---|:---|
| **COMP-MN-SEN** | `CMonsterSensors` | Class | Handles target visibility and sound checks | Engine sounds/decals |
| **COMP-MN-SCH** | `CMonsterScheduler` | Class | Executes active tasks and schedules | Game entities |
| **COMP-MN-CBT** | `CMonsterCombat` | Class | Handles NPC attacks and damage | Combat modules |

### High-Level Architecture Diagram
```
                          [ CBaseMonster ]
                                 |
         +-----------------------+-----------------------+
         |                       |                       |
         v                       v                       v
 [ CMonsterSensors ]    [ CMonsterScheduler ]    [ CMonsterCombat ]
```

---

# Part 3: Implementation Plan

- [ ] **Phase 1: Code Decomposition**
  - [ ] **Task 1.1: Extract Sensory Module**
    - Create `dlls/ai/monster_sensors.cpp` and `monster_sensors.h`.
    - Move `Look`, `Listen`, and enemy threat scoring.
    - _Requirements: [REQ-MN-1]_
  - [ ] **Task 1.2: Extract Scheduler Module**
    - Create `dlls/ai/monster_scheduler.cpp` and `monster_scheduler.h`.
    - Move task execution loops and schedule transition states.
    - _Requirements: [REQ-MN-1]_
  - [ ] **Task 1.3: Extract Combat Module**
    - Create `dlls/ai/monster_combat.cpp` and `monster_combat.h`.
    - Move weapon firing routines, range constraints, and damage calculation helpers.
    - _Requirements: [REQ-MN-1]_

- [ ] **Phase 2: Build Integration**
  - [ ] **Task 2.1: Update MSBuild Files**
    - Update `projects/vs2019/hldll.vcxproj` and `.filters`.
  - [ ] **Task 2.2: Update Linux Makefile**
    - Add compilation targets to `linux/Makefile.hldll`.

- [ ] **Phase 3: Verification**
  - [ ] **Task 3.1: Verify Compiles**
    - Compile on both platforms, verifying clean compilation.
