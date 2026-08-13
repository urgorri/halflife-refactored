# Findings

## DLL Core Files Refactoring Issues
Moving DLL core files (`cbase.cpp`, `globals.cpp`, `util.cpp`, and their headers) to `dlls/core/` broke the build across multiple build environments:

1. **MSBuild Project Configuration (`hldll.vcxproj`)**:
   - Compiling files `cbase.cpp`, `globals.cpp`, and `util.cpp` failed because the project file still pointed to `dlls/` instead of `dlls/core/`.
   - Header references in the project were similarly outdated.
   - The filters file (`hldll.vcxproj.filters`) did not have the correct file mapping paths.

2. **Header Resolution Paths**:
   - `cl_dll/cl_dll.h` hardcoded the relative path `#include "../dlls/cdll_dll.h"`, which became invalid when the file was moved to `dlls/core/`.
   - Files under `game_shared/` (e.g., `voice_gamemgr.cpp` and bot logic) could not find `"extdll.h"`, `"util.h"`, and `"cbase.h"` because `dlls/core` was not part of the header search path of `hldll.vcxproj` and `hl_cdll.vcxproj`.

3. **Linux Makefiles (`linux/Makefile.hldll`, `linux/Makefile.hl_cdll`, `dlls/Makefile`)**:
   - Makefile include paths lacked the new `core/` folder.
   - `linux/Makefile.hldll` still attempted to compile `cbase.cpp`, `globals.cpp`, and `util.cpp` from `dlls/` and lacked rules and target folders for `dlls/core/`.
