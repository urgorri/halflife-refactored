# Legacy Linux Build Fixes

During the refactoring of the Studio and Input subsystems (Task 8.3), local Linux builds using GCC multilib exposed numerous legacy issues with compatibility:

1. **`_snprintf` vs `snprintf`**: Standardized by mapping `_snprintf` to `snprintf`.
2. **Missing `stdio.h`**: Included `<stdio.h>` where `snprintf` was newly mapped.
3. **`stricmp` vs `strcasecmp`**: Replaced non-standard `stricmp` usage with standard `strcasecmp` via `#define stricmp strcasecmp` alongside `<string.h>` includes.
4. **`_alloca` vs `alloca`**: Replaced Windows-specific `_alloca` with standard `alloca` via `<alloca.h>`.
5. **Dynamic Loading (`dlfcn.h`)**: Correctly included `<dlfcn.h>` and mapped Windows `HMODULE` and `GetProcAddress` to standard Linux types like `void*` and `dlsym`.
6. **`POINT` struct**: `in_camera.cpp` failed without a `tagPOINT` declaration on Linux, which was corrected by defining `POINT`.
7. **Includes to `const.h` and `archtypes.h`**: Due to path resolutions, engine and common folder dependencies needed correct relative paths, replacing missing `#include "const.h"` with `#include "../common/const.h"`.

These fixes enabled the client library to compile correctly on Ubuntu 24.04 (GCC multilib).
