# Behavioral Equivalence Verification

This test suite implements automated behavioral equivalence verification for `halflife-refactored`, ensuring that internal refactoring maintains exact gameplay behavior, binary ABI compatibility, and savegame/level-transition serialization with the original Valve Half-Life SDK 2.3.

## Architecture: Fork-Friendly Decoupling

To ensure this repository serves both as a strictly verified refactoring and as a clean foundation for downstream mods or forks:

1. **Compilation Decoupling**:
   - Compile-time regression assertions ([`tests/test_abi_layout.cpp`](file:///c:/code/halflife-refactored/tests/test_abi_layout.cpp) and [`tests/test_saverestore.cpp`](file:///c:/code/halflife-refactored/tests/test_saverestore.cpp)) are compiled exclusively as part of the standalone test runner (`hl_tests`), NOT baked into the production game DLL (`hl.dll` / `hl.so`).
   - Modders can freely add instance variables to `CBasePlayer` or modify shared classes without triggering compile errors when building the game DLL.
2. **CI Pipeline Decoupling**:
   - [`.github/workflows/build.yml`](file:///c:/code/halflife-refactored/.github/workflows/build.yml) is the primary build pipeline: it compiles `client.dll` / `client.so` and `hl.dll` / `hl.so`, verifies binary production, and packages release artifacts. It never fails due to intentional gameplay changes in forks.
   - [`.github/workflows/behavioral-equivalence.yml`](file:///c:/code/halflife-refactored/.github/workflows/behavioral-equivalence.yml) hosts all regression verification layers. It includes a guard condition (`if: github.repository == 'urgorri/halflife-refactored' || vars.ENABLE_BEHAVIORAL_EQUIVALENCE == 'true'`) so downstream forks automatically skip regression checks unless explicitly opted in.

## Overview of Verification Layers

### Layer 1 — Gameplay Constants Audit (`#102`)
- **Script**: [`tests/verify_constants.py`](file:///c:/code/halflife-refactored/tests/verify_constants.py)
- **Golden Baseline**: [`tests/golden/constants.json`](file:///c:/code/halflife-refactored/tests/golden/constants.json)
- **Scope**:
  - Weapon definitions (IDs, auto-switch weights, clip sizes, ammo capacities, default ammo given, ammo pickup quantities, item flags, bullet enum ordinals)
  - Dispersion cone vectors (`VECTOR_CONE_*DEGREES`)
  - HUD definitions & limits (`cdll_dll.h`)
  - Skill levels and `skilldata_t` struct field ordering (`skill.h`)
  - Damage types and duration constants (`damage_defs.h`)
  - Observer modes (`pm_shared.h`)
  - DLL exports (`hl.def`)
  - Entity factory string bindings (`LINK_ENTITY_TO_CLASS` across `dlls/`)
- **How to run**:
  ```bash
  python tests/verify_constants.py --golden tests/golden/constants.json
  ```
- **How to update**:
  ```bash
  python tests/verify_constants.py --update
  ```

---

### Layer 2A — ABI Struct Layout Verification (`#103`)
- **Implementation**: [`tests/test_abi_layout.cpp`](file:///c:/code/halflife-refactored/tests/test_abi_layout.cpp)
- **Scope**:
  - Validates exact byte sizes (`sizeof`) and field offsets (`offsetof`) for all structures crossing the engine/DLL and client/server boundary:
    - `entvars_t`, `edict_t`, `globalvars_t`, `enginefuncs_t`, `DLL_FUNCTIONS`, `NEW_DLL_FUNCTIONS`
    - `playermove_t`, `usercmd_t`, `ItemInfo`, `TraceResult`, `KeyValueData`
    - `physent_t`, `pmtrace_t`, `netadr_t`, `clientdata_t`, `weapon_data_t`, `entity_state_t`, `movevars_t`
- **Execution**:
  - Evaluated entirely at compile-time via `static_assert`.
  - Built as part of the standalone test runner (`hl_tests.vcxproj` and `Makefile.tests`), completely decoupled from the production game DLL.
  - Zero runtime overhead.

---

### Layer 2B — Save/Restore Tables Verification (`#101`)
- **Script**: [`tests/verify_saverestore.py`](file:///c:/code/halflife-refactored/tests/verify_saverestore.py)
- **Golden Baseline**: [`tests/golden/saverestore_baseline.json`](file:///c:/code/halflife-refactored/tests/golden/saverestore_baseline.json)
- **Compile-Time Checks**: [`tests/test_saverestore.cpp`](file:///c:/code/halflife-refactored/tests/test_saverestore.cpp) (compiled in standalone test runner `hl_tests`, decoupled from production game DLL)
- **Scope**:
  - Verifies `TYPEDESCRIPTION` tables for player (`CBasePlayer::m_playerSaveData`), weapons (`CBasePlayerItem`, `CBasePlayerWeapon`, `CGauss`, `CEgon`, `CRpg`, `CSatchel`, `CShotgun`, `CWeaponBox`), projectiles (`CHornet`, `CSqueakGrenade`, `CRpgRocket`, `CTripmineGrenade`), and entity base classes (`CBaseEntity`, `CBaseDelay`, `CBaseAnimating`, `CBaseToggle`, `gEntvarsDescription`).
  - Ensures field count, names, serialization types, and array dimensions match expected baselines.
  - Compile-time assertions check exact member memory offsets.
- **How to run**:
  ```bash
  python tests/verify_saverestore.py --golden tests/golden/saverestore_baseline.json
  ```
- **How to update**:
  ```bash
  python tests/verify_saverestore.py --update
  ```

---

### Layer 3 — Unit Test Infrastructure (`#100`)
- **Framework**: Catch2 v3.7.1 vendored in [`external/catch2/`](file:///c:/code/halflife-refactored/external/catch2/)
- **Mock Engine**: [`tests/mock_engine.h`](file:///c:/code/halflife-refactored/tests/mock_engine.h), [`tests/mock_engine.cpp`](file:///c:/code/halflife-refactored/tests/mock_engine.cpp)
  - Provides lightweight stubs for `enginefuncs_t`, `gpGlobals`, and captures network messages via `g_mockMessageBuffer`.
- **Test Suites**:
  - [`tests/test_pm_movement.cpp`](file:///c:/code/halflife-refactored/tests/test_pm_movement.cpp): Player movement and physics functions (`PM_CalcRoll`, `PM_DropPunchAngle`, `PM_CheckParamters`, `PM_Friction`, `PM_Accelerate`, `PM_AirAccelerate`) validated against upstream golden outputs.
  - [`tests/test_combat.cpp`](file:///c:/code/halflife-refactored/tests/test_combat.cpp): Combat calculations (`RadiusDamage` linear falloff, hitgroup damage multipliers, armor absorption & depletion, network message buffering).
- **Projects / Makefiles**:
  - Windows: Visual Studio project [`projects/vs2019/hl_tests.vcxproj`](file:///c:/code/halflife-refactored/projects/vs2019/hl_tests.vcxproj) included in `projects.sln`.
  - Linux: [`linux/Makefile.tests`](file:///c:/code/halflife-refactored/linux/Makefile.tests) invoked via `make -C linux hl_tests`.
- **How to build & run**:
  - Windows:
    ```bash
    msbuild projects/vs2019/hl_tests.vcxproj /p:Configuration=Release /p:Platform=Win32 /m
    projects/vs2019/Release/hl_tests/hl_tests.exe
    ```
  - Linux:
    ```bash
    make -C linux hl_tests ARCH_CFLAGS_I686="-m32" HL_OUTPUT_DIR=release
    linux/release/hl_tests
    ```

---

### Layer 4 — Exported Symbol Table Verification (`#99`)
- **Script**: [`tests/verify_symbols.py`](file:///c:/code/halflife-refactored/tests/verify_symbols.py)
- **Golden Baselines**:
  - [`tests/golden/symbols_client_windows.txt`](file:///c:/code/halflife-refactored/tests/golden/symbols_client_windows.txt) (624 symbols)
  - [`tests/golden/symbols_server_windows.txt`](file:///c:/code/halflife-refactored/tests/golden/symbols_server_windows.txt) (495 symbols)
  - [`tests/golden/symbols_client_linux.txt`](file:///c:/code/halflife-refactored/tests/golden/symbols_client_linux.txt) (157 symbols)
  - [`tests/golden/symbols_server_linux.txt`](file:///c:/code/halflife-refactored/tests/golden/symbols_server_linux.txt) (506 symbols)
- **Features**:
  - Windows PE parser using `dumpbin /EXPORTS`.
  - Pure-Python ELF32 parser for dynamic symbol tables (`.dynsym`/`.dynstr`), with automatic fallback to `nm -D`.
  - Cross-platform verification: ELF binaries can be validated on Windows without Linux tooling.
- **How to run**:
  - Windows:
    ```bash
    python tests/verify_symbols.py --golden tests/golden/symbols_client_windows.txt --binary projects/vs2019/Release/hl_cdll/client.dll
    python tests/verify_symbols.py --golden tests/golden/symbols_server_windows.txt --binary projects/vs2019/Release/hldll/hl.dll
    ```
  - Linux:
    ```bash
    python3 tests/verify_symbols.py --golden tests/golden/symbols_client_linux.txt --binary linux/release/cl_dlls/client.so
    python3 tests/verify_symbols.py --golden tests/golden/symbols_server_linux.txt --binary linux/release/dlls/hl.so
    ```
- **How to update**:
  ```bash
  python tests/verify_symbols.py --binary <path_to_dll_or_so> --update tests/golden/<baseline_file>.txt
  ```

---

## Upstream Golden Reference Build (`#104`)

To guarantee exact numerical and behavioral equivalence without circular dependencies, golden data is computed from pristine upstream Valve SDK 2.3 reference sources:

- **Reference Sources**: [`tests/upstream_reference/`](file:///c:/code/halflife-refactored/tests/upstream_reference/) (extracted from commit `b1b5cf5892918535619b2937bb927e46cb097ba1`).
- **Generator**: [`tests/generate_golden.cpp`](file:///c:/code/halflife-refactored/tests/generate_golden.cpp)
- **Generated Outputs**:
  - [`tests/golden/pm_movement.json`](file:///c:/code/halflife-refactored/tests/golden/pm_movement.json)
  - [`tests/golden/combat.json`](file:///c:/code/halflife-refactored/tests/golden/combat.json)
  - [`tests/golden/golden_data.h`](file:///c:/code/halflife-refactored/tests/golden/golden_data.h) (C++ header embedded in unit tests)
- **How to regenerate**:
  ```bash
  cl /EHsc /std:c++14 /I dlls /I dlls/core /I engine /I common /I pm_shared /I game_shared /I public tests/generate_golden.cpp /Fe:generate_golden.exe
  ./generate_golden.exe
  ```

