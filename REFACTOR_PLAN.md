# GoldSrc DLL Refactoring Plan

## 1. Current Architecture
The current `dlls/` directory is essentially a flat folder containing almost all the logic for the game. Files are often organized loosely by arbitrary categories that end up violating the Single Responsibility Principle, and hiding important concrete classes.

### Identified Large/Mixed-Responsibility Files:
* `items.cpp`: Contains `CItem` base class, plus `CWorldItem`, `CItemSuit`, `CItemBattery`, `CItemAntidote`, `CItemSecurity`, `CItemLongJump`.
* `weapons.cpp`: Contains the base weapon logic (`CBasePlayerWeapon`, `CBasePlayerAmmo`, `CWeaponBox`), but also concrete implementations like `CRpg`, `CShotgun`, `CGauss`, `CEgon`, `CSatchel`.
* `monsters.cpp`: Contains base `CBaseMonster` logic but mixes general monster behavior and memory.
* `player.cpp`: Massive file containing `CBasePlayer` but also other classes (`CSprayCan`, `CDeadHEV`, `CStripWeapons`, `CRevertSaved`, `CInfoIntermission`).
* `func_tank.cpp`: Contains base `CFuncTank` and concrete ones (`CFuncTankGun`, `CFuncTankLaser`, `CFuncTankRocket`, `CFuncTankMortar`).
* `maprules.cpp`: Contains `CGameTeamSet`, `CGamePlayerZone`, `CGamePlayerHurt`, `CGameCounter`, `CGameCounterSet`, etc.
* `triggers.cpp`: Contains many concrete triggers and some base functionality (e.g. `CBaseTrigger`, `CFrictionModifier`, `CAutoTrigger`, `CTriggerMultiple`, etc.)
* `turret.cpp`: `CBaseTurret`, `CTurret`, `CMiniTurret`, `CSentry`.
* Many others like `bmodels.cpp`, `doors.cpp`, `plats.cpp`.

### Important Class Hierarchies and Dependencies:
* `CBaseEntity` -> `CBaseDelay` -> `CBaseToggle` -> `CBaseMonster`
* `CBaseEntity` -> `CBaseDelay` -> `CBaseToggle` -> `CBasePlayerItem` -> `CBasePlayerWeapon`
* `CBasePlayerItem` -> `CBasePlayerAmmo`
* `CBaseEntity` -> `CBaseDelay` -> `CBaseToggle` -> `CBaseTrigger`

## 2. Target Architecture
The proposed directory tree will group the codebase by domain. Base classes and shared utilities will be separated from concrete implementations.

### Proposed Directory Tree:
```text
dlls/
  ai/           - AI logic, schedules, pathing, base monsters, nodes.
  core/         - Core engine interfaces, base entity classes, util, globals, game rules, saving/restoring, networking.
  gameplay/     - High-level game concepts, map rules, multi-manager, scripted sequences, level transitions.
  items/        - Pickups, items, suit, battery, etc. (Not weapons).
  monsters/     - Concrete monster implementations (zombies, hounds, aliens, scientists, grunts).
  systems/      - Generic systems: triggers, effects, sounds, lights, buttons, decals, doors, breakables.
  weapons/      - Base weapon classes, projectiles, ammo, and specific weapons.
  world/        - World interactions, func_walls, brushes, water, bmodels, plats.
```

### Responsibility of Major Directories
- **`ai/`**: Deals with how monsters think, move, and plan. It houses base monster definitions without detailing the behavior of specific monster types.
- **`core/`**: The backbone of the dll, providing base classes (`cbase`), math structures, client-server bindings, and generic memory/utility stuff.
- **`gameplay/`**: Rules of the game, scoring, team play, map rules, and scripted elements (e.g. `scripted.cpp`).
- **`items/`**: Pickups that the player interacts with by walking over them (excluding weapons and ammo).
- **`monsters/`**: Each concrete NPC gets its own `.cpp`/`.h` pair here.
- **`systems/`**: Entities that make the level alive but are not AI (triggers, buttons, doors, moving platforms, effects).
- **`weapons/`**: The player's arsenal. Includes base logic, ammunition, dropped weapons, and specific weapon logic.
- **`world/`**: The physical environment entities.

### Naming Conventions
* **Files**: Lower-snake-case or standard `class_name` without `c` prefix (e.g., `item_battery.cpp`, `weapon_rpg.cpp`, `trigger_multiple.cpp`).
* **Classes**: Preserve existing class names.
* **Includes**: Update include paths using relative quotes (e.g., `#include "../core/cbase.h"`).

## 3. Migration Strategy
To avoid breaking entity registration, includes, inheritance, and build references, the migration will follow a strict bottom-up approach.

### Migration Order
1. **Directory Setup**: Create the new folder structure.
2. **Core & Base Classes**: Isolate the core classes. `cbase.h` and `util.h` are foundational, they move to `core/`.
3. **AI**: Move base AI systems into `ai/`.
4. **Items**: Extract specific items from `items.cpp` into `items/`.
5. **Weapons**: Extract base weapon logic, then extract specific weapons into individual files inside `weapons/`.
6. **Monsters**: Move concrete monsters into the `monsters/` directory.
7. **Systems & World**: Refactor triggers, buttons, doors, and bmodels into their respective directories.
8. **Gameplay**: Move remaining logic like gamerules and maprules to `gameplay/`.
9. **Final Cleanup**: Fix remaining includes, remove obsolete files, update Makefiles/CMakeLists.txt if necessary.

