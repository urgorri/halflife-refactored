# Design Document

## Overview

This design document outlines the architecture, component relationships, header consolidation strategy, and file restructuring for the GoldSrc DLL refactoring. It eliminates over 70 redundant micro-files and micro-headers while preserving complete binary/runtime compatibility and entity dispatching.

---

## System Architecture

### Component Map

| Component ID | Name | Type | Responsibility | Interfaces With |
|---|---|---|---|---|
| COMP-WPN | Weapons & Projectiles Suite | Server/Client DLL | Weapon classes, firing logic, projectiles, damage models | GameRules, Player, ClientHUD |
| COMP-ITM | World Items Suite | Server DLL | Suit, batteries, healthkits, antidotes, security items | Player, GameRules |
| COMP-STU | Studio Model Renderer | Client DLL | Skeletal bone matrices, submodels, attachments, hitbox rendering | EngineStudioAPI, ClientView |
| COMP-HUD | HUD Indicators & Utilities | Client DLL | Battery, flashlight, geiger, train, status icons, HUD dispatch | Enginefuncs, PlayerState |
| COMP-ENV | Environmental Effects & Screen | Server DLL | Dispensers, shooters, sparks, global states, blood, explosions, screen fade/shake | World, Entities, Player |
| COMP-WRD | World & Brush Entities | Server DLL | Brush walls, conveyors, gun targets, train path changes, tracks | Trains, Triggers, Physics |
| COMP-VGUI | VGUI Controls & Viewport | Client DLL | TeamFortress Viewport, menus, class selection, score panel, config | VGUI Framework, Engine |
| COMP-NPC | Ambient & Corpse Monsters | Server DLL | Dead HEVs, generic cinematic dummies, ambient critters, repel grunts | TalkMonster, MonsterAI |
| COMP-SND | Server Sound Engine | Server DLL | DSP sound effects, ambient sentences, speakers | SoundEngine, Entities |
| COMP-EVT | Client Events & Hooking | Client DLL | Bullet events, explosive events, energy weapons, base entity dispatch | ClientEngine, EventAPI |

---

## Header Consolidation Architecture

### 1. Weapons & Projectiles (`dlls/weapons/`)

```
                  ┌────────────────────────┐
                  │    weapon_base.h       │
                  │ (CBasePlayerWeapon/Item│
                  └───────────┬────────────┘
                              │
            ┌─────────────────┴─────────────────┐
            │                                   │
            ▼                                   ▼
┌────────────────────────┐         ┌────────────────────────┐
│      weapons.h         │         │     projectiles.h      │
│  (CGlock, CMP5,        │         │ (CCrossbowBolt,        │
│   CShotgun, CPython,   │         │  CGrenade, CHornet,    │
│   CCrossbow, CCrowbar, │         │  CRpgRocket, CSatchel, │
│   CEgon, CGauss,       │         │  CSnark, CTripmine)    │
│   CHandGrenade,        │         └────────────────────────┘
│   CHornetgun, CRpg,    │
│   CSatchel, CSnark,    │
│   CSpraycan, CTripmine)│
└────────────────────────┘
```

### 2. VGUI Controls Suite (`cl_dll/vgui/`)

```
[VGUI Framework Headers: Cursor, Frame, Label, Surface, Panel, Button, Signal, Scheme, Font, App...]
                                  │
                                  ▼
                     ┌────────────────────────┐
                     │    vgui_controls.h     │
                     └────────────┬───────────┘
                                  │
                                  ▼
                     ┌────────────────────────┐
                     │       vgui_int.h       │
                     └────────────┬───────────┘
                                  │
      ┌───────────────────────────┼───────────────────────────┐
      ▼                           ▼                           ▼
[TeamFortressViewport]     [ScorePanel]              [CommandMenuPanel]
```

---

## File Consolidation Inventory

### 1. Weapons & Projectiles (`dlls/weapons/` & `dlls/items/`)
- **Created**:
  - `dlls/weapons/weapons.h`: Declares 15 weapon classes.
  - `dlls/weapons/projectiles.h`: Declares 7 projectile classes.
  - `dlls/items/items.h`: Replaces `item_base.h`, declares `CItem`, `CWorldItem`, and world items.
- **Removed (22 micro-headers)**:
  - `weapon_glock.h`, `weapon_mp5.h`, `weapon_shotgun.h`, `weapon_python.h`, `weapon_crossbow.h`, `weapon_crowbar.h`, `weapon_egon.h`, `weapon_gauss.h`, `weapon_handgrenade.h`, `weapon_hornetgun.h`, `weapon_rpg.h`, `weapon_satchel.h`, `weapon_snark.h`, `weapon_spraycan.h`, `weapon_tripmine.h`
  - `projectile_bolt.h`, `projectile_grenade.h`, `projectile_hornet.h`, `projectile_rocket.h`, `projectile_satchel.h`, `projectile_snark.h`, `projectile_tripmine.h`
  - `item_base.h`

### 2. Studio Model & Render Pipeline (`cl_dll/studio/` & `cl_dll/render/`)
- **Consolidated into**:
  - `cl_dll/studio/StudioModelRenderer.cpp` & `StudioModelRenderer.h`: Absorbs `studio_render_attachments.cpp` and `GameStudioModelRenderer.cpp/.h`.
  - `cl_dll/render/view.cpp`: Absorbs `tri.cpp/.h` and `view_bob.cpp`.
