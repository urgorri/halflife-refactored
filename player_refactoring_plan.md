# Project Blueprint: Player Class Decomposition (player.cpp)

**Role:** System Architect and Planning Specialist  
**Project:** Refactoring and Modularization of `dlls/core/player.cpp`.  
**Objective:** Decompose the monolithic, high-complexity `CBasePlayer` class into logical components (Input, Inventory, Physics, and Network) to improve readability and maintainability without altering game behavior.

---

# Part 1: Requirements Document

## Introduction
The `CBasePlayer` class in `dlls/core/player.cpp` handles all aspects of player representation, including movement, spawning, inventory tracking, inputs, UI rendering updates, and networking.

This project divides these responsibilities into distinct helper classes:
1. **`CPlayerInput`** (manages inputs, button checks, and command formatting)
2. **`CPlayerInventory`** (manages weapons, switching, pickups, and ammunition)
3. **`CPlayerPhysics`** (integrates with physical movement loops and collisions)
4. **`CPlayerNetwork`** (formats player state writes to network streams)

## Requirements

### Requirement 1 (REQ-PL-1): Modular Components
**User Story:** As a developer, I want the player responsibilities to be separated, so that I can modify weapon selection logic without editing physics/movement code.
#### Acceptance Criteria
1. THE system SHALL move player inputs, button events, and command sequences to `dlls/core/player_input.cpp` / `.h`.
2. THE system SHALL move player weapon inventories, switching slots, and ammo thresholds to `dlls/core/player_inventory.cpp` / `.h`.
3. THE system SHALL move player world movement and environmental collisions to `dlls/core/player_physics.cpp` / `.h`.
4. THE system SHALL move player write messages and network stream formatting to `dlls/core/player_network.cpp` / `.h`.

### Requirement 2 (REQ-PL-2): Behavior Integrity
**User Story:** As a player, I want all player interactions and physics to feel identical, so that gameplay is completely preserved.
#### Acceptance Criteria
1. Physics computations (speed, jump heights, friction, water movement) SHALL remain identical.
2. Network synchronization rate and packet layout write orders SHALL be preserved.

---

# Part 2: System Design Document

## System Architecture

### Component Map

| Component ID | Name | Type | Responsibility | Interfaces With |
|:---|:---|:---|:---|:---|
| **COMP-PL-IN** | `CPlayerInput` | Class | Handles button state and client commands | Engine CMD callbacks |
| **COMP-PL-INV** | `CPlayerInventory` | Class | Handles weapons, switching, and ammo limits | Game weapon items |
| **COMP-PL-PHY** | `CPlayerPhysics` | Class | Handles player movement, velocity, and collision | PM shared library |
| **COMP-PL-NET** | `CPlayerNetwork` | Class | Prepares client status and network packet writes | Engine network API |

### High-Level Architecture Diagram
```
                          [ CBasePlayer ]
                                 |
         +-------------+---------+---------+-------------+
         |             |                   |             |
         v             v                   v             v
  [ CPlayerInput ] [ CPlayerInventory ] [ CPlayerPhysics ] [ CPlayerNetwork ]
```

---

# Part 3: Implementation Plan

- [ ] **Phase 1: Code Decomposition**
  - [ ] **Task 1.1: Extract Input Component**
    - Create `dlls/core/player_input.cpp` and `player_input.h`.
    - Move button tracking and command handler methods.
    - _Requirements: [REQ-PL-1]_
  - [ ] **Task 1.2: Extract Inventory Component**
    - Create `dlls/core/player_inventory.cpp` and `player_inventory.h`.
    - Move weapon slots, active weapon switching, and ammo limits.
    - _Requirements: [REQ-PL-1]_
  - [ ] **Task 1.3: Extract Physics Component**
    - Create `dlls/core/player_physics.cpp` and `player_physics.h`.
    - Move velocity calculations, friction, and PM loop interactions.
    - _Requirements: [REQ-PL-1]_
  - [ ] **Task 1.4: Extract Network Component**
    - Create `dlls/core/player_network.cpp` and `player_network.h`.
    - Move network write packets and state sync logic.
    - _Requirements: [REQ-PL-1]_

- [ ] **Phase 2: Build Integration**
  - [ ] **Task 2.1: Update MSBuild Files**
    - Update `projects/vs2019/hldll.vcxproj` and `.filters`.
  - [ ] **Task 2.2: Update Linux Makefile**
    - Update object lists in `linux/Makefile.hldll`.

- [ ] **Phase 3: Verification**
  - [ ] **Task 3.1: Verify Compiles**
    - Compile on both Windows and Linux, verifying zero errors.