### Handling Includes and Build Files
For this migration, the strategy focuses purely on filesystem reorganization and splitting files. Whenever a file is moved, its includes must be updated, and any file depending on it must be updated.

## 4. Migration Tasks

### Task 1 — Create Directory Structure
**Prompt:**
> Create the following directories inside `dlls/`: `ai`, `core`, `gameplay`, `items`, `monsters`, `systems`, `weapons`, `world`. Do not move any files yet.

**Files/areas:** `dlls/`
**Dependencies:** None
**Expected result:** The new directory structure exists.

### Task 2 — Migrate Core Files
**Prompt:**
> Move `cbase.cpp`, `cbase.h`, `util.cpp`, `util.h`, `globals.cpp`, `cdll_dll.h`, `enginecallback.h`, `extdll.h`, `saverestore.h`, `vector.h` to `dlls/core/`. Update all files in `dlls/` that include these files to use `#include "core/<file>.h"` instead. Note: If the build system is configured to look in subdirectories, you may not need to change the include paths in every file, but assume for this refactoring that includes should reflect the new structure.

**Files/areas:** `dlls/*`, `dlls/core/`
**Dependencies:** Task 1
**Expected result:** Core files are in `dlls/core/`. All other files can compile with the new paths.

### Task 3 — Extract Concrete Items
**Prompt:**
> Split `dlls/items.cpp` into multiple files inside `dlls/items/`.
> - Keep `CItem` and `CWorldItem` in `dlls/items/item_base.cpp` (and create `item_base.h`).
> - Create `item_battery.cpp`, `item_suit.cpp`, `item_antidote.cpp`, `item_security.cpp`, `item_longjump.cpp` in `dlls/items/`.
> Move the respective class definitions and implementation methods into these new files. Make sure they include `item_base.h`.

**Files/areas:** `dlls/items.cpp`, `dlls/items.h`, `dlls/items/`
**Dependencies:** Task 1, 2
**Expected result:** `items.cpp` is removed or emptied, replaced by modular item files in `dlls/items/`.

### Task 4 — Extract Specific Weapons
**Prompt:**
> Split `dlls/weapons.cpp` and `dlls/weapons.h`.
> - Keep base weapon classes (`CBasePlayerWeapon`, `CBasePlayerAmmo`, `CWeaponBox`, etc.) in `dlls/weapons/weapon_base.cpp` / `weapon_base.h`.
> - Extract `CRpg` to `dlls/weapons/weapon_rpg.cpp`.
> - Extract `CShotgun` to `dlls/weapons/weapon_shotgun.cpp`.
> - Extract `CGauss` to `dlls/weapons/weapon_gauss.cpp`.
> - Extract `CEgon` to `dlls/weapons/weapon_egon.cpp`.
> - Extract `CSatchel` to `dlls/weapons/weapon_satchel.cpp`.
> Update `weapons.cpp` implementations accordingly.

**Files/areas:** `dlls/weapons.cpp`, `dlls/weapons.h`, `dlls/weapons/`
**Dependencies:** Task 1, 2
**Expected result:** Concrete weapon logic is properly modularized.

### Task 5 — Migrate Player and Miscellaneous Classes
**Prompt:**
> Refactor `dlls/player.cpp`. Keep `CBasePlayer` in `dlls/core/player.cpp`.
> Move `CSprayCan` to `dlls/weapons/weapon_spraycan.cpp`.
> Move `CDeadHEV` to `dlls/monsters/monster_deadhev.cpp`.
> Move `CStripWeapons` and `CRevertSaved` to `dlls/gameplay/` or `dlls/systems/`.
> Ensure all headers and includes are appropriately resolved.

**Files/areas:** `dlls/player.cpp`, `dlls/player.h`, new target directories.
**Dependencies:** Task 1, 2, 3, 4
**Expected result:** `player.cpp` contains only `CBasePlayer`.

### Task 6 — Split Triggers
**Prompt:**
> Split `dlls/triggers.cpp`.
> Keep `CBaseTrigger` and base logic in `dlls/systems/trigger_base.cpp`.
> Extract `CFrictionModifier`, `CAutoTrigger`, `CTriggerMultiple`, `CTriggerOnce`, `CTriggerHurt`, `CTriggerPush`, etc., into their own files in `dlls/systems/` (e.g., `trigger_multiple.cpp`, `trigger_hurt.cpp`).

**Files/areas:** `dlls/triggers.cpp`, `dlls/systems/`
**Dependencies:** Task 1, 2
**Expected result:** Modular triggers architecture in `dlls/systems/`.