- **Removed (5 micro-files)**:
  - `studio_render_attachments.cpp`, `GameStudioModelRenderer.cpp`, `GameStudioModelRenderer.h`, `tri.cpp`, `tri.h`

### 3. HUD Indicators & Utility Dispatchers (`cl_dll/hud/`)
- **Consolidated into**:
  - `cl_dll/hud/hud_indicators.cpp`: Absorbs `battery.cpp`, `flashlight.cpp`, `geiger.cpp`, `train.cpp`, `status_icons.cpp`.
  - `cl_dll/hud/hud.cpp` & `hud_redraw.cpp`: Absorbs `hud_update.cpp` and `hud_msg.cpp`.
- **Removed (7 micro-files)**:
  - `battery.cpp`, `flashlight.cpp`, `geiger.cpp`, `train.cpp`, `status_icons.cpp`, `hud_update.cpp`, `hud_msg.cpp`

### 4. Environmental Effects & Screen Systems (`dlls/systems/`)
- **Consolidated into**:
  - `dlls/systems/effects_environment.cpp` & `effects_environment.h`: Absorbs `effects_beverage.*`, `effects_shooters.*`, `effects_env.*`, `env_global.*`, `env_spark.*`, `explode.*`, `airtank.cpp`.
  - `dlls/systems/effects_screen.cpp` & `effects_screen.h`: Absorbs `effects_screen.*`, `info_intermission.cpp`, `revertsaved.cpp`.
- **Removed (14 micro-files)**:
  - `effects_beverage.cpp`, `effects_beverage.h`, `effects_shooters.cpp`, `effects_shooters.h`, `effects_env.cpp`, `effects_env.h`, `env_global.cpp`, `env_global.h`, `env_spark.cpp`, `env_spark.h`, `explode.cpp`, `explode.h`, `airtank.cpp`, `info_intermission.cpp`, `revertsaved.cpp`

### 5. World Brush Entities & Trains (`dlls/world/`)
- **Consolidated into**:
  - `dlls/world/bmodels.cpp` & `bmodels.h`: Absorbs `conveyor.cpp/.h` and `guntarget.cpp/.h`.
  - `dlls/world/trains.cpp` & `trains.h`: Absorbs `trackchange.cpp/.h`.
- **Removed (6 micro-files)**:
  - `conveyor.cpp`, `conveyor.h`, `guntarget.cpp`, `guntarget.h`, `trackchange.cpp`, `trackchange.h`

### 6. VGUI Controls & Viewport Menus (`cl_dll/vgui/`)
- **Consolidated into**:
  - `cl_dll/vgui/vgui_TeamFortressViewport.cpp`: Absorbs `vgui_viewport_menus.cpp` and `vgui_viewport_messages.cpp`.
  - `cl_dll/vgui/vgui_controls.h`: Unified VGUI includes.
- **Removed (5 micro-files)**:
  - `vgui_viewport_menus.cpp`, `vgui_viewport_messages.cpp`, `vgui_ControlConfigPanel.cpp`, `vgui_ControlConfigPanel.h`, `vgui_MOTDWindow.cpp`

### 7. Ambient & Corpse Monsters (`dlls/monsters/`)
- **Consolidated into**:
  - `dlls/monsters/hgrunt.cpp` & `hgrunt.h`: Absorbs `hgrunt_repel.cpp`.
  - `dlls/monsters/monsters_ambient.cpp` & `monsters_ambient.h`: Absorbs `monster_deadhev.*`, `genericmonster.*`, `rat.*`.
- **Removed (6 micro-files)**:
  - `hgrunt_repel.cpp`, `monster_deadhev.cpp`, `monster_deadhev.h`, `genericmonster.cpp`, `genericmonster.h`, `rat.cpp`, `rat.h`

### 8. Server Sound Engine (`dlls/systems/sound_*`)
- **Consolidated into**:
  - `dlls/systems/sound_ambient.cpp` & `sound.cpp`: Absorbs `sound_dsp.cpp`, `sound_speaker.cpp`, `sound_sentences.cpp`.
- **Removed (4 micro-files)**:
  - `sound_dsp.cpp`, `sound_speaker.cpp`, `sound_sentences.cpp`, `sound_local.h`

### 9. Client Event API Suite & Hooking (`cl_dll/events/` & `cl_dll/hl/`)
- **Consolidated into**:
  - `cl_dll/events/ev_common.cpp`: Absorbs `events.cpp` and `hl_events.cpp`.
  - `cl_dll/hl/hl_baseentity.cpp`: Absorbs `hl_objects.cpp`.
  - `cl_dll/events/eventscripts.h`: Packages client engine headers.
- **Removed (3 micro-files)**:
  - `events.cpp`, `hl_events.cpp`, `hl_objects.cpp`

---

## Build System Synchronization

All project and makefile definitions are updated in sync:
- `projects/vs2019/hldll.vcxproj` & `hldll.vcxproj.filters`
- `projects/vs2019/hl_cdll.vcxproj` & `hl_cdll.vcxproj.filters`
- `linux/Makefile.hldll` & `linux/Makefile.hl_cdll`

## Verification Strategy

1. **Local MSBuild**: Build Win32 Release on `hldll.vcxproj` and `hl_cdll.vcxproj`, asserting 0 errors.
2. **GitHub Actions CI**: Automated validation for Windows x86 DLLs and Linux x86 DLLs on push / PR.
