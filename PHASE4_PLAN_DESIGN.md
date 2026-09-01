# Design Document - Phase 4: Monsters and AI Modularization

## Overview

Phase 4 establishes architectural structure across the Half-Life AI and Monster subsystem. The design decouples all 27 monster classes into dedicated header files, extracts auxiliary projectile and sub-entities into dedicated units, and splits the 3,693 LOC `dlls/ai/nodes.cpp` navigation graph engine into modular components governing graph state, routing algorithms, connectivity links, and debug rendering.

## System Architecture

### Component Map

| Component ID | Name | Type | Responsibility | Interfaces With |
|-------------|------|------|----------------|-----------------|
| COMP-AI-1 | Dedicated Monster Headers | Header Layer | Defines class definitions, schedules, tasks, and save data for 27 monsters | `CBaseMonster`, `CSquadMonster`, `CTalkMonster`, `CBaseEntity` |
| COMP-AI-2 | Monster Sub-Entities & Projectiles | Entity Layer | Manages monster-specific projectiles, effects, and subsidiary parts | `CBaseEntity`, Parent Monsters |
| COMP-AI-3 | Node Graph Core (`nodes_graph.cpp`) | AI Core | Graph allocation, node hash tables, node sorting, `CNodeEnt`, and disk serialization | `CGraph`, `CNode` |
| COMP-AI-4 | Node Routing Engine (`nodes_routing.cpp`) | AI Algorithm | Route path computation, Floyd-Warshall search, `PathLength`, `NextNodeInRoute`, `CStack`, `CQueue` | `CGraph`, `CBaseMonster` |
| COMP-AI-5 | Node Links & Connectivity (`nodes_links.cpp`) | AI World | Link visibility (`LinkVisibleNodes`), inline link rejection, link lookups, `CTestHull` | `CGraph`, World Engine Traces |
| COMP-AI-6 | Node Debug & Visualizer (`nodes_debug.cpp`) | AI Debug | Node connection visualizer (`ShowNodeConnections`), `CheckNode`, `CNodeViewer` | `CGraph`, Engine User Messages |

### Architecture Diagram

```
+------------------------------------------------------------------+
|                    Half-Life Server DLL (AI Engine)              |
+------------------------------------------------------------------+
                                  |
            +---------------------+---------------------+
            |                                           |
            v                                           v
+-----------------------+                   +-----------------------+
|  dlls/monsters/*.h    |                   |    dlls/ai/nodes/     |
|  - Dedicated Headers  |                   |  - nodes_graph.cpp    |
|  - Monster Sub-Entities|                  |  - nodes_routing.cpp  |
|  - Clean AI Schedules |                   |  - nodes_links.cpp    |
+-----------------------+                   |  - nodes_debug.cpp    |
            |                               +-----------------------+
            v                                           |
+-------------------------------------------------------v-----------+
|         CBaseMonster / CSquadMonster / CTalkMonster Hierarchy     |
|               CGraph / CNode / Link Network Topology              |
+-------------------------------------------------------------------+
```

## Data Flow & Subsystem Specifications

### 1. Monster Hierarchy & Headers Map

Each monster in `dlls/monsters/` receives a dedicated `.h` containing its class definition, custom schedule/task definitions, and save/restore data tables:

