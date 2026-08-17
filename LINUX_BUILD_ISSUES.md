# Linux Build Issues & Action Plan

This document outlines the verified pre-existing Linux compilation issues blocking a clean build of `hldll` and `hl_cdll` using GCC/G++ on modern Linux distributions (e.g., Ubuntu 22.04 / 24.04).

All items below have been audited against the current codebase and remain **pending** action.

---

## Actionable Tasks

### 1. Fix Steam Header Include Resolution
- [ ] **Ensure `public/` is in include paths across all Linux makefiles**
  - **Root Cause**: `public/steam/steamtypes.h` exists in the repository, but `#include "steam/steamtypes.h"` (in `engine/cdll_int.h` and `public/archtypes.h`) fails when `PUBLIC_SRC_DIR` is not passed or `-I../public` is omitted during standalone make invocations.
  - **Affected Files**: `linux/Makefile.hl_cdll`, `linux/Makefile.hldll`, `linux/Makefile.dmc_cdll`, `linux/Makefile.ricochet_cdll`
  - **Action**: Add `-I$(SOURCE_DIR)/public` and fallback `-I../public` directly into `INCLUDEDIRS` for all subsystem makefiles.

### 2. Resolve Win32 API & Type Dependencies in Client Input
- [ ] **Guard Win32 input calls & types in `cl_dll/inputw32.cpp` (or `cl_dll/input/inputw32.cpp`)**
  - **Root Cause**: `inputw32.cpp` uses Windows-specific types (`POINT`) and calls Win32 APIs directly (`GetCursorPos`, `SetCursorPos`, `SystemParametersInfo`). On Linux, this causes `‘POINT’ does not name a type` and unresolved symbol errors.
  - **Affected Files**: `cl_dll/inputw32.cpp` (or `cl_dll/input/inputw32.cpp`), `common/port.h`, `linux/Makefile.hl_cdll`
  - **Action**:
    - Wrap Win32-specific mouse pointer manipulation in `#ifdef _WIN32` blocks with SDL2 / stub fallbacks for Linux.
    - Ensure `common/port.h` defines `POINT` consistently when building on Linux.

### 3. Fix String Constant to `char*` Conversions (`-Wwrite-strings`)
- [ ] **Replace direct `pfnRegisterVariable` calls with `CVAR_CREATE` or explicit casts**
  - **Root Cause**: Modern C++ forbids implicit conversion from string literals (`const char[]`) to non-const `char*`. While `cl_dll/cl_util.h` provides `CVAR_CREATE` with explicit casts, several files call `gEngfuncs.pfnRegisterVariable` directly with string literals.
  - **Affected Files**:
    - `cl_dll/hud_spectator.cpp` (or `cl_dll/hud/hud_spectator.cpp`)
    - `cl_dll/in_camera.cpp` (or `cl_dll/input/in_camera.cpp`)
    - `cl_dll/input.cpp` (or `cl_dll/input/input.cpp`)
    - `cl_dll/inputw32.cpp` (or `cl_dll/input/inputw32.cpp`)
  - **Action**: Update direct `gEngfuncs.pfnRegisterVariable` calls to use `CVAR_CREATE(name, value, flags)` or explicit `(char *)` casts.

### 4. Include `<strings.h>` for POSIX Case-Insensitive String Functions
- [ ] **Add `#include <strings.h>` to `common/port.h`**
  - **Root Cause**: The Linux build defines `-Dstricmp=strcasecmp` and `-D_strnicmp=strncasecmp`. On POSIX systems, `strcasecmp` and `strncasecmp` are declared in `<strings.h>`, but `common/port.h` only includes `<string.h>`, causing compilation failures when standard POSIX compliance flags are enabled.
  - **Affected Files**: `common/port.h`
  - **Action**: Add `#include <strings.h>` inside the `#ifndef _WIN32` block in `common/port.h`.

### 5. Update Linux Makefiles for Modular Directory Layout
- [ ] **Add pattern rules and output directory creation for refactored folders in `linux/Makefile.hl_cdll`**
  - **Root Cause**: The client codebase has been restructured into subdirectories (`vgui/`, `hud/`, `studio/`, `input/`). The makefile must create these target build directories in `$(HL1_OBJ_DIR)` and provide matching compilation rules (`$(HL1_OBJ_DIR)/hud/%.o`, etc.).
  - **Affected Files**: `linux/Makefile.hl_cdll`
  - **Action**: Ensure `mkdir -p` includes `hud`, `studio`, `input`, and appropriate `$(HL1_OBJ_DIR)/subfolder/%.o` compile rules are declared.
