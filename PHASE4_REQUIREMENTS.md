# Requirements Document - Phase 4: Monsters and AI Modularization

## Introduction

Phase 4 focuses on standardizing and modularizing the AI and Monster ecosystem within the Half-Life server DLL (`dlls/monsters/` and `dlls/ai/`). Currently, monster entities embed their complete class declarations, schedule tables, task lists, and auxiliary projectile/sub-entities within monolithic `.cpp` files with no dedicated header files. Additionally, the node navigation graph (`dlls/ai/nodes.cpp`, 3,694 LOC) concentrates graph initialization, routing tables, link validation, and debug visualization in a single file.

This phase introduces dedicated header files for all monster classes, isolates monster-specific projectiles and helper entities into modular translation units, and decomposes the navigation graph into specialized modules while strictly preserving binary behavior, schedule execution, and network save/restore states.

## Glossary

- **CBaseMonster**: The base class for all AI characters, handling schedule selection, task execution, condition evaluation, and relationship tracking.
- **CSquadMonster**: Extension of `CBaseMonster` enabling squad coordination, group formations, and shared enemy tracking.
- **CTalkMonster**: Base class for friendly/interactive NPCs (e.g., Barney, Scientist) supporting dialogue, following behaviors, and player interaction.
- **CGraph**: The node graph managing world navigation waypoints, connectivity links, Floyd-Warshall/Dijkstra route tables, and trace hull validation.
- **Monster Schedule / Task**: State machine execution blocks determining monster behaviors (e.g., `SCHED_CHASE_ENEMY`, `TASK_RANGE_ATTACK1`).
- **Sub-Entity / Projectile**: Auxiliary entities spawned by monsters (e.g., `CApacheHVR`, `CSquidSpit`, `CBabyCrab`, `CBMortar`, `CControllerHeadBall`, `CControllerZapBall`, `CStomp`, `CSpiral`, `CSmoker`, `CNihilanthHVR`, `CTentacleMaw`).

## Requirements

### Requirement REQ-4.1: Dedicated Monster Class Headers
**User Story:** As an engine developer, I want each monster entity to have a dedicated header file defining its class interface, save/restore tables, schedules, and task enumerations, so that monster behaviors can be cleanly referenced and extended without monolithic header pollution.

#### Acceptance Criteria
1. THE system SHALL provide dedicated header files (`dlls/monsters/<monster>.h`) for all 27 monster files in `dlls/monsters/`:
   - Human & Humanoids: `barney.h`, `scientist.h`, `hgrunt.h`, `hassassin.h`, `gman.h`, `genericmonster.h`, `monster_deadhev.h`.
   - Alien Wildlife & Xen Creatures: `agrunt.h`, `apache.h`, `barnacle.h`, `bigmomma.h`, `bloater.h`, `bullsquid.h`, `controller.h`, `gargantua.h`, `headcrab.h`, `houndeye.h`, `ichthyosaur.h`, `islave.h`, `leech.h`, `nihilanth.h`, `osprey.h`, `rat.h`, `roach.h`, `tentacle.h`, `zombie.h`, `aflock.h`.
2. WHERE monster classes declare custom schedules and tasks, THE monster header SHALL declare their respective enum identifiers and schedule declarations.
3. WHERE monster files declare secondary entities or dead-body variants (e.g., `CDeadBarney`, `CDeadScientist`, `CSittingScientist`, `CHGruntRepel`, `CDeadHGrunt`, `CFlockingFlyerFlock`), THE header SHALL provide clean class declarations for all associated entities.
4. THE system SHALL prevent circular header inclusions through clean forward declarations and include guards.

### Requirement REQ-4.2: Extraction of Monster Projectiles and Sub-Entities
**User Story:** As a gameplay programmer, I want monster-spawned projectiles, effects, and auxiliary entities extracted into distinct translation units, so that monster source files focus strictly on AI logic and state progression.

