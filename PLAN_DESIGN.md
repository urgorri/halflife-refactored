# System Design Document: Subsystem Defragmentation & HUD Consolidation

## Overview

This design outlines the defragmentation and structural consolidation of tightly coupled subsystems across the Half-Life server and client engines. By co-locating tightly coupled class hierarchies (`CBaseTurret` subclasses, `CBeam`/`CSprite` visual entities, and client HUD modules) into unified, cohesive translation units, we reduce compilation overhead, improve code readability, and eliminate unnecessary file dispersion without changing any runtime behavior or public interfaces.

## System Architecture

### Component Map

| Component ID | Name | Subsystem | Responsibility | Interfaces With |
| :--- | :--- | :--- | :--- | :--- |
| **COMP-TURRET** | `dlls/systems/turrets.*` | Server Systems | Automated turret AI, deployment mechanics, targeting, and bullet attacks for standard, mini, and sentry turrets | `CBaseMonster`, `util_entity`, `sound`, `weapon_base` |
| **COMP-BEAMS** | `dlls/systems/effects_beams.*` | Server Systems | Entity-based beam generation, laser aiming, lightning effects, and glowing sprite management | `CBaseEntity`, `effects.h`, `util`, network messages |
| **COMP-HUD-SPEC** | `cl_dll/hud/hud_spectator.*` | Client HUD | Spectator camera tracking, director mode transitions, overview map radar, and spectator command menu UI | `CHudBase`, `vgui`, engine callbacks, spectator network messages |
| **COMP-HUD-AMMO** | `cl_dll/hud/hud_ammo.*` | Client HUD | Ammunition counting, secondary weapon indicators, and pickup history HUD notifications | `CHudBase`, `cl_util`, weapon network user messages |

### High-Level Architecture Diagram

```
+-----------------------------------------------------------------------------------+
|                              HALF-LIFE GAME ENGINE                                |
+-----------------------------------------+-----------------------------------------+
|               SERVER DLL                |               CLIENT DLL                |
+--------------------+--------------------+--------------------+--------------------+
|  Turret Subsystem  |    Beam & Sprite   |   Spectator HUD    |      Ammo HUD      |
|  (turrets.cpp/.h)  |  (effects_beams)   | (hud_spectator.*)  |   (hud_ammo.*)     |
|                    |                    |                    |                    |
| - CBaseTurret      | - CBeam            | - CHudSpectator    | - CHudAmmo         |
| - CTurret          | - CLaser           | - Director Mode    | - CHudAmmoSecondary|
| - CMiniTurret      | - CLightning       | - Overview Radar   | - CHudAmmoHistory  |
| - CSentry          | - CGlow            | - Spectator Menu   |                    |
|                    | - CSprite          |                    |                    |
+--------------------+--------------------+--------------------+--------------------+
|          Core Entity Framework          |          HUD Rendering Pipeline         |
+-----------------------------------------+-----------------------------------------+
```

## Subsystem Details & Refactoring Architecture

### 1. Turret Subsystem Consolidation (`dlls/systems/turrets.cpp` / `turrets.h`)
- **Current Layout**:
  - `turret_base.cpp` (`CBaseTurret`)
  - `turret.cpp` (`CTurret`)
  - `miniturret.cpp` (`CMiniTurret`)
  - `sentry.cpp` (`CSentry`)
  - `turret.h`
- **Target Layout**:
  - `dlls/systems/turrets.h`: Clean class declarations for `CBaseTurret`, `CTurret`, `CMiniTurret`, and `CSentry`.
  - `dlls/systems/turrets.cpp`: Unified implementation of base state machines and specialized derived methods (`Shoot`, `Spawn`, `Precache`, `SpinUpCall`, `SpinDownCall`, `SentryDeath`).
- **Files Deleted**: `turret_base.cpp`, `turret.cpp`, `miniturret.cpp`, `sentry.cpp`, `turret.h`.

### 2. Beam & Sprite Visual Effects (`dlls/systems/effects_beams.cpp`)
- **Current Layout**:
  - `effects_beam.cpp` (`CBeam`)
  - `effects_laser.cpp` (`CLaser`)
  - `effects_lightning.cpp` (`CLightning`)
  - `effects_glow.cpp` (`CGlow`)
  - `effects_sprite.cpp` (`CSprite`)
- **Target Layout**:
  - `dlls/systems/effects_beams.cpp`: Unified implementation of beam vector calculation, laser tracing, lightning arc creation, glow fading, and sprite animators.
  - `dlls/systems/effects.h`: Streamlined header declarations.
- **Files Deleted**: `effects_beam.cpp`, `effects_laser.cpp`, `effects_lightning.cpp`, `effects_glow.cpp`, `effects_sprite.cpp`.

### 3. Client Spectator HUD (`cl_dll/hud/hud_spectator.cpp` / `hud_spectator.h`)
- **Current Layout**:
  - `hud_spectator.cpp`
  - `hud_spectator_director.cpp`
  - `hud_spectator_overview.cpp`
  - `hud_spectator_menu.cpp`
  - `hud_spectator.h`
- **Target Layout**:
  - `cl_dll/hud/hud_spectator.h`: Unified header with `CHudSpectator` declaration and spectator sub-structures.
  - `cl_dll/hud/hud_spectator.cpp`: Unified implementation of camera modes, overview map rendering, director decision logic, and in-game spectator menus.
- **Files Deleted**: `hud_spectator_director.cpp`, `hud_spectator_overview.cpp`, `hud_spectator_menu.cpp`.

### 4. Client Ammo HUD (`cl_dll/hud/hud_ammo.cpp` / `hud_ammo.h`)
- **Current Layout**:
  - `ammo.cpp` (`CHudAmmo`)
  - `ammo_secondary.cpp` (`CHudAmmoSecondary`)
  - `ammohistory.cpp` (`CHudAmmoHistory`)
  - `ammo.h`, `ammohistory.h`
- **Target Layout**:
  - `cl_dll/hud/hud_ammo.h`: Consolidated header declaring `CHudAmmo`, `CHudAmmoSecondary`, and `CHudAmmoHistory`.
  - `cl_dll/hud/hud_ammo.cpp`: Consolidated implementation of ammo counter rendering, secondary bars, and icon history animation fading.
- **Files Deleted**: `ammo.cpp`, `ammo_secondary.cpp`, `ammohistory.cpp`, `ammo.h`, `ammohistory.h`.

## Behavioral Preservation Strategy

1. **Class Names and Linkage**: All `LINK_ENTITY_TO_CLASS` declarations (`monster_turret`, `monster_miniturret`, `monster_sentry`, `env_laser`, `env_beam`, `env_lightning`, `env_glow`, `env_sprite`) remain verbatim.
2. **Network Protocol**: User messages and entity pev fields remain strictly identical.
3. **Save/Restore**: All `TYPEDESCRIPTION` tables and `IMPLEMENT_SAVERESTORE` declarations are preserved exactly.

## Build and Verification Strategy

1. Sychronize Visual Studio project files (`projects/vs2019/hldll.vcxproj`, `projects/vs2019/hl_cdll.vcxproj` and `.filters`).
2. Synchronize Linux Makefiles (`linux/Makefile.hldll`, `linux/Makefile.hl_cdll`).
3. Compile with MSBuild Win32 Release locally and verify 0 errors.
