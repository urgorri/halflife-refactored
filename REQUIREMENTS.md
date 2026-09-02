# Requirements Document - GoldSrc Refactor Master Roadmap

## Introduction

This document specifies the complete set of architectural and modularization requirements for the Half-Life GoldSrc DLL codebase (Server DLL `hl.dll`, Client DLL `client.dll`, and Shared Engine components `pm_shared`, `game_shared`). The goal is to bring the remaining monolithic subsystems into full compliance with `AGENTS.md` while preserving 100% behavior and binary protocol compatibility.

## Glossary

- **GoldSrc**: The game engine powering Half-Life 1 and related mods.
- **PM (Player Move)**: Shared client-server player movement and physics prediction system (`pm_shared/`).
- **View Pipeline**: Client-side camera matrix calculation, view model bobbing, and post-processing screen tints (`cl_dll/render/view.cpp`).
- **GameRules**: Server-side game session coordinator handling spawning, scoring, item respawns, and team logic (`dlls/gameplay/`).
- **Sound System**: Environmental sound emission, DSP effects, sound entities for AI listening, and speech sentences (`dlls/systems/sound.cpp`).
- **Combat Core**: Damage application, blast radius calculations, blood particles, and entity gibbing (`dlls/core/combat.cpp`).
- **Tactical Monsters**: Advanced NPC AI with squad behavior, radio chatter, weapon switching, and interactive dialogue (`dlls/monsters/hgrunt.cpp`, `scientist.cpp`, `talkmonster.cpp`).
- **VGUI Viewport**: Client UI overlay managing menus, scoreboard, and server browser (`cl_dll/vgui/`).

---

## Requirements

### Requirement 1: Shared Movement & Physics Decomposition (PM Shared)

**User Story:** As an engine developer, I want player movement physics separated into dedicated environmental and state modules, so that walking, jumping, swimming, ladder climbing, and crouching logic can be maintained independently without risk of regressions.

#### Acceptance Criteria

1. THE `pm_shared` subsystem SHALL retain 100% mathematical equivalence for all client prediction and server physics.
2. WHEN moving on the ground, THE `pm_move_ground` module SHALL calculate friction, acceleration, and step-climbing.
3. WHEN in air, THE `pm_move_air` module SHALL enforce air acceleration, gravity, bunnyhop limits, and fall damage triggers.
4. WHEN in liquid, THE `pm_move_water` module SHALL manage buoyancy, swimming velocity, and surface exit velocity.
5. WHEN interacting with ladders, THE `pm_ladder` module SHALL compute mounting, climbing angles, and dismounting bounds.
6. WHEN crouching or transitioning stances, THE `pm_duck` module SHALL resize bounding hulls and interpolate eye offsets smoothly.
7. WHEN colliding with world geometry or entities, THE `pm_hull` module SHALL execute trace hulls and push resolutions.
8. THE `pm_shared.c` coordinator SHALL orchestrate `PM_PlayerMove` and dispatch state updates to the submodules.

### Requirement 2: Client View & Rendering Pipeline Decomposition

**User Story:** As a client systems engineer, I want the camera and screen rendering logic partitioned into specialized modules, so that view matrices, viewmodel bobbing, and screen tint effects are clearly decoupled.

#### Acceptance Criteria

1. THE `view_camera` module SHALL compute first-person angles, third-person chase camera vectors, and spectator interpolation.
2. WHEN the player walks, sprints, or takes damage, THE `view_bob` module SHALL calculate bobbing, sway, and punch-angle offsets.
3. WHEN screen transitions, underwater rendering, or damage flashes occur, THE `view_effects` module SHALL render blend tints.
4. THE `view.cpp` coordinator SHALL establish projection matrices and crosshair drawing without duplicating sub-pipeline logic.
5. THE build system SHALL exclude obsolete SDK sample files (`GameStudioModelRenderer_Sample.*`) from the active tree.

### Requirement 3: Multiplayer GameRules & Session Management Modularization

**User Story:** As a gameplay programmer, I want multiplayer game rules separated into distinct functional components, so that spawning, item timers, scoring, and team balancing are independently structured.

#### Acceptance Criteria

1. WHEN a player requests a spawn, THE `gamerules_spawn` module SHALL select optimal spawn points and prevent telefrag collisions.
2. WHEN items or weapons are collected, THE `gamerules_items` module SHALL manage respawn timers and weapon drop rules.
3. WHEN players score kills, die, or commit teamkills, THE `gamerules_scoring` module SHALL calculate frag tallies and broadcast death notices.
4. WHERE teamplay mode is active, THE `gamerules_teamplay` module SHALL enforce team assignments, skin locking, and auto-balance.
5. THE `multiplay_gamerules.cpp` core SHALL dispatch event callbacks to these modules while preserving network queries and cvar bindings.