#### Acceptance Criteria
1. THE system SHALL isolate auxiliary entities and projectiles into dedicated `.h` and `.cpp` files:
   - `dlls/monsters/apache_hvr.h` / `apache_hvr.cpp`: `CApacheHVR` (`hvr_rocket`).
   - `dlls/monsters/bullsquid_spit.h` / `bullsquid_spit.cpp`: `CSquidSpit` (`squidspit`).
   - `dlls/monsters/baby_headcrab.h` / `baby_headcrab.cpp`: `CBabyCrab` (`monster_babycrab`).
   - `dlls/monsters/bigmomma_mortar.h` / `bigmomma_mortar.cpp`: `CBMortar` (`bmortar`).
   - `dlls/monsters/controller_energy.h` / `controller_energy.cpp`: `CControllerHeadBall` (`controller_head_ball`) and `CControllerZapBall` (`controller_energy_ball`).
   - `dlls/monsters/gargantua_effects.h` / `gargantua_effects.cpp`: `CSpiral` (`streak_spiral`), `CStomp` (`garg_stomp`), `CSmoker` (`env_smoker`).
   - `dlls/monsters/nihilanth_energy.h` / `nihilanth_energy.cpp`: `CNihilanthHVR` (`nihilanth_energy_ball`).
   - `dlls/monsters/tentacle_maw.h` / `tentacle_maw.cpp`: `CTentacleMaw` (`monster_tentaclemaw`).
2. THE extracted sub-entities SHALL preserve exact class names, classname strings, spawnflags, and save/restore data descriptors.
3. IF a monster references its sub-entity, THEN it SHALL interact through the sub-entity's public interface header.

### Requirement REQ-4.3: Decomposition of Navigation Node Graph (nodes.cpp)
**User Story:** As an AI systems engineer, I want `nodes.cpp` (3,693 LOC) partitioned into focused modules for graph core management, path routing, link generation, and debug rendering, so that pathfinding logic is maintainable and testable.

#### Acceptance Criteria
1. THE system SHALL decompose `dlls/ai/nodes.cpp` into:
   - `dlls/ai/nodes_graph.cpp`: Graph memory allocation, node arrays, node hash lookup (`InitGraph`, `AllocNodes`, `SortNodes`, `HashInsert`, `HashSearch`, `HashChoosePrimes`), node entity parsing (`CNodeEnt`), and save/restore serialization (`FLoadGraph`, `FSaveGraph`, `FSetGraphPointers`, `CheckNODFile`).
   - `dlls/ai/nodes_routing.cpp`: Routing table computation, Floyd-Warshall shortest path search, `PathLength`, `NextNodeInRoute`, `FindShortestPath`, `BuildRegionTables`, `ComputeStaticRoutingTables`, `TestRoutingTables`, `CStack`, `CQueue`.
   - `dlls/ai/nodes_links.cpp`: Connectivity link generation (`LinkVisibleNodes`, `RejectInlineLinks`, `BuildLinkLookups`), link ent evaluation (`HandleLinkEnt`, `LinkEntForLink`, `FindNearestLink`), and hull trace validation (`CTestHull`).
   - `dlls/ai/nodes_debug.cpp`: Node link drawing (`ShowNodeConnections`), nearest node query diagnostics (`CheckNode`), and debug visualization (`CNodeViewer`).
2. THE navigation graph SHALL retain identical spatial node lookups, connectivity hashes, and route calculation results across all 4 hull types (`HULL_POINT`, `HULL_HUMAN`, `HULL_LARGE`, `HULL_HEADCRAB`).
3. THE system SHALL preserve `CGraph` public and internal method signatures without altering the engine callback interface.

### Requirement REQ-4.4: Build Manifest and Multi-Platform Validation
**User Story:** As a release engineer, I want all newly created header and source files registered across all build systems, so that Windows (MSVC) and Linux (GCC) builds pass with zero warnings or link errors.

#### Acceptance Criteria
1. THE system SHALL register all new files in `projects/vs2019/hldll.vcxproj` and `projects/vs2019/hldll.vcxproj.filters`.
2. THE system SHALL register all new files in `dlls/Makefile` and `linux/Makefile.hldll`.
3. THE continuous integration workflow SHALL compile and link all x86 server DLL targets without errors.
