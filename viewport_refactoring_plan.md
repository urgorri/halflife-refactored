# Project Blueprint: TeamFortress Viewport Decomposition (vgui_TeamFortressViewport.cpp)

**Role:** System Architect and Planning Specialist  
**Project:** Refactoring and Modularization of `cl_dll/vgui/vgui_TeamFortressViewport.cpp`.  
**Objective:** Decompose the monolithic, high-coupling UI coordinator panel class into cohesive sub-panels (Team selection, Class selection, and Command menus) to improve structure.

---

# Part 1: Requirements Document

## Introduction
The client-side VGUI class `TeamFortressViewport` in `cl_dll/vgui/vgui_TeamFortressViewport.cpp` manages all UI sub-menus. This project separates the menus into dedicated files:
1. **`CTeamMenuPanel`** (handles team choosing/statistics UI)
2. **`CClassMenuPanel`** (handles class choosing/information UI)
3. **`CCommandMenuPanel`** (handles command execution lists/sub-options UI)
4. **`TeamFortressViewport`** (retained as coordinator using Mediator pattern)

## Requirements

### Requirement 1 (REQ-VP-1): Menu Decomposition
**User Story:** As a client UI designer, I want menus to be separated, so that I can modify the class menu layout without risking breaking the team menu rendering.
#### Acceptance Criteria
1. THE system SHALL move team selection rendering and click callbacks to `cl_dll/vgui/vgui_TeamMenuPanel.cpp` / `.h`.
2. THE system SHALL move class selection rendering and text attributes to `cl_dll/vgui/vgui_ClassMenuPanel.cpp` / `.h`.
3. THE system SHALL move nested command lists and buttons execution to `cl_dll/vgui/vgui_CommandMenuPanel.cpp` / `.h`.

---

# Part 2: System Design Document

## System Architecture

### Component Map

| Component ID | Name | Type | Responsibility | Interfaces With |
|:---|:---|:---|:---|:---|
| **COMP-VP-TM** | `CTeamMenuPanel` | Class | Team choosing interface rendering | TeamFortressViewport |
| **COMP-VP-CL** | `CClassMenuPanel` | Class | Class choosing interface rendering | TeamFortressViewport |
| **COMP-VP-CM** | `CCommandMenuPanel` | Class | Nested lists execution menu rendering | TeamFortressViewport |
| **COMP-VP-VP** | `TeamFortressViewport` | Class | Coordinates panel focus, key events, and visibility | Client DLL Core |

### High-Level Architecture Diagram
```
                    [ Client DLL Core / HUD ]
                               |
                   [ TeamFortressViewport ]
                               |
         +---------------------+---------------------+
         |                     |                     |
         v                     v                     v
 [ CTeamMenuPanel ]    [ CClassMenuPanel ]    [ CCommandMenuPanel ]
```

---

# Part 3: Implementation Plan

- [ ] **Phase 1: Code Decomposition**
  - [ ] **Task 1.1: Extract Team Menu Panel**
    - Create `cl_dll/vgui/vgui_TeamMenuPanel.cpp` and `vgui_TeamMenuPanel.h`.
    - Move team list UI, graphics drawing, and team buttons.
    - _Requirements: [REQ-VP-1]_
  - [ ] **Task 1.2: Extract Class Menu Panel**
    - Create `cl_dll/vgui/vgui_ClassMenuPanel.cpp` and `vgui_ClassMenuPanel.h`.
    - Move class description rendering and class buttons.
    - _Requirements: [REQ-VP-1]_
  - [ ] **Task 1.3: Extract Command Menu Panel**
    - Create `cl_dll/vgui/vgui_CommandMenuPanel.cpp` and `vgui_CommandMenuPanel.h`.
    - Move nested menu parsing and execution callback events.
    - _Requirements: [REQ-VP-1]_

- [ ] **Phase 2: Build Integration**
  - [ ] **Task 2.1: Update MSBuild Files**
    - Add files to `projects/vs2019/hl_cdll.vcxproj` and `.filters`.
  - [ ] **Task 2.2: Update Linux Makefile**
    - Add object compilation rules in `linux/Makefile.hl_cdll`.

- [ ] **Phase 3: Verification**
  - [ ] **Task 3.1: Verify Compiles**
    - Compile client DLL on Windows and Linux, verifying zero errors.
