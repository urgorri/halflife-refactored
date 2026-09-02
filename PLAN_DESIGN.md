# System Design Document - GoldSrc Refactor Master Architecture

## Overview

This document defines the comprehensive architecture and component decomposition plan to refactor the remaining monolithic modules in the Half-Life GoldSrc DLL repository into clean, focused, single-responsibility units as mandated by `AGENTS.md`. The design preserves 100% binary compatibility, memory layouts, network protocols, and mathematical behavior.

---

## System Architecture

### Component Map

| Component ID | Name | Subsystem | Responsibility | Interfaces With |
| :--- | :--- | :--- | :--- | :--- |
| **COMP-PM-1** | `pm_move_ground` | Physics (`pm_shared/`) | Friction, ground acceleration, step-up/down | `pm_shared.c`, `pm_hull` |
| **COMP-PM-2** | `pm_move_air` | Physics (`pm_shared/`) | Air acceleration, strafe dynamics, gravity | `pm_shared.c`, `pm_hull` |
| **COMP-PM-3** | `pm_move_water` | Physics (`pm_shared/`) | Buoyancy, water friction, swimming physics | `pm_shared.c`, `pm_hull` |
| **COMP-PM-4** | `pm_ladders` | Physics (`pm_shared/`) | Ladder mounting, climbing speed, dismount | `pm_shared.c` |
| **COMP-PM-5** | `pm_duck` | Physics (`pm_shared/`) | Duck transitions, ducking hull resizing | `pm_shared.c`, `pm_hull` |
| **COMP-PM-6** | `pm_hull` | Physics (`pm_shared/`) | Trace hulls, entity push, player collisions | `pm_shared.c` |
| **COMP-VIEW-1** | `view_camera` | Client Render (`cl_dll/render/`) | View angles, 3rd-person chase camera | `view.cpp`, `input` |
| **COMP-VIEW-2** | `view_bob` | Client Render (`cl_dll/render/`) | Weapon/eye bobbing, sway, roll, punch | `view.cpp` |
| **COMP-VIEW-3** | `view_effects` | Client Render (`cl_dll/render/`) | Screen blends, damage flash, water tint | `view.cpp` |
| **COMP-GR-1** | `gamerules_spawn` | Gameplay (`dlls/gameplay/`) | Spawn point selection, telefrag avoidance | `multiplay_gamerules.cpp` |
| **COMP-GR-2** | `gamerules_items` | Gameplay (`dlls/gameplay/`) | Item respawn timers, weapon drops | `multiplay_gamerules.cpp` |
| **COMP-GR-3** | `gamerules_scoring` | Gameplay (`dlls/gameplay/`) | Frag counters, suicide/teamkill penalties | `multiplay_gamerules.cpp` |
| **COMP-GR-4** | `gamerules_teamplay` | Gameplay (`dlls/gameplay/`) | Team balancing, model locking, teamplay | `teamplay_gamerules.cpp` |
| **COMP-SND-1** | `sound_ambient` | Systems (`dlls/systems/`) | `ambient_generic` playback, loop cycles | `sound.cpp` |
| **COMP-SND-2** | `sound_ai_ent` | Systems (`dlls/systems/`) | `CSoundEnt` AI hearing sensory pulses | `sound.cpp`, `dlls/ai/` |
| **COMP-SND-3** | `sound_sentences` | Systems (`dlls/systems/`) | Speech sentence parser and LRU pools | `sound.cpp` |
| **COMP-SND-4** | `sound_dsp` | Systems (`dlls/systems/`) | Environmental reverb & water audio DSP | `sound.cpp` |
| **COMP-CBT-1** | `combat_damage` | Core (`dlls/core/`) | Armor curves, damage types, blast radius | `combat.cpp` |
| **COMP-CBT-2** | `combat_gib` | Core (`dlls/core/`) | Decapitation, gib velocity, blood spray | `combat.cpp` |
| **COMP-UTIL-1** | `util_math` | Core (`dlls/core/`) | Angle vectors, matrix operations, math | `util.cpp` |
| **COMP-UTIL-2** | `util_trace` | Core (`dlls/core/`) | Tracelines, point contents, visibility | `util.cpp` |
| **COMP-AI-1** | `hgrunt_combat` | Monsters (`dlls/monsters/`) | Grunt weapon schedules and grenade fire | `hgrunt.cpp` |
| **COMP-AI-2** | `hgrunt_squad` | Monsters (`dlls/monsters/`) | Squad fallback, covering fire, radio voice | `hgrunt.cpp` |
| **COMP-AI-3** | `talkmonster_speech`| AI (`dlls/ai/`) | TalkMonster dialogue, greeting, banter | `talkmonster.cpp` |
| **COMP-AI-4** | `scientist_heal` | Monsters (`dlls/monsters/`) | Scientist syringe healing schedules | `scientist.cpp` |
| **COMP-VGUI-1** | `vgui_viewport_menus` | UI (`cl_dll/vgui/`) | Command menu, team/class picker dialogs | `vgui_TeamFortressViewport.cpp`|

---

## Data Flow Specifications

### 1. Player Physics Frame Flow (`pm_shared/`)

