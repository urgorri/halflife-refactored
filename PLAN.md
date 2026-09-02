# Implementation Plan

- [ ] 1. Weapons, Projectiles & World Items Subsystem Consolidation
  - [ ] 1.1 Create `dlls/weapons/weapons.h` declaring all 15 player weapon classes
  - [ ] 1.2 Create `dlls/weapons/projectiles.h` declaring all 7 projectile classes
  - [ ] 1.3 Create `dlls/items/items.h` declaring `CItem`, `CWorldItem`, and all world item entities
  - [ ] 1.4 Update `cl_dll/hl/hl_weapons.cpp` and all weapon `.cpp` files to include `weapons.h` and `projectiles.h`
  - [ ] 1.5 Delete the 22 obsolete micro-headers in `dlls/weapons/` and `dlls/items/`
  - _Requirements: [REQ-1]_

- [ ] 2. Studio Model & Render Pipeline Consolidation
  - [ ] 2.1 Reintegrate `studio_render_attachments.cpp` and `GameStudioModelRenderer.cpp/.h` into `StudioModelRenderer.cpp` and `StudioModelRenderer.h`
  - [ ] 2.2 Reintegrate `tri.cpp/.h` and `view_bob.cpp` into `view.cpp` and `view_camera.cpp`
  - [ ] 2.3 Delete 5 obsolete micro-files (`studio_render_attachments.cpp`, `GameStudioModelRenderer.*`, `tri.*`)
  - _Requirements: [REQ-2]_
  - _Dependencies: Phase 1_

- [ ] 3. HUD Indicators & Utility Dispatchers Consolidation
  - [ ] 3.1 Create `cl_dll/hud/hud_indicators.cpp` consolidating `battery.cpp`, `flashlight.cpp`, `geiger.cpp`, `train.cpp`, `status_icons.cpp`
  - [ ] 3.2 Reintegrate `hud_update.cpp` and `hud_msg.cpp` into `hud.cpp` and `hud_redraw.cpp`
  - [ ] 3.3 Delete 7 obsolete micro-files in `cl_dll/hud/`
  - _Requirements: [REQ-3]_
  - _Dependencies: Phase 2_

- [ ] 4. Environmental Effects & Screen Systems Consolidation
  - [ ] 4.1 Create `dlls/systems/effects_environment.cpp/.h` consolidating `effects_beverage`, `effects_shooters`, `effects_env`, `env_global`, `env_spark`, `explode`, and `airtank.cpp`
  - [ ] 4.2 Create `dlls/systems/effects_screen.cpp/.h` consolidating `effects_screen`, `info_intermission.cpp`, and `revertsaved.cpp`
  - [ ] 4.3 Delete 14 obsolete micro-files in `dlls/systems/`
  - _Requirements: [REQ-4]_
  - _Dependencies: Phase 3_

- [ ] 5. World Brush Entities & Transportation Systems Consolidation
  - [ ] 5.1 Reintegrate `conveyor.cpp/.h` and `guntarget.cpp/.h` into `dlls/world/bmodels.cpp/.h`
  - [ ] 5.2 Reintegrate `trackchange.cpp/.h` into `dlls/world/trains.cpp/.h`
  - [ ] 5.3 Delete 6 obsolete micro-files in `dlls/world/`
  - _Requirements: [REQ-5]_
  - _Dependencies: Phase 4_

- [ ] 6. VGUI Controls Suite & Viewport Menus Consolidation
  - [ ] 6.1 Create `cl_dll/vgui/vgui_controls.h` with unified `<VGUI_*.h>` includes and update `vgui_int.h`
  - [ ] 6.2 Reintegrate `vgui_viewport_menus.cpp` and `vgui_viewport_messages.cpp` into `vgui_TeamFortressViewport.cpp`
  - [ ] 6.3 Consolidate secondary dialogs (`vgui_ControlConfigPanel.*`, `vgui_MOTDWindow.cpp`)
  - [ ] 6.4 Delete 5 obsolete micro-files in `cl_dll/vgui/`
  - _Requirements: [REQ-6]_
  - _Dependencies: Phase 5_

- [ ] 7. Ambient & Cinematic NPCs Consolidation
  - [ ] 7.1 Reintegrate `hgrunt_repel.cpp` into `dlls/monsters/hgrunt.cpp/.h`
  - [ ] 7.2 Create `dlls/monsters/monsters_ambient.cpp/.h` consolidating `monster_deadhev.*`, `genericmonster.*`, and `rat.*`
  - [ ] 7.3 Delete 6 obsolete micro-files in `dlls/monsters/`
  - _Requirements: [REQ-7]_
  - _Dependencies: Phase 6_

- [ ] 8. Server Sound Engine Consolidation
  - [ ] 8.1 Consolidate `sound_dsp.cpp`, `sound_speaker.cpp`, `sound_sentences.cpp` into `sound_ambient.cpp` and `sound.cpp`
  - [ ] 8.2 Delete 4 obsolete micro-files in `dlls/systems/`
  - _Requirements: [REQ-8]_
  - _Dependencies: Phase 7_

- [ ] 9. Client Event API Suite & Hooking Consolidation
  - [ ] 9.1 Package engine/event headers into `cl_dll/events/eventscripts.h`
  - [ ] 9.2 Reintegrate `events.cpp` and `hl_events.cpp` into `cl_dll/events/ev_common.cpp`
  - [ ] 9.3 Reintegrate `hl_objects.cpp` into `cl_dll/hl/hl_baseentity.cpp`
  - [ ] 9.4 Delete 3 obsolete micro-files
  - _Requirements: [REQ-9]_
  - _Dependencies: Phase 8_

- [ ] 10. Build Systems Synchronization & Verification
  - [ ] 10.1 Synchronize `projects/vs2019/hldll.vcxproj` and `hldll.vcxproj.filters`
  - [ ] 10.2 Synchronize `projects/vs2019/hl_cdll.vcxproj` and `hl_cdll.vcxproj.filters`
  - [ ] 10.3 Synchronize `linux/Makefile.hldll` and `linux/Makefile.hl_cdll`
  - [ ] 10.4 Build and verify locally with MSBuild Win32 Release (`0 errors`)
  - [ ] 10.5 Commit, push to topic branch, open PR, and verify GitHub Actions CI
  - _Requirements: [REQ-10]_
  - _Dependencies: Phase 9_
