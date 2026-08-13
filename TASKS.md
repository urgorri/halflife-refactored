# Tasks

- [x] Fix compilation issues caused by DLL core files refactor:
  - [x] Update file compilation paths in `hldll.vcxproj` and `hldll.vcxproj.filters` to compile from `dlls/core/`.
  - [x] Update include paths in `hldll.vcxproj` and `hl_cdll.vcxproj` to search `dlls/core/`.
  - [x] Update relative include of `cdll_dll.h` in `cl_dll/cl_dll.h`.
  - [x] Update Linux makefiles (`dlls/Makefile`, `linux/Makefile.hldll`, `linux/Makefile.hl_cdll`) with the new core directory variables, include paths, compilation rules, object lists, and directory creations.
