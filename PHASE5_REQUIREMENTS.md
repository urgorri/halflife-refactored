# Requirements Document - Phase 5: Client and Bot Navigation Modularization

## Introduction

Phase 5 addresses high-complexity monolithic subsystems in the Client DLL (`cl_dll/`) and the shared Bot Navigation system (`game_shared/bot/nav_area.cpp`).

Key targets in this phase:
- `cl_dll/studio/StudioModelRenderer.cpp` (2,103 LOC): Handles studio model setup, bone transformations, lighting calculation, chrome shading, and submodel attachment positioning.
- `cl_dll/hud/hud_spectator.cpp` (1,945 LOC): Handles spectator camera modes, director command decoding, map overview radar rendering, and player list interfaces.
- `cl_dll/hud/ammo.cpp` (1,205 LOC): Handles ammo count rendering, weapon selection menus, and ammo pickup animation history.
- `cl_dll/events/ev_hldm.cpp` (1,775 LOC): Manages client-side weapon event playback, muzzle flashes, tracers, sound dispatch, and ejection brass for all multiplayer weapons.
- `game_shared/bot/nav_area.cpp` (5,191 LOC): Manages navigation mesh area geometry, boundary calculation, splitting/merging algorithms, hiding spots, sniper points, ladder connections, and staircase elevation analysis.

## Glossary

- **StudioModelRenderer**: The client-side renderer responsible for animating and rendering skeletal studio models (`.mdl`), transforming bones, and applying dynamic software/hardware lighting.
- **HUD Spectator**: The user interface and camera controller for spectator mode, managing overview mini-maps and director broadcasts (`DRC_CMD_EVENT`).
- **HUD Ammo**: The HUD component displaying current weapon ammo, reserve clips, and weapon slot selection menus.
- **EV HLDM**: Event system handling client-side predicted weapon fire animations, shell casings, dynamic light flashes, and sound playback.
- **Nav Area (CNavArea)**: A convex polygon in 3D space representing navigable walkable surface for AI bots, tracking approach points, tactical hiding spots, and neighbor connections.

## Requirements

### Requirement REQ-5.1: StudioModelRenderer Decomposition
**User Story:** As a graphics engineer, I want `StudioModelRenderer.cpp` modularized into distinct bone transformation, lighting/shading, and attachment calculation modules, so that skeletal rendering logic is maintainable and decoupled.

#### Acceptance Criteria
1. THE system SHALL decompose `cl_dll/studio/StudioModelRenderer.cpp` into:
   - `cl_dll/studio/studio_render_bones.cpp`: Bone setup, motion blending, and transformation matrix computation (`StudioSetUpTransform`, `StudioSetupBones`, `StudioSaveBones`, `StudioMergeBones`, `StudioSlerpBones`, `StudioCalcBoneAdj`, `StudioCalcBoneQuaterion`, `StudioCalcBonePosition`, `StudioCalcRotations`, `StudioPlayerBlend`, `StudioEstimateGait`, `StudioProcessGait`, `StudioFxTransform`, `StudioEstimateInterpolant`, `StudioEstimateFrame`).
   - `cl_dll/studio/studio_render_lighting.cpp`: Normal transformations, dynamic light accumulation, Lambertian diffuse, and chrome reflection calculations (`StudioDynamicLight`, `StudioLighting`, `StudioChrome`, `StudioSetupLighting`, `StudioLightParams`).
   - `cl_dll/studio/studio_render_attachments.cpp`: Sub-model attachment positioning and bodypart bone binding (`StudioCalcAttachments`, `StudioSetupAttachments`, `StudioSetupBodyparts`, `StudioSetupSubmodels`).
   - `cl_dll/studio/StudioModelRenderer.cpp` (core orchestrator): Render dispatch loop, precaching, and interface initialization (`Init`, `StudioDrawModel`, `StudioDrawPlayer`, `StudioRenderModel`, `StudioRenderFinal`, `StudioRenderFinal_Software`, `StudioRenderFinal_Hardware`).
2. THE skeletal rendering pipeline SHALL preserve identical bone matrix transformations, animation sequence blending, and vertex lighting values.

### Requirement REQ-5.2: HUD Spectator Modularization
**User Story:** As a UI developer, I want `hud_spectator.cpp` separated into director command decoding, overview radar projection, and spectator HUD menus, so that spectating features are cleanly organized.

#### Acceptance Criteria
1. THE system SHALL decompose `cl_dll/hud/hud_spectator.cpp` into:
   - `cl_dll/hud/hud_spectator_director.cpp`: Director network message decoding (`DirectorMessage`, `HandleDirectorEvent`, `FindBestPlayer`, `CheckCameraModes`).
   - `cl_dll/hud/hud_spectator_overview.cpp`: Map overview bitmap parsing, coordinate transformation, radar blip rendering, and player tracking (`DrawOverview`, `ParseOverviewFile`, `DrawOverviewEntities`, `DrawOverviewLayer`).
   - `cl_dll/hud/hud_spectator_menu.cpp`: Bottom control bar, player list dropdown, and camera mode buttons (`DrawSpectatorMenu`, `CheckSettings`, `DrawPlayerList`).
   - `cl_dll/hud/hud_spectator.cpp` (core orchestrator): Main spectator HUD lifecycle and dispatch (`Init`, `VidInit`, `Draw`, `Think`).
