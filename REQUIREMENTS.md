# Requirements Document

## Introduction

This document specifies the comprehensive architectural refactoring and consolidation plan for the Half-Life GoldSrc DLL codebase (`dlls/`, `cl_dll/`, `game_shared/`). The primary objectives are to eliminate repetitive `#include` blocks, consolidate micro-headers and micro-files into cohesive subsystems, reduce compiler I/O and parsing overhead, and improve navigation and maintainability while preserving 100% of the game behavior, network protocols, entity registrations, and engine interfaces.

## Glossary

- **GoldSrc**: Valve Software game engine powering Half-Life (1998).
- **CBasePlayerWeapon**: Base class in `dlls/weapons/` for all player-equippable weapons.
- **CItem**: Base class in `dlls/items/` for world pickup items (suit, battery, healthkit, etc.).
- **CGrenade**: Base class in `dlls/weapons/` for explosive and ballistic projectile entities.
- **CStudioModelRenderer**: Skeletal mesh rendering pipeline class in `cl_dll/studio/`.
- **CTeamFortressViewport**: Main client VGUI HUD/menu manager in `cl_dll/vgui/`.
- **Micro-Header**: A header file containing only a single class or small stub (20-50 lines) that causes include pollution and repetitive inclusion cascades.
- **Micro-File**: A translation unit (.cpp) containing under 100 lines implementing isolated methods of an existing class or minor entity.

---

## Requirements

### Requirement 1: Weapons, Projectiles & World Items Consolidation

**User Story:** As an engine developer, I want all player weapons, projectile classes, and world items declared in unified, cohesive headers (`weapons.h`, `projectiles.h`, `items.h`), so that translation units across the server and client DLLs do not require 20+ repetitive micro-headers.

#### Acceptance Criteria

1. THE System SHALL consolidate the 15 weapon micro-headers (`weapon_glock.h`, `weapon_mp5.h`, `weapon_shotgun.h`, `weapon_python.h`, `weapon_crossbow.h`, `weapon_crowbar.h`, `weapon_egon.h`, `weapon_gauss.h`, `weapon_handgrenade.h`, `weapon_hornetgun.h`, `weapon_rpg.h`, `weapon_satchel.h`, `weapon_snark.h`, `weapon_spraycan.h`, `weapon_tripmine.h`) into `dlls/weapons/weapons.h`.
2. THE System SHALL consolidate the 7 projectile micro-headers (`projectile_bolt.h`, `projectile_grenade.h`, `projectile_hornet.h`, `projectile_rocket.h`, `projectile_satchel.h`, `projectile_snark.h`, `projectile_tripmine.h`) into `dlls/weapons/projectiles.h`.
3. THE System SHALL consolidate `dlls/items/item_base.h` into `dlls/items/items.h`, declaring `CItem`, `CWorldItem`, and all world items (`item_suit`, `item_battery`, `item_healthkit`, `item_antidote`, `item_security`, `item_longjump`).
4. THE System SHALL update `cl_dll/hl/hl_weapons.cpp` and all weapon/item translation units to include `weapons.h`, `projectiles.h`, and `items.h`.
5. THE System SHALL delete all 22 obsolete micro-headers.

### Requirement 2: Studio Model & Render Pipeline Consolidation

**User Story:** As a client graphics developer, I want the studio model and view rendering subsystems consolidated into cohesive files, so that fragmented helper methods and tiny callbacks do not clutter the source tree.

#### Acceptance Criteria

1. THE System SHALL reintegrate `studio_render_attachments.cpp` and `GameStudioModelRenderer.cpp/.h` directly into `cl_dll/studio/StudioModelRenderer.cpp` and `StudioModelRenderer.h`.
2. THE System SHALL reintegrate `cl_dll/render/tri.cpp/.h` and `view_bob.cpp` into `cl_dll/render/view.cpp` and `view_camera.cpp`.
3. THE System SHALL delete 5 obsolete micro-files (`studio_render_attachments.cpp`, `GameStudioModelRenderer.cpp`, `GameStudioModelRenderer.h`, `tri.cpp`, `tri.h`).

### Requirement 3: HUD Status Indicators & Utility Dispatchers Consolidation

**User Story:** As a HUD developer, I want player suit indicators and minor HUD elements grouped into unified translation units, so that each minor icon does not require a full compilation pipeline.

#### Acceptance Criteria

1. THE System SHALL consolidate `battery.cpp`, `flashlight.cpp`, `geiger.cpp`, `train.cpp`, and `status_icons.cpp` into `cl_dll/hud/hud_indicators.cpp`.
2. THE System SHALL reintegrate `hud_update.cpp` and `hud_msg.cpp` directly into `cl_dll/hud/hud.cpp` and `cl_dll/hud/hud_redraw.cpp`.
3. THE System SHALL delete 7 obsolete micro-files.

### Requirement 4: Environmental Effects & Screen Systems Consolidation

