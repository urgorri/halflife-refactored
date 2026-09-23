# Prompt: Design and Implementation of Automated CI Behavioral Regression Checks

> **Role & Objective**: You are a Principal CI/CD and Test Automation Architect specializing in game engine verification. Your objective is to design, implement, and integrate automated test suites and CI checks into `halflife-refactored` that automatically detect behavioral regressions, mathematical drift, uncanonical logic modifications, and level transition defects *before* code is merged to `master` and tested in-game.

---

## 1. Problem Statement: Why Existing CI Failed to Catch Runtime Bugs

The repository currently possesses 4 verification layers and a client smoke test:
- **Layer 1** (`verify_constants.py`): Validates numeric gameplay constants and enums against `constants.json`.
- **Layer 2** (`verify_saverestore.py` & struct layout tests): Validates entity struct sizes, member offsets, and `TYPEDESCRIPTION` tables against `saverestore_baseline.json`.
- **Layer 3** (`hl_tests.exe`): Catch2 unit tests for specific features.
- **Layer 4** (`verify_symbols.py`): Checks DLL export tables against `symbols_{client,server}_{windows,linux}.txt`.
- **Smoke Test** (`smoke_test_client.exe`): Dynamically loads `client.dll` and invokes `Initialize(&mockEngfuncs, 7)`, `HUD_Init()`, and `HUD_GetStudioModelInterface(1, ...)`.

### The Gaps:
1. **Static vs Dynamic**: Layers 1, 2, and 4 verify *metadata* (offsets, symbols, constants), not *runtime behavior or algorithms*.
2. **Shallow Smoke Tests**: `smoke_test_client.exe` validates that functions can be invoked without immediate segfaults on startup, but it never renders a model, never executes bone transformation loops, and never simulates entity ticks.
3. **No Level Transition Tests**: No tests exercise `trigger_changelevel`, landmark delta coordinates, or player carry-over between maps. When `ChangeList` or landmark offset math broke, no test failed.
4. **No Animation Matrix Baselines**: When `curstate.scale` or corrupted span interpolation formulas mutated bone matrices in `StudioModelRenderer`, no test verified mathematical matrix outputs against golden standards.

---

## 2. Required New CI Verification Layers to Implement

Design and implement the following automated test suites:

### Suite A: Map & Level Transition Simulation Test (Layer 3C)
- **Target File**: `tests/test_level_transition.cpp` (compiled into `hl_tests.exe`).
- **Description**:
  A headless unit/integration test suite using Catch2 and mock entity structures that tests the full changelevel pipeline:
  1. **Landmark Coordinate Delta**:
     - Given a player at `origin = (100, 200, 50)` and a source landmark `info_landmark` at `origin = (50, 50, 0)`, the relative delta is `(50, 150, 50)`.
     - Given a destination landmark in the target map at `origin = (1000, 2000, 500)`, the restored player origin must be `(1050, 2150, 550)`.
     - Test that `CChangeLevel::ChangeList`, `AddTransitionToList`, and landmark transformation math produce this exact result.
  2. **Transition Volume Checking**:
     - Test `CChangeLevel::InTransitionVolume` with entities inside and outside the brush volume.
  3. **Multiple Entities in Transition**:
     - Test that companions (Barney, Scientist) within the transition volume are correctly collected into `pLevelList` with proper landmark deltas.
  4. **Corner Cases**:
     - Empty landmark name (seamless level change vs hard level change).
     - Missing landmark in destination map (graceful fallback vs crash).
     - Player ducking / hull size adjustment across transition.

### Suite B: Studio Model Animation & Golden Matrix Test (Layer 3D)
- **Target File**: `tests/test_studio_matrix_golden.cpp` (compiled into `hl_tests.exe`).
- **Description**:
  A deterministic regression test verifying that studio bone transformations match golden mathematical baselines:
  1. Create synthetic or extracted `mstudiobone_t` and `mstudioanim_t` structures representing multi-frame walk, run, and idle sequences with bone controllers and multi-blend animations.
  2. Run the sequence through:
     - `StudioCalcBonePosition`
     - `StudioCalcBoneQuaterion`
     - `StudioCalcRotations`
     - `StudioSetUpTransform`
     - `StudioSetupBones`
  3. Compare resultant bone matrices `bonematrix[MAXSTUDIOBONES][3][4]` against a golden snapshot JSON file (`tests/golden/studio_matrices_baseline.json`) with an epsilon tolerance of `1e-4`.
  4. **Why this matters**: Any erroneous scale multiplication (e.g. `curstate.scale`), broken span interpolation formula, or corrupted slerp logic will immediately fail this test in CI before reaching a human tester.