2. THE spectator interface SHALL retain full backward compatibility with engine director messages and HUD drawing callbacks.

### Requirement REQ-5.3: HUD Ammo & Weapon Selection Modularization
**User Story:** As a client UI developer, I want `ammo.cpp` divided into weapon selection logic, weapon resource handling, and ammo counter rendering, so that weapon HUD elements are isolated.

#### Acceptance Criteria
1. THE system SHALL decompose `cl_dll/hud/ammo.cpp` into:
   - `cl_dll/hud/hud_weapon_selection.cpp`: Weapon slot selection menu, fast weapon switch, and mouse/keyboard slot navigation (`SelectItem`, `SelectSlot`, `DrawWList`, `SlotInput`, `UserCmd_Slot1`..`UserCmd_Slot10`, `UserCmd_NextWeapon`, `UserCmd_PrevWeapon`).
   - `cl_dll/hud/ammo.cpp` (core): Primary/secondary ammo counter rendering, `WeaponsResource` management, and HUD user message callbacks (`Init`, `VidInit`, `Draw`, `MsgFunc_CurWeapon`, `MsgFunc_AmmoX`).
2. THE HUD ammo display and weapon switching animations SHALL remain visually identical and responsive to input commands.

### Requirement REQ-5.4: Weapon Event System (ev_hldm.cpp) Modularization
**User Story:** As an audio/visual effects engineer, I want `ev_hldm.cpp` categorized into bullet weapons, energy weapons, and explosive/projectile event modules, so that weapon effects can be individually maintained.

#### Acceptance Criteria
1. THE system SHALL decompose `cl_dll/events/ev_hldm.cpp` into:
   - `cl_dll/events/ev_bullets.cpp`: Bullet tracer effects, muzzle flash sprites, shell ejection, and bullet sounds (Glock, Shotgun, MP5, Python).
   - `cl_dll/events/ev_energy.cpp`: Dynamic lighting, beam effects, glow sprites, and audio loops (Gauss, Egon, Hornet).
   - `cl_dll/events/ev_explosives.cpp`: Projectile trajectory effects, explosion sprites, and view kick (Crossbow, RPG, Tripmine, Satchel, Snark, Crowbar).
   - `cl_dll/events/ev_hldm.cpp` (core): Event registration table (`EV_HLDM_Init`) and common helper functions (`EV_EjectBrass`, `EV_GetGunPosition`, `EV_HLDM_FireBullets`, `EV_TrainPitchAdjust`).
2. THE client-side event playback SHALL maintain exact prediction timing, sound attenuation, and particle/tracer visual fidelity.

### Requirement REQ-5.5: Bot Navigation Area (nav_area.cpp) Modularization
**User Story:** As a bot AI engineer, I want `nav_area.cpp` (5,191 LOC) partitioned into geometry, connections, tactical evaluation, and area lifecycle modules, so that navigation mesh computations are modular and scalable.

#### Acceptance Criteria
1. THE system SHALL decompose `game_shared/bot/nav_area.cpp` into:
   - `game_shared/bot/nav_area_geometry.cpp`: Quad mesh polygon calculation, boundary bounding boxes, Z-height elevation interpolation, and surface normal computation (`GetZ`, `ComputeNormal`, `IsOverlapping`, `GetClosestPointOnArea`, `Contains`, `GetDistanceSquaredToPoint`).
   - `game_shared/bot/nav_area_connect.cpp`: Directional connectivity (`ConnectTo`, `Disconnect`), ladder connections, area splitting (`SplitEdit`), and area merging (`MergeEdit`).
   - `game_shared/bot/nav_area_tactical.cpp`: Tactical analysis algorithms, hiding spot generation (`ComputeHidingSpots`), sniper spot computation, approach points, and encounter areas.
   - `game_shared/bot/nav_area.cpp` (core): `CNavArea` constructor, destructor, serialization (`Save`, `Load`), static list management, and property accessors.
2. THE navigation area algorithms SHALL produce identical mesh connectivity, hiding spots, and tactical evaluations.

### Requirement REQ-5.6: Build Manifest and Cross-Platform Validation
**User Story:** As a build engineer, I want all new client and bot source files registered in all solution and makefile targets, so that Windows client DLL (`client.dll`), Windows server DLL (`hldll.dll`), Linux client SO (`client.so`), and Linux server SO (`hl.so`) build cleanly.

#### Acceptance Criteria
1. THE system SHALL register all new files in `projects/vs2019/hl_cdll.vcxproj`, `projects/vs2019/hl_cdll.vcxproj.filters`, `projects/vs2019/hldll.vcxproj`, and `projects/vs2019/hldll.vcxproj.filters`.
2. THE system SHALL register all new files in `linux/Makefile.hl_cdll`, `dlls/Makefile`, and `linux/Makefile.hldll`.
3. ALL CI jobs across Windows x86 and Linux x86 SHALL pass with zero warnings or errors.
