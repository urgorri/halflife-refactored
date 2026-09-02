# Implementation Plan

- [x] 1. Monster Auxiliary Component Reintegration
  - [x] 1.1 Reintegrate `baby_headcrab` into `dlls/monsters/headcrab.cpp` and delete `baby_headcrab.cpp`/`.h`
  - [x] 1.2 Reintegrate `bullsquid_spit` into `dlls/monsters/bullsquid.cpp` and delete `bullsquid_spit.cpp`/`.h`
  - [x] 1.3 Reintegrate `bigmomma_mortar` into `dlls/monsters/bigmomma.cpp` and delete `bigmomma_mortar.cpp`/`.h`
  - [x] 1.4 Reintegrate `controller_energy` into `dlls/monsters/controller.cpp` and delete `controller_energy.cpp`/`.h`
  - [x] 1.5 Reintegrate `gargantua_effects` into `dlls/monsters/gargantua.cpp` and delete `gargantua_effects.cpp`/`.h`
  - [x] 1.6 Reintegrate `nihilanth_energy` into `dlls/monsters/nihilanth.cpp` and delete `nihilanth_energy.cpp`/`.h`
  - [x] 1.7 Reintegrate `tentacle_maw` into `dlls/monsters/tentacle.cpp` and delete `tentacle_maw.cpp`/`.h`
  - [x] 1.8 Reintegrate `apache_hvr` into `dlls/monsters/apache.cpp` and delete `apache_hvr.cpp`/`.h`
  - _Requirements: [REQ-1]_

- [x] 2. Domain Entity Subsystems Consolidation
  - [x] 2.1 Consolidate `charger_health` and `charger_suit` into `dlls/systems/chargers.cpp` and `chargers.h`, deleting split files
  - [x] 2.2 Reintegrate `door_rotating` and `door_momentary` into `dlls/systems/doors.cpp`, deleting split files
  - [x] 2.3 Reintegrate `xen_tree` and `xen_spores` into `dlls/world/xen.cpp`, deleting split files
  - [x] 2.4 Consolidate `func_tankgun`, `func_tanklaser`, `func_tankmortar`, `func_tankrocket` into `dlls/systems/func_tank.cpp`, deleting split files
  - _Requirements: [REQ-2]_
  - _Dependencies: Phase 1_

- [x] 3. Map Rules & Triggers Architecture
  - [x] 3.1 Consolidate all 11 `game_*` map rule micro-entities into `dlls/gameplay/maprules.cpp`, deleting split files
  - [x] 3.2 Create `dlls/systems/triggers_brush.cpp` and consolidate all volumetric BSP brush triggers
  - [x] 3.3 Create `dlls/systems/triggers_point.cpp` and consolidate all logical non-brush point triggers
  - [x] 3.4 Delete all 24 obsolete individual trigger `.cpp` files
  - _Requirements: [REQ-3]_
  - _Dependencies: Phase 2_

- [x] 4. World Item & Ground Weapon Deduplication
  - [x] 4.1 Create declarative `IMPLEMENT_WORLD_ITEM` macro/template in `dlls/items/item_base.h` and consolidate standard pickups into `dlls/items/items.cpp`, deleting redundant micro-files
  - [x] 4.2 Create `INITIALIZE_WORLD_WEAPON` macro/helper in `dlls/weapons/weapon_base.h` and update weapon ground initialization
  - _Requirements: [REQ-4]_
  - _Dependencies: Phase 3_

- [x] 5. Dead Code Removal & Documentation
  - [x] 5.1 Remove unreferenced legacy stubs: `dlls/core/mpstubb.cpp`, `cl_dll/hud/scoreboard.cpp`, `cl_dll/vgui/MOTD.cpp`, `cl_dll/vgui/vgui_ConsolePanel.cpp` / `.h`, `cl_dll/systems/soundsystem.cpp`
  - [x] 5.2 Update `README.md` and `AGENTS.md` documenting legacy code exclusion policy
  - _Requirements: [REQ-5]_
  - _Dependencies: Phase 4_

- [x] 6. Build Systems Synchronization & Verification
  - [x] 6.1 Update and synchronize `projects/vs2019/hldll.vcxproj` and `hldll.vcxproj.filters`
  - [x] 6.2 Update and synchronize `projects/vs2019/hl_cdll.vcxproj` and `hl_cdll.vcxproj.filters`
  - [x] 6.3 Update and synchronize `linux/Makefile.hldll` and `linux/Makefile.hl_cdll`
  - [x] 6.4 Build locally with MSBuild on Win32 Release and verify 0 errors, 0 warnings
  - _Requirements: [REQ-6]_
  - _Dependencies: Phase 5_
