# Design Document: Pragmatic Modularization & Code Deduplication

## Overview

This design document outlines the technical architecture for consolidating over-modularized entity classes, eliminating cross-weapon ammo boilerplate via reusable patterns, and separating multi-entity source files into focused modules, in direct compliance with the refined [AGENTS.md](file:///E:/Dev/urgorri/halflife-refactored/AGENTS.md) standards.

---

## Component Architecture & System Boundaries

### Component Map

| Component ID | Module / File | Responsibility | Relationships |
| :--- | :--- | :--- | :--- |
| **COMP-1** | [`dlls/monsters/hgrunt.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/monsters/hgrunt.cpp) | Consolidated `CHGrunt` lifecycle, tactical squad AI, and weapon combat. | Derives from `CSquadMonster`. |
| **COMP-2** | [`dlls/monsters/hgrunt_repel.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/monsters/hgrunt_repel.cpp) | Isolated `CHGruntRepel` rappel mechanics and `CDeadHGrunt` decorative prop. | Spawns `monster_human_grunt`. |
| **COMP-3** | [`dlls/monsters/scientist.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/monsters/scientist.cpp) | Consolidated `CScientist` behavioral AI, panic schedules, heal routines, and sitting/dead variants. | Derives from `CTalkMonster`. |
| **COMP-4** | [`dlls/ai/talkmonster.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/ai/talkmonster.cpp) | Consolidated `CTalkMonster` base follower AI, friend lookup, and speech scheduling. | Base class for Barney, Scientist. |
| **COMP-5** | [`dlls/weapons/ammo_base.h`](file:///E:/Dev/urgorri/halflife-refactored/dlls/weapons/ammo_base.h) | Reusable factory macros and templates for ammo entity registration. | Included by all `weapon_*.cpp`. |
| **COMP-6** | [`dlls/weapons/weapon_box.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/weapons/weapon_box.cpp) | `CWeaponBox` item container, weapon packing, and ammo collection. | Used on player death / drop. |
| **COMP-7** | [`dlls/weapons/item_base.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/weapons/item_base.cpp) | `CBasePlayerItem` ground physics, materialization, and touch rules. | Base class for `CBasePlayerWeapon`. |
| **COMP-8** | [`dlls/systems/effects_*.cpp`](file:///E:/Dev/urgorri/halflife-refactored/dlls/systems) | Isolated visual entities (`effects_beam.cpp`, `effects_lightning.cpp`, `effects_laser.cpp`, `effects_glow.cpp`, `effects_sprite.cpp`). | Independent map entities. |
| **COMP-9** | [`cl_dll/input/input_hardware.cpp`](file:///E:/Dev/urgorri/halflife-refactored/cl_dll/input/input_hardware.cpp) | Low-level OS/DirectInput mouse and joystick polling. | Feeds raw axes to `input.cpp`. |

---

## Detailed Component Specifications

### 1. Monster Consolidation Architecture

```
   BEFORE: Over-Fragmented Class Methods             AFTER: Cohesive Entity File
  ┌─────────────────────────────────────┐          ┌────────────────────────────┐
  │ hgrunt.cpp        (Core lifecycle)  │          │                            │
  │ hgrunt_combat.cpp (Combat methods)  │  ──────► │ hgrunt.cpp (CHGrunt class) │
  │ hgrunt_squad.cpp  (Squad schedules) │          │                            │
  └─────────────────────────────────────┘          └────────────────────────────┘
  ┌─────────────────────────────────────┐          ┌────────────────────────────┐
  │ hgrunt_repel.cpp (Repel & Prop)     │  ──────► │ hgrunt_repel.cpp (Repel)   │
  └─────────────────────────────────────┘          └────────────────────────────┘
```

- **Consolidation Pattern**:
  - `CHGrunt`'s member methods from `hgrunt_combat.cpp` and `hgrunt_squad.cpp` are moved back into `hgrunt.cpp`.
  - `hgrunt_repel.cpp` remains dedicated exclusively to `CHGruntRepel` and `CDeadHGrunt`.
  - `scientist.cpp` absorbs `scientist_heal.cpp`, reuniting `CanHeal`/`Heal` and sitting/dead variants.
  - `talkmonster.cpp` absorbs `talkmonster_speech.cpp`, eliminating unnatural `extern Schedule_t slIdleResponse[];` linkage across files.

---

### 2. Reusable Ammo Engine Architecture (`dlls/weapons/ammo_base.h`)

Currently, 13 weapon files duplicate this pattern verbatim:

```cpp
// Boilerplate duplicated across all 13 weapon files:
class CCrossbowAmmo : public CBasePlayerAmmo {
    void Spawn( void ) { Precache(); SET_MODEL( ENT( pev ), "models/w_crossbow_clip.mdl" ); CBasePlayerAmmo::Spawn(); }
    void Precache( void ) { PRECACHE_MODEL( "models/w_crossbow_clip.mdl" ); PRECACHE_SOUND( "items/9mmclip1.wav" ); }
    BOOL AddAmmo( CBaseEntity *pOther ) {
        int iResult = ( pOther->GiveAmmo( AMMO_CROSSBOWCLIP_GIVE, "bolts", _MAX_CARRY ) != -1 );
        if ( iResult ) EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM );
        return iResult;
    }
};
LINK_ENTITY_TO_CLASS( ammo_crossbow, CCrossbowAmmo );
```

#### Reusable Design Pattern:

Define a unified template/macro in `ammo_base.h`:

```cpp
#ifndef WEAPONS_AMMO_BASE_H
#define WEAPONS_AMMO_BASE_H

#include "weapons/weapon_base.h"

#define IMPLEMENT_SIMPLE_AMMO( className, entityName, modelPath, ammoName, giveAmount, maxCarry, soundPath ) \
class className : public CBasePlayerAmmo { \
public: \
    void Spawn( void ) { Precache(); SET_MODEL( ENT( pev ), modelPath ); CBasePlayerAmmo::Spawn(); } \
    void Precache( void ) { PRECACHE_MODEL( modelPath ); PRECACHE_SOUND( soundPath ); } \
    BOOL AddAmmo( CBaseEntity *pOther ) { \
        int iResult = ( pOther->GiveAmmo( giveAmount, ammoName, maxCarry ) != -1 ); \
        if ( iResult ) EMIT_SOUND( ENT( pev ), CHAN_ITEM, soundPath, 1, ATTN_NORM ); \
        return iResult; \
    } \
}; \
LINK_ENTITY_TO_CLASS( entityName, className );

#endif // WEAPONS_AMMO_BASE_H
```

This reusable abstraction eliminates over 350 lines of duplicate copy-pasted class boilerplate across `dlls/weapons/`.

---

### 3. Decoupling `weapon_base.cpp` (1,398 LOC)

- **`dlls/weapons/weapon_box.cpp`**: Extracts `CWeaponBox` and its methods (`Precache`, `Spawn`, `Kill`, `Touch`, `PackWeapon`, `PackAmmo`, `GiveAmmo`, `HasWeapon`, `IsEmpty`).
- **`dlls/weapons/item_base.cpp`**: Extracts `CBasePlayerItem` and `CBasePlayerAmmo` ground physics (`FallInit`, `FallThink`, `Materialize`, `AttemptToMaterialize`, `CheckRespawn`, `Respawn`, `DefaultTouch`, `DestroyItem`, `AddToPlayer`, `Drop`, `Kill`, `Holster`, `AttachToPlayer`).
- **`dlls/weapons/weapon_base.cpp`**: Retains core `CBasePlayerWeapon` animation, ammo consumption, holster, deploy, prediction update, and client data sync.

---

### 4. Environmental Effects Decomposition (`dlls/systems/effects.cpp`)

- **`effects_beam.cpp`**: Core `CBeam` entity and math methods (`BeamCreate`, `PointsInit`, `PointEntInit`, `EntsInit`, `HoseInit`, `RelinkBeam`, `DoSparks`).
- **`effects_lightning.cpp`**: `CLightning` (`env_lightning`, `env_beam`) and `CTripBeam` (`trip_beam`).
- **`effects_laser.cpp`**: `CLaser` (`env_laser`) targeting and beam projection.
- **`effects_glow.cpp`**: `CGlow` (`env_glow`) sprite scale and visibility logic.
- **`effects_sprite.cpp`**: `CSprite` (`env_sprite`) frame animation, transparency, and rendering.

---

## Build System & Compatibility

- Update Visual Studio project files (`hl_cdll.vcxproj`, `hl_cdll.vcxproj.filters`, `hldll.vcxproj`, `hldll.vcxproj.filters`).
- Update Linux Makefiles (`Makefile.hl_cdll`, `Makefile.hldll`).
- Zero changes to entity names, save/restore tables, or network messages.
