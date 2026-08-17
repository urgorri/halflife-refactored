# Linux Build Issues

When attempting to build the `hl_cdll` project locally on Linux (Ubuntu 24.04 via g++), there are several pre-existing compilation errors that block a successful build. These issues are related to the codebase age and compatibility with modern compilers.

## Findings:

1. **Missing Steam Header:**
   The client build looks for `steam/steamtypes.h` (included via `engine/cdll_int.h`), but this file is missing from the repository.

2. **Windows Types in Linux Build:**
   The file `cl_dll/inputw32.cpp` uses Windows-specific types like `POINT` (resulting in `error: ‘POINT’ does not name a type; did you mean ‘tagPOINT’?`) and is not correctly excluded from the Linux makefile configuration, causing failures when compiling `inputw32.o`.

3. **String Constant Conversion Restrictions:**
   Modern g++ correctly forbids converting a string constant to `char*` without a cast (`-Wwrite-strings`). There are multiple occurrences of this in `cl_dll/hud_spectator.cpp` (e.g., `gEngfuncs.pfnRegisterVariable( "spec_drawstatus", "1", 0 );`).

4. **Case-Insensitive String Comparisons:**
   The build uses macros like `-Dstricmp=strcasecmp`, but depending on the standard library includes (e.g. `string.h` vs `strings.h`), `stricmp` often fails to map correctly without explicit `#include <strings.h>`.

These issues were not introduced by the VGUI refactoring and require broader platform and dependency fixes beyond the scope of this update.
