# Implementation Plan

- [x] 1. Turret Subsystem Consolidation
  - [x] 1.1 Create `dlls/systems/turrets.h` with complete class declarations for `CBaseTurret`, `CTurret`, `CMiniTurret`, and `CSentry`
  - [x] 1.2 Create `dlls/systems/turrets.cpp` consolidating base turret logic and derived variants
  - [x] 1.3 Update references to `turret.h` across the codebase to `systems/turrets.h`
  - [x] 1.4 Delete redundant micro-files: `turret_base.cpp`, `turret.cpp`, `miniturret.cpp`, `sentry.cpp`, `turret.h`
  - _Requirements: [REQ-1]_

- [x] 2. Visual Effects & Beam Subsystem Consolidation
  - [x] 2.1 Update `dlls/systems/effects.h` with clean declarations for `CBeam`, `CLaser`, `CLightning`, `CGlow`, and `CSprite`
  - [x] 2.2 Create `dlls/systems/effects_beams.cpp` consolidating beam, laser, lightning, glow, and sprite implementations
  - [x] 2.3 Delete redundant micro-files: `effects_beam.cpp`, `effects_laser.cpp`, `effects_lightning.cpp`, `effects_glow.cpp`, `effects_sprite.cpp`
  - _Requirements: [REQ-2]_
  - _Dependencies: Phase 1_

- [x] 3. Client Spectator HUD Subsystem Consolidation
  - [x] 3.1 Update `cl_dll/hud/hud_spectator.h` consolidating all spectator sub-structures and helper declarations
  - [x] 3.2 Consolidate camera director, overview radar, and spectator menus into `cl_dll/hud/hud_spectator.cpp`
  - [x] 3.3 Delete redundant micro-files: `hud_spectator_director.cpp`, `hud_spectator_overview.cpp`, `hud_spectator_menu.cpp`
  - _Requirements: [REQ-3]_
  - _Dependencies: Phase 2_

- [x] 4. Client Ammo HUD Subsystem Consolidation
  - [x] 4.1 Create `cl_dll/hud/hud_ammo.h` declaring `CHudAmmo`, `CHudAmmoSecondary`, and `CHudAmmoHistory`
  - [x] 4.2 Consolidate ammo counter, secondary ammo bar, and pickup history into `cl_dll/hud/hud_ammo.cpp`
  - [x] 4.3 Update client references (`hud.h`, `hud_redraw.cpp`, etc.) to use `hud_ammo.h`
  - [x] 4.4 Delete redundant micro-files: `ammo.cpp`, `ammo_secondary.cpp`, `ammohistory.cpp`, `ammo.h`, `ammohistory.h`
  - _Requirements: [REQ-4]_
  - _Dependencies: Phase 3_

- [x] 5. Build Systems Synchronization & Verification
  - [x] 5.1 Update and synchronize `projects/vs2019/hldll.vcxproj` and `hldll.vcxproj.filters`
  - [x] 5.2 Update and synchronize `projects/vs2019/hl_cdll.vcxproj` and `hl_cdll.vcxproj.filters`
  - [x] 5.3 Update and synchronize `linux/Makefile.hldll` and `linux/Makefile.hl_cdll`
  - [x] 5.4 Audit and clean up unused `#include` statements across touched files
  - [x] 5.5 Build locally with MSBuild on Win32 Release (`hldll` and `hl_cdll`) and verify 0 errors
  - _Requirements: [REQ-5]_
  - _Dependencies: Phase 4_
