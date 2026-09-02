# Requirements Document

## Introduction

This document specifies the requirements for the consolidation and defragmentation of tightly coupled domain subsystems in the Half-Life GoldSrc refactored repository (`halflife-refactored`). The target is to unify fragmented class hierarchies (turrets, beam/sprite rendering effects, and client-side spectator/ammo HUD components) into cohesive, maintainable translation units while strictly preserving 100% binary-compatible gameplay and networking behaviors.

## Glossary

- **CBaseTurret**: Base class for all stationary automated defensive gun entities in Half-Life.
- **CTurret / CMiniTurret / CSentry**: Subclasses of `CBaseTurret` with specialized animations, firing rates, and models.
- **CBeam / CLaser / CLightning / CSprite / CGlow**: Rendering entity hierarchy managing dynamic sprite/beam visual effects and network synchronizations.
- **CHudSpectator**: Client HUD module handling camera modes, director triggers, overview maps, and spectator menu panels.
- **CHudAmmo / CHudAmmoHistory**: Client HUD module rendering primary/secondary ammunition counts and weapon pickup history animations.

## Requirements

### Requirement 1: Turret Subsystem Consolidation

**User Story:** As an engine maintainer, I want all turret variants and their base class consolidated into a single translation unit, so that turret behaviors are easy to navigate, modify, and maintain without cross-file boilerplate.

#### Acceptance Criteria

1. THE Server DLL SHALL implement `CBaseTurret`, `CTurret` (`monster_turret`), `CMiniTurret` (`monster_miniturret`), and `CSentry` (`monster_sentry`) within a single unified source file `dlls/systems/turrets.cpp` and header `dlls/systems/turrets.h`.
2. WHEN `monster_turret`, `monster_miniturret`, or `monster_sentry` is spawned in a map, THE Server DLL SHALL initialize and execute identical AI thinking, deploy/retract animation cycles, sound emissions, and projectile/bullet attacks as the original implementation.
3. THE Server DLL SHALL delete the redundant split files `turret_base.cpp`, `turret.cpp`, `miniturret.cpp`, `sentry.cpp`, and `turret.h`.

### Requirement 2: Visual Effects and Beam Hierarchy Consolidation

**User Story:** As an engine maintainer, I want sprite and beam visual rendering entities consolidated into a cohesive translation unit, so that render entity declarations and implementations are unified and build faster.

#### Acceptance Criteria

1. THE Server DLL SHALL consolidate `CBeam`, `CLaser` (`env_laser`), `CLightning` (`env_beam`, `env_lightning`), `CGlow` (`env_glow`), and `CSprite` (`env_sprite`) into `dlls/systems/effects_beams.cpp` and `dlls/systems/effects.h`.
2. WHEN an `env_laser`, `env_beam`, `env_lightning`, `env_glow`, or `env_sprite` entity is activated, triggered, or animated, THE Server DLL SHALL produce identical entity state updates, sound effects, and network user messages.
3. THE Server DLL SHALL delete the redundant micro-files `effects_beam.cpp`, `effects_laser.cpp`, `effects_lightning.cpp`, `effects_glow.cpp`, and `effects_sprite.cpp`.

### Requirement 3: Client Spectator HUD Subsystem Consolidation

**User Story:** As a client developer, I want all spectator interface logic consolidated into a single cohesive translation unit, so that spectator camera tracking, director mode, overview radar, and menu controls are unified.

#### Acceptance Criteria

1. THE Client DLL SHALL consolidate `CHudSpectator` camera directors, overview mapping, and UI menu interactions into `cl_dll/hud/hud_spectator.cpp` and `cl_dll/hud/hud_spectator.h`.
2. WHEN the local client or demo enters spectator mode, THE Client DLL SHALL render identical director views, overview insets, player tracking lists, and command menus.
3. THE Client DLL SHALL delete the redundant micro-files `hud_spectator_director.cpp`, `hud_spectator_overview.cpp`, and `hud_spectator_menu.cpp`.

### Requirement 4: Client Ammo HUD Subsystem Consolidation

**User Story:** As a client developer, I want the ammunition counter, secondary ammo bar, and pickup history HUD elements consolidated, so that weapon inventory HUD logic resides in a single clear module.

#### Acceptance Criteria

1. THE Client DLL SHALL consolidate `CHudAmmo`, `CHudAmmoSecondary`, and `CHudAmmoHistory` into `cl_dll/hud/hud_ammo.cpp` and `cl_dll/hud/hud_ammo.h`.
2. WHEN the player fires, reloads, or picks up weapons/ammunition, THE Client DLL SHALL render identical HUD icons, animation timers, fade transitions, and digit counters.
3. THE Client DLL SHALL delete the redundant micro-files `ammo.cpp`, `ammo_secondary.cpp`, `ammohistory.cpp`, `ammo.h`, and `ammohistory.h`.

### Requirement 5: Build Systems Synchronization & Clean Compilation

**User Story:** As a developer and CI pipeline, I want project files and Makefiles synchronized across all configurations, so that both Windows MSBuild and Linux GCC build cleanly with zero errors.

#### Acceptance Criteria

1. THE build system SHALL synchronize `projects/vs2019/hldll.vcxproj`, `projects/vs2019/hldll.vcxproj.filters`, `projects/vs2019/hl_cdll.vcxproj`, and `projects/vs2019/hl_cdll.vcxproj.filters`.
2. THE build system SHALL synchronize `linux/Makefile.hldll` and `linux/Makefile.hl_cdll`.
3. THE Server and Client DLLs SHALL compile cleanly on Win32 Release MSBuild and Linux x86 GCC with 0 errors.