### Requirement 4: Environmental Sound & Speech Sentence Engine Decomposition

**User Story:** As an audio engineer, I want ambient sounds, AI audio entities, speech sentence parsing, and DSP effects separated into distinct source units, so that sound playback and AI sensory systems are decoupled.

#### Acceptance Criteria

1. THE `sound_ambient` module SHALL manage `ambient_generic` playback, dynamic volume attenuation, and loop cycling.
2. WHEN weapon sounds or monster footsteps occur, THE `sound_ai_ent` module SHALL register `CSoundEnt` sensory pulses for NPC listeners.
3. THE `sound_sentences` module SHALL parse `sentences.txt`, maintain LRU sentence pools, and sequence phonetic speech groups.
4. THE `sound_dsp` module SHALL compute room reverb presets and environmental audio filters based on player location.
5. THE `sound.cpp` coordinator SHALL provide top-level precache helpers and sound channel dispatchers.

### Requirement 5: Combat Damage & Server Utility Decomposition

**User Story:** As a core systems developer, I want combat damage calculation, gibbing, and server utility functions separated into focused units, so that core entity operations are clean and modular.

#### Acceptance Criteria

1. THE `combat_damage` module SHALL compute damage types, armor absorption curves, and explosive radius falloff.
2. WHEN fatal damage occurs, THE `combat_gib` module SHALL compute velocity vectors, spawn gore chunks, and trigger blood particle effects.
3. THE `util_math` module SHALL provide coordinate transformations, aiming vectors, and matrix operations.
4. THE `util_trace` module SHALL encapsulate world raycasts, point contents tests, and line-of-sight checks.
5. THE `combat.cpp` and `util.cpp` modules SHALL retain high-level entity interfaces without circular dependencies.

### Requirement 6: Tactical Monster AI & Speech Dialogue Modularization

**User Story:** As an AI developer, I want complex monsters like the Human Grunt, Scientist, and TalkMonster base classes separated into distinct tactical, speech, and medical modules, so that individual entity classes stay within reasonable LOC limits.

#### Acceptance Criteria

1. THE `hgrunt_combat` module SHALL evaluate weapon selection (MP5 vs Shotgun), fire schedules, and grenade trajectories.
2. THE `hgrunt_squad` module SHALL coordinate squad fallback, cover points, and radio chatter dialogue.
3. THE `hgrunt_repel` module SHALL isolate `hgrunt_repel` and `dead_grunt` entities from the primary combat AI.
4. THE `talkmonster_speech` module SHALL handle dynamic greetings, following agreements/declines, and panic vocalizations.
5. THE `scientist_heal` module SHALL govern player health diagnostic checks, syringe animation sequences, and health kit dispensing.

### Requirement 7: Client VGUI Viewport Decomposition

**User Story:** As a UI developer, I want the client VGUI viewport separated into dedicated panel managers, so that menu command handling, server browsing, and scoreboard rendering are decoupled.

#### Acceptance Criteria

1. THE `vgui_viewport_serverbrowser` module SHALL encapsulate server list fetching, ping displays, and connection modals.
2. THE `vgui_viewport_menus` module SHALL handle command menus, team selection dialogs, and class pickers.
3. THE `vgui_TeamFortressViewport.cpp` coordinator SHALL handle screen resize events, mouse state transitions, and canvas layouts.

### Requirement 8: Build Verification & Cross-Platform Integrity

**User Story:** As a release engineer, I want all new modular source files synchronized with Visual Studio project files and Linux makefiles, so that builds compile cleanly on both Windows (MSVC) and Linux (GCC/Clang) with 0 errors.

#### Acceptance Criteria

1. ALL newly extracted client files SHALL be registered in `projects/vs2019/hl_cdll.vcxproj`, `projects/vs2019/hl_cdll.vcxproj.filters`, and `linux/Makefile.hl_cdll`.
2. ALL newly extracted server/shared files SHALL be registered in `projects/vs2019/hldll.vcxproj`, `projects/vs2019/hldll.vcxproj.filters`, and `linux/Makefile.hldll`.
3. MSBuild and make pipelines SHALL produce `client.dll` and `hl.dll` with 0 compilation errors.
