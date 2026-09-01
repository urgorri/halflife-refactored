# Design Document - Phase 5: Client and Bot Navigation Modularization

## Introduction

Phase 5 addresses high-complexity monolithic subsystems across the Client DLL (`cl_dll/`) and shared Bot Navigation system (`game_shared/bot/`).

## System Architecture

### Component Map

| Component ID | Name | Type | Responsibility | Interfaces With |
|-------------|------|------|----------------|-----------------|
| COMP-CL-1 | Studio Bones Renderer (`studio_render_bones.cpp`) | Client Studio | Computes bone transformations, frame blending, and skeletal matrices | `CStudioModelRenderer`, Studio Engine |
| COMP-CL-2 | Studio Lighting Renderer (`studio_render_lighting.cpp`) | Client Studio | Computes dynamic light shading, normal calculations, and chrome reflections | `CStudioModelRenderer`, Engine Dynamic Lights |
| COMP-CL-3 | Studio Attachments (`studio_render_attachments.cpp`) | Client Studio | Evaluates sub-model attachment points and bone bindings | `CStudioModelRenderer` |
| COMP-CL-4 | Spectator Director (`hud_spectator_director.cpp`) | Client HUD | Decodes director commands (`DRC_CMD_EVENT`), targets, and camera modes | `CHudSpectator`, Engine User Messages |
| COMP-CL-5 | Spectator Overview (`hud_spectator_overview.cpp`) | Client HUD | Renders map overview radar, player markers, and map coordinates | `CHudSpectator`, Engine TriAPI |
| COMP-CL-6 | Spectator Menus (`hud_spectator_menu.cpp`) | Client HUD | Draws spectator control bar, player list, and settings UI | `CHudSpectator` |
| COMP-CL-7 | Weapon Selection (`hud_weapon_selection.cpp`) | Client HUD | Implements weapon selection HUD cursor, slots, and fastswitch logic | `CHudAmmo`, `WeaponsResource` |
| COMP-CL-8 | Bullet Weapon Events (`ev_bullets.cpp`) | Client Events | Renders bullet traces, muzzle flashes, and brass for Glock, Shotgun, MP5, Python | Engine Event System |
| COMP-CL-9 | Energy Weapon Events (`ev_energy.cpp`) | Client Events | Renders dynamic beams, glow sprites, and audio loops for Gauss, Egon, Hornet | Engine Event System |
| COMP-CL-10 | Explosive Weapon Events (`ev_explosives.cpp`) | Client Events | Manages explosions, projectile trails, and view kick for RPG, Crossbow, Satchel | Engine Event System |
| COMP-BOT-1 | Nav Area Geometry (`nav_area_geometry.cpp`) | Shared Bot | Calculates polygon boundaries, elevations, normals, and point containment | `CNavArea`, `CNavMesh` |
| COMP-BOT-2 | Nav Area Connect (`nav_area_connect.cpp`) | Shared Bot | Manages directional connections, ladder links, and mesh split/merge operations | `CNavArea`, `CNavLadder` |
| COMP-BOT-3 | Nav Area Tactical (`nav_area_tactical.cpp`) | Shared Bot | Computes hiding spots, sniper vantage points, and approach encounter points | `CNavArea`, `HidingSpot` |

### Architecture Diagram

```
+-------------------------------------------------------------------------------+
|                            Client DLL (cl_dll)                                |
+---------------------------------------+---------------------------------------+
|  cl_dll/studio/                       |  cl_dll/hud/                          |
|  - StudioModelRenderer.cpp (Core)     |  - ammo.cpp (Core Ammo Count)         |
|  - studio_render_bones.cpp            |  - hud_weapon_selection.cpp           |
|  - studio_render_lighting.cpp         |  - hud_spectator.cpp (Core)           |
|  - studio_render_attachments.cpp      |  - hud_spectator_director.cpp         |
+---------------------------------------+  - hud_spectator_overview.cpp         |
|  cl_dll/events/                       |  - hud_spectator_menu.cpp             |
|  - ev_hldm.cpp (Core Dispatch)        +---------------------------------------+
|  - ev_bullets.cpp                     |
|  - ev_energy.cpp                      |
|  - ev_explosives.cpp                  |
+---------------------------------------+
                                        |
+---------------------------------------+---------------------------------------+
|                      Shared Bot Navigation (game_shared/bot)                  |
+-------------------------------------------------------------------------------+
|  game_shared/bot/                                                             |
|  - nav_area.cpp (Core Lifecycle & Serialization)                              |
|  - nav_area_geometry.cpp (Mesh Quads, Z-height, Normals)                      |
|  - nav_area_connect.cpp (Connections, Splitting, Merging, Ladders)            |
|  - nav_area_tactical.cpp (Hiding Spots, Approach Points, Sniper Vantage)      |
+-------------------------------------------------------------------------------+
```

## Modular Decomposition Details