### Suite C: Canonical Upstream Drift Scanner (Layer 5)
- **Target File**: `tests/verify_upstream_drift.py` (executed via Python in CI).
- **Description**:
  An automated script that audits critical files in `halflife-refactored` against `upstream/master` (or pinned canonical snapshots in `tests/upstream_reference/`):
  1. **Forbidden Patterns Scanner**:
     - Scan for known dangerous patterns introduced in past refactors:
       - Accesses to `pbones[pbones[i].parent]` without `pbones[i].parent != -1` guard.
       - Use of `g_PlayerExtraInfo` indexed by raw entity indices (`> MAX_PLAYERS`).
       - Uncanonical matrix scaling (`curstate.scale` in non-sprite rendering).
       - Omitted `m_pPlayerInfo = NULL;` in studio rendering entry/exit.
       - Missing `IMPLEMENT_SAVERESTORE` or omitted fields in `m_SaveData`.
  2. **AST / Token Divergence Check**:
     - For designated mission-critical algorithmic functions (e.g. `StudioSetUpTransform`, `StudioCalcBonePosition`, `InTransitionVolume`, `ChangeList`), compare token sequences against upstream canonical implementations to flag unapproved mathematical alterations.

### Suite D: Headless Server/Client Frame Execution Smoke Test
- **Target File**: `tests/smoke_test_gameplay.cpp` (compiled to `projects/vs2019/Release/smoke_test_gameplay/smoke_test_gameplay.exe` and Linux equivalent).
- **Description**:
  Extends `smoke_test_client.exe` to test a simulated frame cycle:
  1. Initialize mock server engine and mock client engine.
  2. Instantiate a player and mock entity.
  3. Execute 5 simulated frame steps:
     - `PlayerPreThink()`
     - `PlayerPostThink()`
     - `R_StudioDrawPlayer()`
     - `R_StudioDrawModel()`
  4. Assert:
     - Zero unhandled SEH / signal exceptions intercepted.
     - Frame execution time does not exceed latency thresholds (catches infinite loops or exception-handling overhead).

---

## 3. GitHub Actions CI Integration

Integrate the new checks into `.github/workflows/behavioral-equivalence.yml` and `.github/workflows/build.yml`:
1. **Linux and Windows Equivalence Workflows**:
   - Add Step: `Run Level Transition & Studio Matrix Tests (hl_tests)`
   - Add Step: `Run Upstream Canonical Drift Audit (verify_upstream_drift.py)`
   - Add Step: `Run Gameplay Frame Smoke Test (smoke_test_gameplay)`
2. **Gating Policy**:
   - PR builds must fail if:
     - Any landmark offset math deviates from expected golden coordinates.
     - Any studio bone matrix deviates from canonical golden baselines.
     - The upstream drift scanner identifies forbidden patterns or unapproved mathematical changes.

---

## 4. Implementation Steps

1. **Step 1: Baseline Generation**:
   - Create golden matrix snapshots using canonical Valve SDK algorithms.
2. **Step 2: Implement Unit & Integration Tests**:
   - Write `tests/test_level_transition.cpp` and `tests/test_studio_matrix_golden.cpp`.
   - Wire them into `CMakeLists.txt`, `projects/vs2019/hl_tests.vcxproj`, and `linux/Makefile.tests`.
3. **Step 3: Implement Upstream Drift Scanner**:
   - Write `tests/verify_upstream_drift.py` with clear, colored console output and detailed error diagnostics.
4. **Step 4: Update CI Workflows**:
   - Update `.github/workflows/behavioral-equivalence.yml` and `.github/workflows/build.yml`.
5. **Step 5: Verify Locally & in CI**:
   - Ensure all new tests pass locally on Windows and Linux, and confirm CI runs 100% green.

---

## 5. Deliverables

1. Complete source code for `test_level_transition.cpp` and `test_studio_matrix_golden.cpp`.
2. Complete script `tests/verify_upstream_drift.py`.
3. Updated project files (`hl_tests.vcxproj`, `CMakeLists.txt`, `Makefile.tests`).
4. Updated GitHub Actions workflow files with full job logs demonstrating automated catching of regressions.