**User Story:** As a map entity developer, I want environmental and screen effects consolidated into thematic modules, so that world entity logic is organized and maintainable.

#### Acceptance Criteria

1. THE System SHALL consolidate `effects_beverage.*`, `effects_shooters.*`, `effects_env.*`, `env_global.*`, `env_spark.*`, `explode.*`, and `airtank.cpp` into `dlls/systems/effects_environment.cpp` and `effects_environment.h`.
2. THE System SHALL consolidate `effects_screen.*`, `info_intermission.cpp`, and `revertsaved.cpp` into `dlls/systems/effects_screen.cpp` and `effects_screen.h`.
3. THE System SHALL delete 14 obsolete micro-files.

### Requirement 5: World Brush Entities & Transportation Systems Consolidation

**User Story:** As a world systems developer, I want brush model entities and track transportation systems consolidated into cohesive modules, eliminating orphan files.

#### Acceptance Criteria

1. THE System SHALL reintegrate `conveyor.cpp/.h` and `guntarget.cpp/.h` into `dlls/world/bmodels.cpp` and `bmodels.h`.
2. THE System SHALL reintegrate `trackchange.cpp/.h` into `dlls/world/trains.cpp` and `trains.h`.
3. THE System SHALL delete 6 obsolete micro-files.

### Requirement 6: VGUI Controls Suite & Viewport Menus Consolidation

**User Story:** As a UI developer, I want a unified VGUI controls header and consolidated Viewport implementation, eliminating repeated 18-line include blocks and split viewport files.

#### Acceptance Criteria

1. THE System SHALL create `cl_dll/vgui/vgui_controls.h` packaging standard `<VGUI_*.h>` headers, and include it in `vgui_int.h`.
2. THE System SHALL reintegrate `vgui_viewport_menus.cpp` and `vgui_viewport_messages.cpp` into `cl_dll/vgui/vgui_TeamFortressViewport.cpp`.
3. THE System SHALL consolidate secondary dialogs (`vgui_ControlConfigPanel.*`, `vgui_MOTDWindow.cpp`).
4. THE System SHALL delete 5 obsolete micro-files.

### Requirement 7: Ambient & Cinematic NPCs Consolidation

**User Story:** As an AI developer, I want static, corpse, and ambient NPCs grouped into a single ambient monster module, keeping monster files clean.

#### Acceptance Criteria

1. THE System SHALL reintegrate `hgrunt_repel.cpp` into `dlls/monsters/hgrunt.cpp` and `hgrunt.h`.
2. THE System SHALL consolidate `monster_deadhev.*`, `genericmonster.*`, and `rat.*` into `dlls/monsters/monsters_ambient.cpp` and `monsters_ambient.h`.
3. THE System SHALL delete 6 obsolete micro-files.

### Requirement 8: Server Sound Engine Consolidation

**User Story:** As an audio system developer, I want server sound emitters and speech DSP modules consolidated into unified sound units, improving audio engine cohesion.

#### Acceptance Criteria

1. THE System SHALL consolidate `sound_dsp.cpp`, `sound_speaker.cpp`, and `sound_sentences.cpp` into `dlls/systems/sound_ambient.cpp` and `dlls/systems/sound.cpp`.
2. THE System SHALL delete 4 obsolete micro-files.

### Requirement 9: Client Event API Suite & Hooking Consolidation

**User Story:** As a client networking developer, I want client event hooks and engine API includes packaged into clean headers, eliminating repeated 10-line engine include blocks.

#### Acceptance Criteria

1. THE System SHALL package client engine event headers into `cl_dll/events/eventscripts.h`.
2. THE System SHALL reintegrate `events.cpp` and `hl_events.cpp` into `cl_dll/events/ev_common.cpp` or `ev_hldm.cpp`.
3. THE System SHALL reintegrate `hl_objects.cpp` into `cl_dll/hl/hl_baseentity.cpp`.
4. THE System SHALL delete 3 obsolete micro-files.

### Requirement 10: Build Systems Synchronization & Zero-Regression Verification

**User Story:** As a release engineer, I want all build systems (Visual Studio 2019 Win32 Release/Debug, Linux Makefiles x86) fully synchronized, so that the refactored code compiles with 0 errors.

#### Acceptance Criteria

1. THE System SHALL update `projects/vs2019/hldll.vcxproj` and `hldll.vcxproj.filters` matching all added, consolidated, and removed server files.
2. THE System SHALL update `projects/vs2019/hl_cdll.vcxproj` and `hl_cdll.vcxproj.filters` matching all added, consolidated, and removed client files.
3. THE System SHALL update `linux/Makefile.hldll` and `linux/Makefile.hl_cdll`.
4. THE System SHALL compile locally with MSBuild on Win32 Release producing 0 errors.
5. THE System SHALL pass all automated GitHub Actions CI checks on Windows and Linux.
