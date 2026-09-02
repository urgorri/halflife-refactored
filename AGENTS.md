# AGENTS.md

## Purpose

This repository is a behavior-preserving refactor of the Half-Life GoldSrc DLL.

The goal is to improve the internal codebase without changing how the game works.

## Scope & Target

* **Target**: Base Half-Life single-player and multiplayer (`dlls/`, `cl_dll/`, `pm_shared/`, `game_shared/`).
* **Out of Scope**: Unused legacy mod directories (`ricochet/`, `dmc/`, etc.) are out of scope and should not be refactored or maintained.

## Core Rules

* Preserve all existing game behavior and functionality.
* Do not modify public interfaces, protocols, or externally observable behavior.
* Prefer refactoring over rewriting.
* Remove duplicated logic where safe.
* Reduce unnecessary coupling and complexity.
* Separate responsibilities into focused modules/files.
* Keep each entity or responsibility in its own file where practical.
* Avoid excessive fragmentation: do not over-modularize self-contained entities if splitting them hurts readability or navigation ergonomics.
* Modularize primarily when logic can be shared, reused, or when a file has clearly distinct, cohesive sub-responsibilities.
* Prefer simple, explicit code over unnecessary abstractions.
* Do not introduce dependencies without a clear justification.
* Preserve compatibility with the existing build and runtime environment.

## Workflow

Before making changes:

1. Inspect the relevant code and surrounding dependencies.
2. Identify the smallest safe change that solves the problem.

## Refactoring Standard

A good refactor should make the code:

* easier to understand
* easier to navigate
* less duplicated (shared logic extracted for reuse)
* less coupled
* appropriately modular without over-fragmentation
* easier to test and maintain

Do not refactor code merely for stylistic preference.

## Priority

When choosing between improvements, prioritize:

1. Correctness and behavior preservation
2. Architectural improvements and reusable modularity
3. Reduction of duplication and coupling
4. Maintainability, ergonomics, and readability
5. Cosmetic consistency

The final implementation should feel like a professionally engineered, internally optimized version of the original GoldSrc DLL while remaining functionally equivalent.

