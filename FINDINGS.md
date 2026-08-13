# Findings

## Discovered Problems

1. **Syntax Error in GitHub Actions Workflow (`build.yml`)**:
   - In [.github/workflows/build.yml](file:///E:/Dev/urgorri/halflife-refactored/.github/workflows/build.yml), the PowerShell verify script under the `build-windows-server` job (lines 59–65) was missing a closing curly brace `}` for the `if` block. This prevented the script from parsing and execution, causing validation to fail.

2. **Missing Directories for Post-Build Binary Copying (`filecopy.bat`)**:
   - The MSBuild configurations (`*.vcxproj` under `projects/vs2019/`) invoke a post-build step calling [filecopy.bat](file:///E:/Dev/urgorri/halflife-refactored/filecopy.bat) to copy target `.dll` and `.pdb` files to `..\..\..\game\mod\cl_dlls\` or `..\..\..\game\mod\dlls\`.
   - These target directories do not exist in clean checkouts or inside GitHub Actions runners.
   - The standard `copy` command does not create non-existent destination directories, causing the command to fail with code 1, which in turn causes the entire build job to fail.

3. **Hardcoded GCC-5/G++-5 Compiler Versions in Linux Makefile (`linux/Makefile`)**:
   - The Linux Makefile hardcoded `gcc-5` and `g++-5` as the compiler executables (`CC` and `CPLUS`).
   - Modern Linux distributions and CI runners (such as `ubuntu-24.04` used in the GitHub Actions workflow) do not have these legacy compiler packages in their default repositories, causing `make` to fail with command-not-found errors.

4. **Missing Directory Creation for Refactored Items in Makefiles (`dlls/Makefile`, `linux/Makefile.hldll`)**:
   - The refactor that split `items.cpp` into individual files under `items/` introduced new object targets like `$(HLDLL_OBJ_DIR)/items/item_base.o`.
   - The Makefiles' directory creation targets (`neat` and `dirs`) were not updated to create the `items` subdirectory under the object output folders, resulting in compiler errors on Linux due to non-existent target directories.

5. **Incorrect Item Compilation Path in Ricochet Linux Makefile (`linux/Makefile.ricochetdll`)**:
   - `linux/Makefile.ricochetdll` was incorrectly modified to compile modular item object files (`items/item_base.o`, etc.), but the Ricochet game folder was not refactored and still uses the monolithic `items.cpp` and `items.h` in `ricochet/dlls/`.
   - This mismatch caused compiler errors when attempting to locate non-existent source files in `ricochet/dlls/items/`.