```
dlls/monsters/
├── aflock.h / aflock.cpp (CFlockingFlyer, CFlockingFlyerFlock)
├── agrunt.h / agrunt.cpp (CAGrunt)
├── apache.h / apache.cpp (CApache)
│   └── apache_hvr.h / apache_hvr.cpp (CApacheHVR)
├── barnacle.h / barnacle.cpp (CBarnacle)
├── barney.h / barney.cpp (CBarney, CDeadBarney)
├── bigmomma.h / bigmomma.cpp (CBigMomma, CInfoBM)
│   └── bigmomma_mortar.h / bigmomma_mortar.cpp (CBMortar)
├── bloater.h / bloater.cpp (CBloater)
├── bullsquid.h / bullsquid.cpp (CBullsquid)
│   └── bullsquid_spit.h / bullsquid_spit.cpp (CSquidSpit)
├── controller.h / controller.cpp (CController)
│   └── controller_energy.h / controller_energy.cpp (CControllerHeadBall, CControllerZapBall)
├── gargantua.h / gargantua.cpp (CGargantua)
│   └── gargantua_effects.h / gargantua_effects.cpp (CSpiral, CStomp, CSmoker)
├── genericmonster.h / genericmonster.cpp (CGenericMonster)
├── gman.h / gman.cpp (CGMan)
├── hassassin.h / hassassin.cpp (CHAssassin)
├── headcrab.h / headcrab.cpp (CHeadCrab)
│   └── baby_headcrab.h / baby_headcrab.cpp (CBabyCrab)
├── hgrunt.h / hgrunt.cpp (CHGrunt, CHGruntRepel, CDeadHGrunt)
├── houndeye.h / houndeye.cpp (CHoundeye)
├── ichthyosaur.h / ichthyosaur.cpp (CIchthyosaur)
├── islave.h / islave.cpp (CISlave)
├── leech.h / leech.cpp (CLeech)
├── monster_deadhev.h / monster_deadhev.cpp (CDeadHEV)
├── nihilanth.h / nihilanth.cpp (CNihilanth)
│   └── nihilanth_energy.h / nihilanth_energy.cpp (CNihilanthHVR)
├── osprey.h / osprey.cpp (COsprey)
├── rat.h / rat.cpp (CRat)
├── roach.h / roach.cpp (CRoach)
├── scientist.h / scientist.cpp (CScientist, CDeadScientist, CSittingScientist)
├── tentacle.h / tentacle.cpp (CTentacle)
│   └── tentacle_maw.h / tentacle_maw.cpp (CTentacleMaw)
└── zombie.h / zombie.cpp (CZombie)
```

### 2. Navigation Node Graph Decomposition

The 3,693 LOC `dlls/ai/nodes.cpp` is divided into 4 modular components:

1. **`dlls/ai/nodes_graph.cpp`**:
   - Graph initialization (`CGraph::InitGraph`).
   - Node allocation, heap sorting, and bounding range validation (`CGraph::AllocNodes`, `CGraph::SortNodes`, `CGraph::HashInsert`, `CGraph::HashSearch`, `CGraph::HashChoosePrimes`).
   - Node entity setup (`CNodeEnt`).
   - Binary serialization and network save/restore (`CGraph::FLoadGraph`, `CGraph::FSaveGraph`, `CGraph::FSetGraphPointers`, `CGraph::CheckNODFile`).

2. **`dlls/ai/nodes_routing.cpp`**:
   - Shortest path distance calculations (`CGraph::PathLength`, `CGraph::FindShortestPath`).
   - Step-by-step route traversal (`CGraph::NextNodeInRoute`).
   - Routing table construction (`CGraph::BuildRegionTables`, `CGraph::ComputeStaticRoutingTables`, `CGraph::TestRoutingTables`).
   - Route traversal structures (`CStack`, `CQueue`).

3. **`dlls/ai/nodes_links.cpp`**:
   - Link generation between adjacent waypoints (`CGraph::LinkVisibleNodes`, `CGraph::RejectInlineLinks`, `CGraph::BuildLinkLookups`).
   - Hull visibility and drop testing (`CTestHull::Spawn`, `CTestHull::DropDelay`, `CTestHull::CallBuildNodeGraph`, `CTestHull::BuildNodeGraph`, `CTestHull::PathFind`).
   - Link entity handling (`CGraph::HandleLinkEnt`, `CGraph::LinkEntForLink`, `CGraph::FindNearestLink`).

4. **`dlls/ai/nodes_debug.cpp`**:
   - Visual debugging routines (`CGraph::ShowNodeConnections`, `CGraph::CheckNode`).
   - Debug visualization entities (`CNodeViewer`).

## System Boundaries & Constraints

- **Preserve Behavior 100%**: Zero alteration to AI state machines, task schedules, relationship matrices, and pathfinding results.
- **Maintain Engine Compatibility**: All engine callback hooks (`g_engfuncs`), entity linkage macros (`LINK_ENTITY_TO_CLASS`), and `pev` field offsets remain untouched.
- **Save/Restore Integrity**: All `TYPEDESCRIPTION` tables must exactly match original fields to ensure savegame backward compatibility.

## Testing & Build Strategy

- **Build Verification**: Compile and link on both MSVC (Windows x86 `hldll.dll`) and GCC (Linux x86 `hl.so`).
- **CI Validation**: Automated execution on GitHub Actions workflow across all configurations.