### Task 7 — Split Func Tanks
**Prompt:**
> Split `dlls/func_tank.cpp`.
> Move `CFuncTank` to `dlls/systems/func_tank.cpp`.
> Move `CFuncTankGun` to `dlls/systems/func_tankgun.cpp`.
> Move `CFuncTankLaser` to `dlls/systems/func_tanklaser.cpp`.
> Move `CFuncTankRocket` to `dlls/systems/func_tankrocket.cpp`.
> Move `CFuncTankMortar` to `dlls/systems/func_tankmortar.cpp`.

**Files/areas:** `dlls/func_tank.cpp`, `dlls/systems/`
**Dependencies:** Task 1, 2
**Expected result:** `func_tank.cpp` is replaced by modular files.

### Task 8 — Move Monsters
**Prompt:**
> Move all concrete monster implementations into `dlls/monsters/`.
> Examples: `bullsquid.cpp`, `barney.cpp`, `bigmomma.cpp`, `houndeye.cpp`, `roach.cpp`, `apache.cpp`, `osprey.cpp`.
> Make sure `#include` paths inside these files are updated to point to the base classes like `../ai/monsters.h` or `../core/cbase.h`.

**Files/areas:** Various `dlls/*.cpp` for monsters, `dlls/monsters/`
**Dependencies:** Task 1, 2
**Expected result:** `dlls/` root is cleaner, monsters live in `dlls/monsters/`.

### Task 9 — Move Base AI
**Prompt:**
> Move AI-related files like `monsters.cpp`, `monsters.h`, `nodes.cpp`, `nodes.h`, `schedule.cpp`, `schedule.h`, `defaultai.cpp`, `defaultai.h`, `squadmonster.cpp`, `squadmonster.h` into `dlls/ai/`. Update includes globally.

**Files/areas:** AI files, `dlls/ai/`
**Dependencies:** Task 1, 2
**Expected result:** Base AI behavior is isolated.

### Task 10 — Move Systems and Gameplay
**Prompt:**
> Move map rules and scoring files (e.g., `maprules.cpp`, `maprules.h`, `multiplay_gamerules.cpp`, `gamerules.cpp`, `scripted.cpp`) to `dlls/gameplay/`.
> Split `maprules.cpp` if possible, extracting entities like `CGameTeamSet`, `CGamePlayerZone`, etc., into `dlls/gameplay/game_teamset.cpp`, etc.

**Files/areas:** `dlls/maprules.cpp`, `dlls/gamerules.cpp`, `dlls/gameplay/`
**Dependencies:** Task 1, 2
**Expected result:** Gameplay rules are separated.

### Task 11 — Split Turrets
**Prompt:**
> Split `dlls/turret.cpp`.
> Keep `CBaseTurret` in `dlls/systems/turret_base.cpp`.
> Extract `CTurret`, `CMiniTurret`, `CSentry` to their own files in `dlls/systems/`.

**Files/areas:** `dlls/turret.cpp`, `dlls/systems/`
**Dependencies:** Task 1, 2
**Expected result:** Modular turrets.

## 5. Final Cleanup

### Task 12 — Cleanup and Review
**Prompt:**
> Perform a final pass over `dlls/` root. Move remaining files like `bmodels.cpp`, `doors.cpp`, `plats.cpp` to `dlls/world/` or `dlls/systems/`.
> Remove any empty, original monolithic files (e.g., `items.cpp`, `weapons.cpp`).
> Verify that no large mixed-responsibility files remain.
> Update build files (Makefile/CMake) to include all the newly created files and directories.

**Files/areas:** `dlls/*`, Build system files
**Dependencies:** Tasks 1-11
**Expected result:** A perfectly clean `dlls/` root with only subdirectories and possibly a central entry point (`cdll_dll.cpp`/`h_export.cpp`).

## 6. Architecture Completion Checklist

Use this checklist to verify that the refactoring is complete:

- [ ] `dlls/` root contains no source files except necessary entry points (e.g., `h_export.cpp`).
- [ ] `items.cpp` is removed. All items (`CItemBattery`, `CItemSuit`, etc.) have their own `.cpp`/`.h` files in `dlls/items/`.
- [ ] `weapons.cpp` is removed. All weapons (`CRpg`, `CEgon`, etc.) have their own `.cpp`/`.h` files in `dlls/weapons/`.
- [ ] `player.cpp` only contains `CBasePlayer` and deeply coupled core player functionality. Other classes are moved out.
- [ ] `triggers.cpp` is removed, replaced by individual trigger files in `dlls/systems/`.
- [ ] `func_tank.cpp` is removed, replaced by individual tank files in `dlls/systems/`.
- [ ] `maprules.cpp` is removed, replaced by individual map rule files in `dlls/gameplay/`.
- [ ] `turret.cpp` is removed, replaced by individual turret files in `dlls/systems/`.
- [ ] Base classes (e.g. `CBaseMonster`, `CBasePlayerWeapon`) are separate from concrete implementations.
- [ ] There is a clear mapping of domain logic to its corresponding directory.
- [ ] All includes are correctly pointing to the newly located files.
- [ ] The build system (Makefiles, Visual Studio projects, etc.) correctly references all new files.
- [ ] The code compiles successfully without warnings related to missing files.
- [ ] The game behavior remains 100% identical to the pre-refactor state.
