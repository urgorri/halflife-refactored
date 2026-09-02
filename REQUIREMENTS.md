# Requirements Document: Pragmatic Modularization & Code Deduplication

## Introduction

This document specifies the requirements for consolidating over-modularized entity classes, eliminating cross-weapon ammo duplication, and decoupling multi-entity systems in the Half-Life GoldSrc DLL codebase, strictly adhering to the updated [AGENTS.md](file:///E:/Dev/urgorri/halflife-refactored/AGENTS.md) guidelines.

## Glossary

- **Cohesive Entity**: A self-contained game entity whose member functions and state are private to itself and not shared across other game entities.
- **Ammo Factory / Helper**: A reusable template or macro mechanism in `dlls/weapons/ammo_base.h` that eliminates repetitive boilerplate for `ammo_*` entity classes.
- **Weapon Box**: Container entity (`CWeaponBox`) spawned when players drop weapons or die.
- **Environmental Effects**: Visual entities (`beam`, `env_lightning`, `trip_beam`, `env_laser`, `env_glow`, `env_sprite`) currently coupled inside `effects.cpp`.

---

## Requirements

### Requirement 1: Consolidation of Over-Fragmented Monster Classes

**User Story:** As an engine developer, I want monster member functions consolidated in their primary entity files so that navigating and maintaining monster behavior is intuitive and cohesive.

#### Acceptance Criteria

1. WHEN inspecting [`dlls/monsters/hgrunt.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/monsters/hgrunt.cpp), THE monster class `CHGrunt` SHALL contain all its member functions (`hgrunt_combat.cpp` and `hgrunt_squad.cpp` consolidated), while [`hgrunt_repel.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/monsters/hgrunt_repel.cpp) SHALL isolate `CHGruntRepel` and `CDeadHGrunt`.
2. WHEN inspecting [`dlls/monsters/scientist.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/monsters/scientist.cpp), THE `CScientist` class SHALL contain its healing routines and prop variants (`scientist_heal.cpp` consolidated into `scientist.cpp`).
3. WHEN inspecting [`dlls/ai/talkmonster.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/ai/talkmonster.cpp), THE `CTalkMonster` base class SHALL contain all speech and dialogue scheduling (`talkmonster_speech.cpp` consolidated into `talkmonster.cpp`).
4. THE system SHALL eliminate forced `extern` schedule and helper linkage across fragmented monster files.

### Requirement 2: Reusable Ammo Deduplication Engine

**User Story:** As a developer, I want a reusable ammo entity definition mechanism so that duplicate `Spawn`, `Precache`, and `AddAmmo` boilerplate is eliminated across all 13 weapon source files.

#### Acceptance Criteria

1. THE system SHALL provide a centralized header [`dlls/weapons/ammo_base.h`](file:///E:/Dev/urgorri/halflife-refactored/dlls/weapons/ammo_base.h) defining standard ammo registration macros and base helpers.
2. WHEN an ammo entity (`ammo_glockclip`, `ammo_9mmAR`, `ammo_buckshot`, `ammo_357`, `ammo_crossbow`, `ammo_rpgclip`, etc.) is declared, IT SHALL utilize the shared base macro/helper without copy-pasted boilerplate.
3. THE system SHALL preserve identical model paths, ammo give amounts, maximum carry limits, and pickup sound behavior for every ammo type.

### Requirement 3: Weapon Base Subsystem & Contained Entity Decoupling

**User Story:** As a programmer, I want `CWeaponBox` and `CBasePlayerItem` decoupled from `weapon_base.cpp` so that the weapon state machine remains focused and clean.

#### Acceptance Criteria

1. THE `CWeaponBox` entity and its weapon/ammo packing methods SHALL be extracted to [`dlls/weapons/weapon_box.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/weapons/weapon_box.cpp) and [`dlls/weapons/weapon_box.h`](file:///E:/Dev/urgorri/halflife-refactored/dlls/weapons/weapon_box.h).
2. THE `CBasePlayerItem` world physics, falling, and materialization routines SHALL be extracted to [`dlls/weapons/item_base.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/weapons/item_base.cpp).
3. THE core file [`dlls/weapons/weapon_base.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/weapons/weapon_base.cpp) SHALL retain only `CBasePlayerWeapon` state transitions, deployment, animation, and prediction callbacks.

### Requirement 4: Environmental Effects & Visual Entity Separation

**User Story:** As a level designer and programmer, I want separate entity files for distinct environmental effects so that beams, lasers, sprites, and glows do not share a single 1,169-line monolithic file.

#### Acceptance Criteria

1. THE beam entities (`beam`, `CBeam`) SHALL reside in [`dlls/systems/effects_beam.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/systems/effects_beam.cpp).
2. THE lightning and trip-beam entities (`env_lightning`, `env_beam`, `trip_beam`) SHALL reside in [`dlls/systems/effects_lightning.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/systems/effects_lightning.cpp).
3. THE laser projector entity (`env_laser`, `CLaser`) SHALL reside in [`dlls/systems/effects_laser.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/systems/effects_laser.cpp).
4. THE glow and sprite entities (`env_glow`, `CGlow`, `env_sprite`, `CSprite`) SHALL reside in [`dlls/systems/effects_glow.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/systems/effects_glow.cpp) and [`dlls/systems/effects_sprite.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/systems/effects_sprite.cpp).

### Requirement 5: Client Input & Hardware Polling Decoupling

**User Story:** As an engine developer, I want client hardware input separated from usercmd movement building so that mouse/joystick polling does not pollute view angle calculation.

#### Acceptance Criteria

1. THE Win32 and raw mouse/joystick polling routines SHALL reside in [`cl_dll/input/input_hardware.cpp`](file:///E:/Dev/urgorri/halflife-refactored/cl_dll/input/input_hardware.cpp).
2. THE client movement calculation (`CL_CreateMove`) and button state processing SHALL reside in [`cl_dll/input/input.cpp`](file:///E:/Dev/urgorri/halflife-refactored/cl_dll/input/input.cpp).

### Requirement 6: Build Verification & Cross-Platform Integrity

**User Story:** As a maintainer, I want project files and Makefiles synchronized so that Windows and Linux CI checks pass with zero errors and zero regressions.

#### Acceptance Criteria

1. THE build system SHALL compile both Client (`client.dll` / `client.so`) and Server (`hl.dll` / `hl.so`) with **0 errors**.
2. ALL changes SHALL preserve exact GoldSrc entity names, network message formats, and gameplay behaviors.
