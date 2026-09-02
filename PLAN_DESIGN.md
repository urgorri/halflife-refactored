# Design Document

## Overview

This design outlines the refactoring and consolidation strategy for Half-Life GoldSrc DLL codebase. By eliminating redundant translation units, extracting reusable pickup macros for world items and ground weapons, partitioning triggers cleanly into brush vs point categories, and removing unreferenced dead legacy files, the codebase achieves optimal locality, maintainability, and significantly faster compilation speeds without altering any game behavior.

## System Architecture

### Component Map

| Component ID | Name | Type | Responsibility | Interfaces With |
|-------------|------|------|----------------|-----------------|
| COMP-AI | Monster AI & Satellites | Server | NPC behaviors, animations, attacks, projectiles | Core Game Engine |
| COMP-SYS | Systems & Environmental Entities | Server | Chargers, Doors, Bmodels, Tanks, Effects | Game Rules, Core |
| COMP-WORLD | World & Xen Fauna | Server | World geometry interactions, Xen plants | Physics, Core |
| COMP-RULES | Map Rules Subsystem | Server | BSP level logic, scoring, team routing | Game Rules, Client |
| COMP-TRIG | Triggers (Brush & Point) | Server | Touch volumes, changelevels, relays, counters | Dispatcher, Core |
| COMP-ITEMS | Items & World Pickups | Server | Healthkits, batteries, suit, ground weapons | Player Inventory, HUD |
| COMP-CL | Client HUD & VGUI | Client | HUD rendering, VGUI menus, View models | Engine Client API |

---

## Data Flow Specifications

### 1. World Item Pickup Flow (`IMPLEMENT_WORLD_ITEM`)

```
1. Map/Monster Spawns Item → Precache model/sound → Drop to Floor (FallInit)
2. Player touches item bounding box → Item::MyTouch(pPlayer)
3. Resource granted to player (Armor, Health, HEV Suit, Ammo)
4. EMIT_SOUND (pickup audio) + Send Net Message (gmsgItemPickup) → Destroy or Respawn Item
```

### 2. Triggers Partitioning Flow

```
[Brush Triggers - triggers_brush.cpp]
Player/Monster enters BSP Volume → Touch() → Evaluate Filter/Flags → Fire Target(Activator)

[Point Triggers - triggers_point.cpp]
Trigger/Logic receives Use() → Evaluate Counter/Condition → Dispatch UseTargets()
```

---

## Detailed Components Refactoring

### 1. Monster Subsystem Locality
- Reintegrate satellite projectiles and effect entities back into primary monster source files:
  - `CBabyCrab` in `headcrab.cpp`
  - `CSquidSpit` in `bullsquid.cpp`
  - `CBigMommaMortar` in `bigmomma.cpp`
  - `CControllerHeadBall` / `CControllerZapBall` in `controller.cpp`
  - `CGargantuaFlame` in `gargantua.cpp`
  - `CNihilanthEnergyOrb` in `nihilanth.cpp`
  - `CTentacleMaw` in `tentacle.cpp`
  - `CApacheHVR` in `apache.cpp`
- Remove all 8 satellite `.cpp` and `.h` files.

### 2. Chargers, Doors, Xen Flora, and Tanks Consolidation
- **Chargers**: Consolidate `CBaseWallCharger`, `CWallHealth`, `CWallRecharge` in `dlls/systems/chargers.cpp` (and `chargers.h`).
- **Doors**: Reintegrate `CRotDoor` and `CMomentaryDoor` into `dlls/systems/doors.cpp`.
- **Xen Flora**: Reintegrate `CXenTree`, `CXenTreeTrigger`, and `CXenSpore` into `dlls/world/xen.cpp`.
- **Tanks**: Consolidate `CFuncTank`, `CFuncTankGun`, `CFuncTankLaser`, `CFuncTankMortar`, `CFuncTankRocket` into `dlls/systems/func_tank.cpp`.

### 3. Triggers & Map Rules Architecture
- Consolidate all 11 `game_*` map rule entities into `dlls/gameplay/maprules.cpp`.
- Partition triggers into:
  - `dlls/systems/triggers_brush.cpp` (Brush entities with BSP models & volume touch)
  - `dlls/systems/triggers_point.cpp` (Point entities with logic & relay dispatch)
- Remove 24 fragmented micro-files.

### 4. Items & Ground Weapon Deduplication Engine
- Macro engine in `dlls/items/item_base.h`:
  ```cpp
  #define IMPLEMENT_WORLD_ITEM(className, entityName, modelName, soundName) \
      ...
  ```
- Ground weapon initializer in `dlls/weapons/weapon_base.h`:
  ```cpp
  #define INITIALIZE_WORLD_WEAPON(entityName, weaponId, worldModel, defaultAmmo) \
      ...
  ```

### 5. Dead Code Cleanup & Documentation
- Delete 5 verified uncompiled legacy files (`dlls/core/mpstubb.cpp`, `cl_dll/hud/scoreboard.cpp`, `cl_dll/vgui/MOTD.cpp`, `cl_dll/vgui/vgui_ConsolePanel.cpp` + header, `cl_dll/systems/soundsystem.cpp`).
- Document explicit legacy code exclusion in `README.md` and `AGENTS.md`.

---

## Testing Strategy

1. **Static Build Verification**:
   - Visual Studio 2019 Win32 Release build: 0 errors, 0 warnings.
   - GCC/Clang Linux build verification.
2. **Behavior Preservation Verification**:
   - Ensure all class names registered via `LINK_ENTITY_TO_CLASS` are 100% identical.
   - Verify save/restore `TYPEDESCRIPTION` tables match original layouts.
   - Verify map entity dispatch, touch physics, and damage calculation logic match verbatim.