```
1. Engine → PM_PlayerMove(playermove_t *pmove): Set up global move context
2. PM_PlayerMove → PM_Duck: Evaluate ducking / un-ducking transition and select hull
3. PM_PlayerMove → PM_Ladder: If on ladder, apply ladder climbing physics
4. PM_PlayerMove → PM_WaterMove: If in water, apply swimming and buoyancy
5. PM_PlayerMove → PM_AirMove / PM_GroundMove: Apply air strafe or ground friction
6. PM_PlayerMove → PM_StepSlideMove: Trace step obstacles and slide against walls
7. PM_PlayerMove → Engine: Return final origin, velocity, and punchangles
```

### 2. Client Camera and View Frame Flow (`cl_dll/render/view.cpp`)

```
1. Engine → V_CalcRefdef(refdef_t *pparams): Receive player origin and view angles
2. V_CalcRefdef → view_camera: Compute 1st person eye offset or 3rd person chase camera
3. V_CalcRefdef → view_bob: Calculate weapon and viewmodel bobbing, roll, and sway
4. V_CalcRefdef → view_effects: Apply screen fades, damage flashes, and water distortion
5. V_CalcRefdef → Engine: Return reference definition and projection matrices
```

### 3. Monster Combat AI Decision Flow (`dlls/monsters/hgrunt.cpp`)

```
1. Monster Think → GetSchedule(): Evaluate sensory conditions (enemy visible, heard sound)
2. GetSchedule() → hgrunt_combat: Determine weapon range (Shotgun vs MP5) or grenade opportunity
3. GetSchedule() → hgrunt_squad: Check squad leader orders, covering fire, and radio callouts
4. RunTask() → Execute combat schedules and fire weapon traces
```

---

## Phased Implementation Decomposition

The refactoring will be executed across 7 self-contained, reviewable phases:

```
Phase 6: Shared Movement & Physics Engine (pm_shared/)
  ├── pm_move_ground.c (Friction, walking, step climbing)
  ├── pm_move_air.c    (Air acceleration, strafing, gravity)
  ├── pm_move_water.c  (Buoyancy, swimming, water surface)
  ├── pm_ladders.c     (Ladder detection, mount, climb)
  ├── pm_duck.c        (Ducking, un-ducking, hull switch)
  ├── pm_hull.c        (Trace hulls, push resolution)
  └── pm_shared.c      (Core coordinator and PM_PlayerMove)

Phase 7: Client View & Rendering Pipeline (cl_dll/render/)
  ├── view_camera.cpp  (Camera positioning, 3rd person chase)
  ├── view_bob.cpp     (Weapon/view bobbing, sway, roll)
  ├── view_effects.cpp (Screen blends, damage flash, water tint)
  └── view.cpp         (Core view setup & projection matrix)

Phase 8: Multiplayer GameRules & Session Management (dlls/gameplay/)
  ├── gamerules_spawn.cpp    (Spawn point selection & telefrag avoidance)
  ├── gamerules_items.cpp    (Item respawn timers & weapon drop rules)
  ├── gamerules_scoring.cpp  (Frag tallies & death notifications)
  ├── gamerules_teamplay.cpp (Team balance & model locking)
  └── multiplay_gamerules.cpp (Core rules dispatcher & thinker)

Phase 9: Environmental Audio & Voice Sentence Engine (dlls/systems/)
  ├── sound_ambient.cpp   (ambient_generic playback & loop cycles)
  ├── sound_ai_ent.cpp    (CSoundEnt sensory pulses for NPC hearing)
  ├── sound_sentences.cpp (sentences.txt parser & LRU speech pools)
  ├── sound_dsp.cpp       (Room reverb types & water DSP)
  └── sound.cpp           (Core sound manager & precache dispatch)

Phase 10: Server Core Combat & Utilities (dlls/core/)
  ├── combat_damage.cpp (Damage curves, armor absorption, blast radius)
  ├── combat_gib.cpp    (Gore chunks, decapitation, blood spray)
  ├── combat.cpp        (Core combat dispatch & trace attacks)
  ├── util_math.cpp     (Angle vectors, aiming helpers, coordinate math)
  ├── util_trace.cpp    (Tracelines, hull traces, point contents)
  └── util.cpp          (Top-level utility dispatcher)

Phase 11: Tactical Monster AI & Speech Dialogue (dlls/monsters/ & dlls/ai/)
  ├── hgrunt_combat.cpp     (Grunt weapon selection & grenade fire)
  ├── hgrunt_squad.cpp      (Squad tactical fallback & radio chatter)
  ├── hgrunt_repel.cpp      (hgrunt_repel & dead_grunt entities)
  ├── hgrunt.cpp            (Core Human Grunt AI schedules)
  ├── talkmonster_speech.cpp(TalkMonster dialogue & banter)
  ├── scientist_heal.cpp    (Scientist syringe heal sequence)
  └── scientist.cpp         (Core Scientist AI schedules)

Phase 12: Client VGUI Viewport Modernization (cl_dll/vgui/)
  ├── vgui_viewport_serverbrowser.cpp (Server browser & connection modal)
  ├── vgui_viewport_menus.cpp         (Command menu & team/class dialogs)
  └── vgui_TeamFortressViewport.cpp   (Core viewport layout & routing)
```

---

## Quality and Verification Standards

1. **Compilation Guarantee**: Every phase must build with **0 errors** on MSBuild (Win32 Release/Debug) and GCC/Clang (Linux makefiles).
2. **Behavior Preservation**: All calculations (physics, damage, trajectories, view angles) must maintain bit-identical logic with no altered constants or algorithms.
3. **No Circular Dependencies**: Submodules must include focused headers rather than monolithic cross-includes.
