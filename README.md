# GoldSrc Refactored

A behavior-preserving refactor of the Half-Life 1 GoldSrc game DLL codebase.

This fork focuses exclusively on improving the **internal architecture, organization, readability, and maintainability** of the original code while keeping its functionality and externally observable behavior unchanged.

## Goals

* Refactor complex or difficult-to-maintain code.
* Remove duplicated logic.
* Improve modularity and separation of responsibilities.
* Split entities and responsibilities into focused files where practical.
* Reduce unnecessary coupling and complexity.
* Improve consistency and readability.
* Preserve the original game behavior and functionality.

This project is **not intended to add gameplay features or change how Half-Life works**.

## Scope & Target Architecture

This repository targets exclusively the **base Half-Life single-player and multiplayer experience** (`dlls/`, `cl_dll/`, `pm_shared/`, `game_shared/`).

Unused legacy mod components (`ricochet/`, `dmc/`, etc.) and unreferenced source stubs are intentionally excluded from active maintenance.

### Subsystem Architecture Highlights

- **Monsters Subsystem (`dlls/monsters/`)**: Monster entities keep their tightly coupled projectile/satellite entities co-located (e.g. `headcrab.cpp` manages `babyheadcrab`, `nihilanth.cpp` manages `nihilanth_energy`, `apache.cpp` manages `apache_hvr`, etc.).
- **Map Rules (`dlls/gameplay/maprules.cpp`, `maprules.h`)**: Consolidated all 11 `game_*` map rule entities into a single coherent rules manager.
- **Triggers Subsystem (`dlls/systems/`)**: Triggers are logically partitioned into:
  - `triggers_brush.cpp`/`.h`: Volumetric BSP touch triggers (`trigger_multiple`, `trigger_once`, `trigger_hurt`, `trigger_push`, `trigger_teleport`, `trigger_changelevel`, `func_ladder`, `func_friction`, etc.).
  - `triggers_point.cpp`/`.h`: Point logic and relay triggers (`trigger_auto`, `trigger_relay`, `trigger_camera`, `multi_manager`, `env_render`, `target_cdaudio`, `fireanddie`).
- **Entity Subsystems (`dlls/systems/`, `dlls/world/`)**:
  - `chargers.cpp`/`.h`: Unified wall-mounted charging stations (`func_healthcharger`, `func_recharge`).
  - `doors.cpp`/`.h`: Unified door controllers (`func_door`, `func_door_rotating`, `momentary_door`).
  - `func_tank.cpp`/`.h`: Unified mounted weapons (`func_tank`, `func_tanklaser`, `func_tankrocket`, `func_tankmortar`).
  - `xen.cpp`/`.h`: Unified Xen environmental entities (`xen_tree`, `xen_spore_*`).
- **Items Subsystem (`dlls/items/`)**: Unified standard world pickup entities (`item_suit`, `item_battery`, `item_healthkit`, `item_antidote`, `item_security`, `item_longjump`) using declarative `IMPLEMENT_WORLD_ITEM` macros.
- **Weapons Subsystem (`dlls/weapons/`)**: Deduplicated ground weapon pickups using `INITIALIZE_WORLD_WEAPON`.

## Development Philosophy

The target is a cleaner internal implementation of the original GoldSrc DLL:

> **Same behavior. Better code.**

Changes should provide a concrete architectural or maintenance benefit. Cosmetic refactoring and unnecessary abstractions should be avoided.

## Building

### CMake (Recommended)

A modern cross-platform CMake build system is provided to compile the server library (`hl`), client library (`client`), and behavioral equivalence test suite (`hl_tests`).

#### Windows (Visual Studio 2019 / 2022 / BuildTools)

Configure for 32-bit x86 architecture and compile:

```cmd
cmake -B build -A Win32
cmake --build build --config Release
```

Output binaries are generated in `build/bin/Release/`:
- `hl.dll` (Server)
- `client.dll` (Client)
- `hl_tests.exe` (Behavioral equivalence test suite)

To run the test suite:

```cmd
ctest --test-dir build -C Release --output-on-failure
```

#### Linux (GCC / Clang x86)

Ensure 32-bit multilib tools and CMake are installed:

```bash
sudo dpkg --add-architecture i386
sudo apt-get update && sudo apt-get install -y gcc-multilib g++-multilib make cmake
```

Configure and compile:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Output binaries are generated in `build/bin/`:
- `hl.so` (Server)
- `client.so` (Client)
- `hl_tests` (Behavioral equivalence test suite)

To run tests:

```bash
ctest --test-dir build --output-on-failure
```

#### Downstream Mod Extension Hooks

Downstream mods and forks can place custom translation units inside `dlls/custom/` or `cl_dll/custom/` (or create subdirectories). CMake automatically discovers and compiles them into `hl` and `client` respectively via `CONFIGURE_DEPENDS`, eliminating git merge conflicts on upstream pulls.

### Legacy Build Systems (Backward Compatibility)

The original SDK build definitions remain fully retained and functional:

* **Windows**: `projects/vs2019/projects.sln` (`hldll.vcxproj`, `hl_cdll.vcxproj`, `hl_tests.vcxproj`)
* **Linux**: `linux/Makefile` (`make hl`, `make hl_cdll`, `make hl_tests`)

## License

This project is derived from the **Half-Life 1 SDK** originally released by Valve Corporation.

The original SDK license is retained in this repository:

* [`LICENSE`](./LICENSE)

Copyright © Valve Corp.

See the included license file for the complete terms governing the SDK and derivative works.

## Disclaimer

This project is an independent refactoring effort based on the publicly available Half-Life 1 SDK.

It does not represent an official Valve project and is not affiliated with or endorsed by Valve Corporation.
