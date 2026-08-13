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

## Development Philosophy

The target is a cleaner internal implementation of the original GoldSrc DLL:

> **Same behavior. Better code.**

Changes should provide a concrete architectural or maintenance benefit. Cosmetic refactoring and unnecessary abstractions should be avoided.


## Building

Build requirements and procedures remain based on the original Half-Life SDK.

For Windows, the original SDK provides Visual Studio projects under:

```text
projects/vs2019
```

For Linux, build files are provided under:

```text
linux
```

Refer to the original SDK documentation and project files for environment-specific requirements.

## License

This project is derived from the **Half-Life 1 SDK** originally released by Valve Corporation.

The original SDK license and required third-party license information are retained in this repository:

* [`license.txt`](./license.txt)
* [`third_party_licenses.txt`](./third_party_licenses.txt)

Copyright © Valve Corp.

See the included license files for the complete terms governing the SDK and derivative works.

## Disclaimer

This project is an independent refactoring effort based on the publicly available Half-Life 1 SDK.

It does not represent an official Valve project and is not affiliated with or endorsed by Valve Corporation.