### 1. `StudioModelRenderer.cpp` Decomposition
- **`studio_render_bones.cpp`**: Extracts `StudioSetupBones`, `StudioSetUpTransform`, `StudioSaveBones`, `StudioMergeBones`, `StudioSlerpBones`, `StudioCalcBoneAdj`, `StudioCalcBoneQuaterion`, `StudioCalcBonePosition`, `StudioCalcRotations`, `StudioPlayerBlend`, `StudioEstimateGait`, `StudioProcessGait`, `StudioFxTransform`, `StudioEstimateInterpolant`, `StudioEstimateFrame`.
- **`studio_render_lighting.cpp`**: Extracts `StudioDynamicLight`, `StudioLighting`, `StudioChrome`, `StudioSetupLighting`, `StudioLightParams`, and normal transformations.
- **`studio_render_attachments.cpp`**: Extracts `StudioCalcAttachments`, `StudioSetupAttachments`, `StudioSetupBodyparts`, and `StudioSetupSubmodels`.
- **`StudioModelRenderer.cpp`**: Retains top-level render entry points (`StudioDrawModel`, `StudioDrawPlayer`, `StudioRenderModel`, `StudioRenderFinal`, `StudioRenderFinal_Software`, `StudioRenderFinal_Hardware`, `Init`, constructor, destructor).

### 2. `hud_spectator.cpp` Decomposition
- **`hud_spectator_director.cpp`**: Extracts director packet processing (`DirectorMessage`, `HandleDirectorEvent`, `FindBestPlayer`, `CheckCameraModes`).
- **`hud_spectator_overview.cpp`**: Extracts overview rendering (`DrawOverview`, `ParseOverviewFile`, `DrawOverviewEntities`, `DrawOverviewLayer`).
- **`hud_spectator_menu.cpp`**: Extracts menu UI (`DrawSpectatorMenu`, `CheckSettings`, `DrawPlayerList`).
- **`hud_spectator.cpp`**: Retains main `CHudSpectator` constructor, `Draw`, `Think`, `Init`, and `VidInit`.

### 3. `ammo.cpp` Decomposition
- **`hud_weapon_selection.cpp`**: Extracts `SelectItem`, `SelectSlot`, `DrawWList`, `DrawCrosshair`, `SlotInput`, `UserCmd_Slot1`..`UserCmd_Slot10`, `UserCmd_Close`, `UserCmd_NextWeapon`, `UserCmd_PrevWeapon`, `UserCmd_AdjustSlots`.
- **`ammo.cpp`**: Retains core `CHudAmmo` HUD registration, message callbacks (`MsgFunc_CurWeapon`, `MsgFunc_AmmoX`, `MsgFunc_WeaponList`, `MsgFunc_CustWeapon`, `MsgFunc_AmmoPickup`, `MsgFunc_WeapPickup`, `MsgFunc_ItemPickup`, `MsgFunc_HideWeapon`), `WeaponsResource` methods, and main digit drawing.

### 4. `ev_hldm.cpp` Decomposition
- **`ev_bullets.cpp`**: Extracts `EV_FireGlock1`, `EV_FireGlock2`, `EV_FireShotGunSingle`, `EV_FireShotGunDouble`, `EV_FireMP5`, `EV_FireMP52`, `EV_FirePython`.
- **`ev_energy.cpp`**: Extracts `EV_SpinGauss`, `EV_StopPreviousGauss`, `EV_FireGauss`, `EV_EgonFire`, `EV_EgonStop`, `EV_HornetGunFire`.
- **`ev_explosives.cpp`**: Extracts `EV_Crowbar`, `EV_BoltCallback`, `EV_FireCrossbow`, `EV_FireCrossbow2`, `EV_FireRpg`, `EV_TripmineFire`, `EV_SnarkFire`.
- **`ev_hldm.cpp`**: Retains event initialization table (`EV_HLDM_Init`) and common helper functions (`EV_EjectBrass`, `EV_GetGunPosition`, `EV_GetDefaultShellInfo`, `EV_HLDM_FireBullets`, `EV_TrainPitchAdjust`).

### 5. `nav_area.cpp` Decomposition
- **`nav_area_geometry.cpp`**: Extracts `GetZ`, `ComputeNormal`, `IsOverlapping`, `GetClosestPointOnArea`, `Contains`, `GetDistanceSquaredToPoint`, `IsCoplanar`, `ComputePortal`, `GetLightIntensity`.
- **`nav_area_connect.cpp`**: Extracts `ConnectTo`, `Disconnect`, `SplitEdit`, `MergeEdit`, `SpliceEdit`, `RaiseLowerEdit`, `ConnectLadder`, `DisconnectLadder`.
- **`nav_area_tactical.cpp`**: Extracts `ComputeHidingSpots`, `ComputeSniperSpots`, `ComputeApproachPoints`, `ComputeEncounterAreas`, `SpotEncounterArea`, `EncounterSpot`, `ComputeEarliestOccupyTimes`.
- **`nav_area.cpp`**: Retains `CNavArea` constructor, destructor, `Initialize`, `Save`, `Load`, `Reset`, and static list definitions.

## System Boundaries & Constraints

- **Preserve Client Rendering Exactness**: Studio model bone blending, lighting calculations, and attachments must remain 100% faithful to the original GoldSrc renderer.
- **HUD Event Timing**: Weapon muzzle flashes, shell casings, and HUD menu selections must maintain identical responsiveness.
- **Bot Nav Mesh Compatibility**: All mesh area connections, hiding spots, and area IDs must maintain exact parity with nav mesh files (`.nav`).

## Testing & Build Verification Strategy

- **Solution & Makefile Validation**: Update all solution files (`projects/vs2019/hl_cdll.vcxproj`, `projects/vs2019/hldll.vcxproj`, `linux/Makefile.hl_cdll`, `linux/Makefile.hldll`, `dlls/Makefile`).
- **CI Build Pipeline**: Validate zero warnings and clean linking on Windows MSVC and Linux GCC workflows.
