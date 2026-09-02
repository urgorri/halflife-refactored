# Requirements Document

## Introduction

This document specifies the architectural, maintainability, and compilation optimization requirements for the Half-Life 1 GoldSrc DLL codebase refactoring effort. The objective is to eliminate redundant boilerplate across world items and weapon ground pickups, consolidate over-fragmented domain entities (monsters, chargers, doors, xen flora, tanks, map rules, triggers), remove unreferenced dead code, and drastically reduce compilation times while preserving 100% of the original game behavior.

## Glossary

- **GoldSrc**: The game engine powering Half-Life 1.
- **Translation Unit (TU)**: A source file (`.cpp`) along with all included headers compiled by the compiler into an object file (`.obj` / `.o`).
- **Brush Entity**: An entity whose volume and physics bounds are defined by a BSP model brush (e.g., `trigger_multiple`, `func_door`).
- **Point Entity**: An entity positioned at a single point coordinate with no BSP volume (e.g., `trigger_relay`, `info_target`).
- **Ground Pickup**: An item or weapon entity residing in the physical world waiting to be collected by a player.
- **Dead Code**: Source files or symbols that are neither referenced nor compiled in any configuration.

---

## Requirements

### Requirement 1: Monster Auxiliary Component Reintegration (Locality & Ergonomics)

**User Story:** As a developer maintaining monster AI, I want monster-specific projectiles, effects, and auxiliary routines to reside in the parent monster file, so that related code is easily navigable without artificial header indirection.

#### Acceptance Criteria
1. THE monster subsystem SHALL consolidate `CBabyCrab` into `dlls/monsters/headcrab.cpp`.
2. THE monster subsystem SHALL consolidate `CSquidSpit` into `dlls/monsters/bullsquid.cpp`.
3. THE monster subsystem SHALL consolidate `CBigMommaMortar` into `dlls/monsters/bigmomma.cpp`.
4. THE monster subsystem SHALL consolidate `CControllerHeadBall` and `CControllerZapBall` into `dlls/monsters/controller.cpp`.
5. THE monster subsystem SHALL consolidate `CGargantuaFlame` and effects into `dlls/monsters/gargantua.cpp`.
6. THE monster subsystem SHALL consolidate `CNihilanthEnergyOrb` into `dlls/monsters/nihilanth.cpp`.
7. THE monster subsystem SHALL consolidate `CTentacleMaw` into `dlls/monsters/tentacle.cpp`.
8. THE monster subsystem SHALL consolidate `CApacheHVR` into `dlls/monsters/apache.cpp`.
9. IF auxiliary monster files and headers are consolidated, THEN all 8 obsolete `.h` and `.cpp` files SHALL be removed from the project.

---

### Requirement 2: Domain Entity Consolidation (Chargers, Doors, Flora, Tanks)

**User Story:** As a developer working with world and system entities, I want tightly related entity subclasses to be grouped into cohesive domain modules, so that the number of redundant translation units is minimized.

#### Acceptance Criteria
1. THE chargers subsystem SHALL consolidate `CWallHealth` and `CWallRecharge` into `dlls/systems/chargers.cpp`.
2. THE doors subsystem SHALL consolidate `CRotDoor` and `CMomentaryDoor` into `dlls/systems/doors.cpp`.
3. THE xen flora subsystem SHALL consolidate `CXenTree`, `CXenTreeTrigger`, and `CXenSpore` into `dlls/world/xen.cpp`.
4. THE tanks subsystem SHALL consolidate `CFuncTankGun`, `CFuncTankLaser`, `CFuncTankMortar`, and `CFuncTankRocket` into `dlls/systems/func_tank.cpp`.

---

### Requirement 3: Map Rules & Triggers Architectural Separation

**User Story:** As a developer maintaining gameplay logic and triggers, I want map rules consolidated and triggers strictly partitioned between brush and point entities, so that entity boundaries are obvious and clean.

#### Acceptance Criteria
1. THE map rules subsystem SHALL consolidate all 11 `game_*` point rule entities into `dlls/gameplay/maprules.cpp`.
2. THE triggers subsystem SHALL place all BSP volumetric touch triggers into `dlls/systems/triggers_brush.cpp`.
3. THE triggers subsystem SHALL place all logical, non-brush point triggers into `dlls/systems/triggers_point.cpp`.
4. WHEN compiling triggers, THE build system SHALL compile exactly 2 translation units for all 24 trigger entities.

---

### Requirement 4: World Item and Ground Weapon Pickup Deduplication

**User Story:** As a developer adding or modifying world pickups, I want reusable declarative macros/templates for items and ground weapons, so that repetitive `Spawn()`, `Precache()`, and `FallInit()` boilerplate is eliminated.

#### Acceptance Criteria
1. THE item subsystem SHALL provide an `IMPLEMENT_WORLD_ITEM` macro/template in `dlls/items/item_base.h`.
2. THE item subsystem SHALL declare standard world pickups (`item_battery`, `item_suit`, `item_antidote`, `item_security`, `item_longjump`, `item_healthkit`) in `dlls/items/items.cpp`.
3. THE weapon subsystem SHALL provide an `INITIALIZE_WORLD_WEAPON` macro/helper in `dlls/weapons/weapon_base.h` for consistent entity naming, model binding, and `FallInit()` ground setup.

---

### Requirement 5: Dead Code Removal & Policy Documentation

**User Story:** As a developer auditing the codebase, I want unreferenced legacy stubs and obsolete UI files removed, so that there is zero confusion regarding active codebase components.

#### Acceptance Criteria
1. THE build system and repository SHALL remove unreferenced legacy files: `dlls/core/mpstubb.cpp`, `cl_dll/hud/scoreboard.cpp`, `cl_dll/vgui/MOTD.cpp`, `cl_dll/vgui/vgui_ConsolePanel.cpp` (and header), and `cl_dll/systems/soundsystem.cpp`.
2. THE `README.md` and `AGENTS.md` SHALL explicitly document the removal of unreferenced legacy files.

---

### Requirement 6: Build Synchronization & Multi-Platform Verification

**User Story:** As a contributor, I want Visual Studio project files and Linux makefiles fully synchronized and tested, so that builds succeed with 0 errors across Windows and Linux.

#### Acceptance Criteria
1. THE Visual Studio solution files (`hldll.vcxproj`, `hl_cdll.vcxproj`, and filters) SHALL contain all updated and consolidated files with zero missing references.
2. THE Linux makefiles (`Makefile.hldll`, `Makefile.hl_cdll`) SHALL compile all object files without missing symbols.
3. THE codebase SHALL compile with 0 errors on MSBuild Win32 Release and GitHub Actions CI for Linux and Windows.
