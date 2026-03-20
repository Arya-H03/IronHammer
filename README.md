# IronHammer Engine

> A custom 2D game engine built from scratch in **C++17** — featuring an archetype-based ECS, hand-rolled memory allocators, a three-stage physics pipeline, a multithreaded debug logger, and a full **Dear ImGui** editor.

---

## Table of Contents

- [Overview](#overview)
- [Tech Stack](#tech-stack)
- [Architecture at a Glance](#architecture-at-a-glance)
- [Entity Component System](#entity-component-system)
  - [Why Archetypes?](#why-archetypes)
  - [Component Self-Registration](#component-self-registration)
  - [Queries](#queries)
  - [CommandBuffer](#commandbuffer)
  - [Entity Generation Safety](#entity-generation-safety)
- [Memory Allocators](#memory-allocators)
  - [TemplatedSlabAllocator](#templatedslaballocator)
  - [TypeErasedBlockAllocator](#typeerasedblocallocator)
  - [TypeErasedSlabAllocator](#typeerasedslabollocator)
- [Physics Pipeline](#physics-pipeline)
  - [Stage 1 — Broad Phase](#stage-1--broad-phase)
  - [Stage 2 — Narrow Phase](#stage-2--narrow-phase)
  - [Stage 3 — Resolution](#stage-3--resolution)
- [Rendering System](#rendering-system)
- [Debug Logger](#debug-logger)
- [Editor](#editor)
  - [Viewport & Coordinate Systems](#viewport--coordinate-systems)
  - [Transform Gizmo](#transform-gizmo)
  - [Prefab System](#prefab-system)
  - [Scene Serialization](#scene-serialization)
- [Project Structure](#project-structure)

---

## Overview

IronHammer started as a question: *what does a game engine actually look like under the hood?*

The answer turned out to be every system from scratch — no engine framework, no scaffolding, no middleware beyond SFML for windowing. Every architectural decision had to be made deliberately, from how entities are stored in memory to how the editor converts between three different coordinate systems.

The engine is built around **Data-Oriented Design (DOD)**: organise data the way the CPU wants to read it, keep things contiguous, minimise indirection, and never pay for abstraction that doesn't earn its cost.

---

## Tech Stack

| | |
|---|---|
| **Language** | C++17 |
| **Windowing / Rendering** | SFML 2.6 |
| **Editor UI** | Dear ImGui |
| **Serialization** | nlohmann/json |
| **Stack Traces** | backward-cpp |
| **Profiling** | Tracy |
| **Build System** | CMake |

---

## Architecture at a Glance

```
IronHammer
├── Engine          ← Main loop, play/edit mode, world management
├── Editor          ← All Dear ImGui panels and tools
│   ├── ViewportPanel
│   ├── InspectorPanel
│   ├── DebugPanel
│   ├── BottomPanel (log window)
│   └── MenuBarPanel
├── ECS
│   ├── World       ← Entity creation, queries, serialization
│   ├── Archetype   ← SoA chunk storage
│   ├── ComponentRegistry ← Self-registering type system
│   ├── CommandBuffer ← Deferred structural mutations
│   └── Query       ← Bitset-masked archetype filtering
├── Physics
│   ├── BroadPhaseCollisionSystem  ← Spatial grid
│   ├── NarrowPhaseCollisionSystem ← AABB + prev-frame normals
│   └── CollisionResolutionSystem  ← Impulse response
├── Rendering
│   └── RenderSystem ← Batched vertex array rendering
├── Core
│   ├── Memory      ← 3 custom allocators
│   └── Debug       ← Multithreaded logger
└── Scene
    └── SceneManager ← JSON save/load
```

The main loop in `IronHammer.h` is intentionally simple — all complexity lives in the systems, not the driver:

```cpp
void Run()
{
    while (m_engine.GetRenderWindow().isOpen())
    {
        float dt = m_engine.BeginFrame();
        m_engine.UpdateRuntime();   // ECS + physics + gameplay
        m_editor.Update();          // All ImGui panels
        m_engine.RenderFrame();     // SFML draw call
        m_engine.EndFrame();

        Debug::FlushLogs();         // Move resolved logs to read queue
    }
    Debug::ShutdownLoggerThread();  // Join before exit
}
```

---

## Entity Component System

### Why Archetypes?

In a typical object-oriented approach, an entity holds *pointers* to its components — a `Transform*`, a `Sprite*`, a `RigidBody*`. Iterating thousands of entities means chasing a new pointer for every component access, causing CPU cache misses and stalling the processor.

IronHammer uses an **archetype-based ECS** where entities that share the same set of component types are stored together in contiguous memory. Component arrays are laid out as **Structure of Arrays (SoA)**:

```
Archetype {Transform, Sprite, RigidBody}

  Chunk 0:
    [T0][T1][T2]...[T63]   ← all Transforms packed together
    [S0][S1][S2]...[S63]   ← all Sprites packed together
    [R0][R1][R2]...[R63]   ← all RigidBodies packed together
```

Iterating transforms across an entire chunk is a single sequential memory read. The CPU prefetcher sees the pattern and loads data before it's needed. No pointer chasing, no cache misses.

**Sparse / Dense index mapping** makes component membership O(1) without a hash map:

```
m_sparse[componentId]  →  gives the allocator (dense) index
m_densIds[denseIndex]  →  gives back the componentId (for validation)
```

Component access by template resolves to direct pointer arithmetic into the SoA block:

```cpp
template <typename Component>
Component* GetComponentPtrByTemplate(const EntityStorageLocation& loc)
{
    ComponentId id    = ComponentRegistry::GetComponentID<Component>();
    uint16_t denseIdx = m_sparse[id];                    // O(1) lookup

    void* base        = m_chunks[loc.chunkIndex].components[denseIdx];
    char* byteBase    = static_cast<char*>(base);

    // Direct pointer arithmetic — no search, no indirection
    return reinterpret_cast<Component*>(byteBase + loc.indexInChunk * sizeof(Component));
}
```

---

### Component Self-Registration

Every component registers itself **before `main()` runs** using the `REGISTER_COMPONENT(T)` macro, which injects a static initializer storing a `ComponentInfo` struct in a global `ComponentRegistry`.

`ComponentInfo` holds type-erased function pointers for every operation the engine needs to perform on a component — without knowing its type at the call site:

```cpp
struct ComponentInfo
{
    ComponentId  id;
    size_t       size;
    const char*  name;

    void (*ConstructDefault)(void* dst);
    void (*DestroyComponent)(void* ptr);
    void (*MoveComponent)(void* src, void* dst, size_t srcIdx, size_t dstIdx);
    void (*SerializeComponent)(Json& json, void* ptr);
    void (*DeSerializeComponent)(const Json& json, void* dst);
    void (*DisplayComponent)(void* ptr, const std::function<void()>& removeCb, bool* isDirty);
};
```

This means the archetype system, the serializer, and the editor inspector can all operate on any component type through a single uniform interface. Adding a new component is:

1. Write the struct
2. Write `to_json` / `from_json`
3. Write `GuiInspectorDisplay`
4. Call `REGISTER_COMPONENT(MyComponent)`

Nothing else changes.

---

### Queries

Systems declare what they need through a `Query` — a pair of bitset masks:

```cpp
// "Give me all entities that have CTransform AND CSprite, but NOT CNotDrawable"
Query* spriteQuery = world->Query<
    RequiredComponents<CSprite, CTransform>,
    ExcludedComponents<CNotDrawable>
>();
```

When a new archetype is created, it is tested against all registered queries immediately. Matching archetypes are added to the query's list. At runtime, iterating a query is a flat loop — no per-frame filtering, no searching:

```cpp
for (auto& archetype : spriteQuery->GetMatchingArchetypes())
    for (auto& chunk : archetype->GetChunks())
    {
        auto sprites    = chunk.GetComponentRow<CSprite>();
        auto transforms = chunk.GetComponentRow<CTransform>();

        for (size_t i = 0; i < chunk.size; ++i)
            ProcessEntity(sprites[i], transforms[i]);
    }
```

The match check uses bitwise AND on two `std::bitset` masks:

```cpp
bool matches = ((required & signature) == required)
            && ((excluded & signature) == 0);
```

---

### CommandBuffer

You cannot safely add or remove components while iterating archetypes — doing so would invalidate chunk iterators mid-loop. The `CommandBuffer` defers all structural changes and flushes them after all systems finish.

It supports two paths:

**Templated path** — full type safety, captures components in a `std::tuple` inside a lambda:

```cpp
template <typename... Components>
void CreateEntityFromComponents(Components&&... comps)
{
    auto tuple = std::make_tuple(std::forward<Components>(comps)...);

    m_createEntityCommandsTemplated.push_back({ nullptr,
        [tuple = std::move(tuple)](EntityManager& em) mutable {
            return std::apply([&](auto&&... c) {
                return em.CreateEntity(std::forward<decltype(c)>(c)...);
            }, std::move(tuple));
        }
    });
}
```

**Type-erased path** — for entities loaded from JSON templates at runtime, no templates needed at the call site:

```cpp
std::vector<PendingComponent>& CreateEntityFromTemplate(
    std::vector<PendingComponent>&& components)
{
    m_createEntityCommandsTypeErased.push_back({ nullptr, std::move(components) });
    return m_createEntityCommandsTypeErased.back().pendingComponents;
}
```

---

### Entity Generation Safety

Entity IDs are reused when entities are destroyed using a **free list**. To prevent stale handles from pointing at recycled slots, every `Entity` carries a **32-bit generation counter**:

```
Entity { id: 42, generation: 3 }
          │                └── increments each time slot 42 is recycled
          └── index into entity slot arrays
```

Validation checks both:

```cpp
bool isOccupied = m_entitySlots[entity.id].isOccupied;
bool isGenValid = m_entitySlots[entity.id].generation == entity.generation;
```

A system holding an old handle after the entity was destroyed will fail the generation check and get `nullptr` — zero-overhead protection with no runtime cost on the happy path.

---

## Memory Allocators

### TemplatedSlabAllocator

Designed for fixed-type, high-frequency allocation. Each slab holds `SlabSize` objects and tracks free slots using a **bitset** — `1` = free, `0` = used.

Finding a free slot uses `std::countr_zero` — a C++20 intrinsic that maps to a **single CPU instruction** (`TZCNT`/`BSF`). For slabs under 64 elements, the entire free-list scan is one 64-bit operation:

```cpp
size_t FindFirstFreeBit()
{
    if constexpr (wordCount == 1)
    {
        // Single instruction — finds lowest set bit
        if (bitSet.single != 0) return std::countr_zero(bitSet.single);
    }
    else
    {
        for (size_t i = 0; i < wordCount; ++i)
            if (bitSet.multiple[i] != 0)
                return (i * 64) + std::countr_zero(bitSet.multiple[i]);
    }
}
```

Allocation marks the bit as used. Deallocation sets it back. No pointer chasing. No linked list. Amortised O(1).

---

### TypeErasedBlockAllocator

Used inside archetypes to hand out raw memory blocks for component SoA arrays. Operates without type knowledge — only element size and block capacity. Allocations are **64-byte aligned** to ensure component arrays start on cache line boundaries:

```cpp
void AllocateNewBlock()
{
    void* newBlock = operator new(m_dataPerBlock * m_dataSize, std::align_val_t(64));
    m_allBlocks.push_back(newBlock);
    m_freeBlocks.push_back(newBlock);
}
```

---

### TypeErasedSlabAllocator

Extends the slab concept to types only known at runtime. Uses a **free-list linked through the slots themselves** — the `nextFreeSlot` pointer lives in the same memory as the data, so no external bookkeeping is needed:

```cpp
struct Slot
{
    Slot* nextFreeSlot;
    alignas(64) char data[1]; // actual data goes here
};
```

On allocation, the free-list head is popped. On deallocation, the slot is pushed back. O(1) for both operations.

---

## Physics Pipeline

The pipeline runs in three explicit stages each frame. Separating them keeps each stage independently testable and optimisable.

```
All Entities
     │
     ▼
┌─────────────────────────────┐
│  BROAD PHASE (Spatial Grid) │  ← Rules out pairs that can't possibly collide
│  O(n) candidate generation  │
└──────────────┬──────────────┘
               │  Candidate Pairs
               ▼
┌─────────────────────────────┐
│  NARROW PHASE (AABB)        │  ← Exact overlap test + normal disambiguation
│  Previous-frame data        │
└──────────────┬──────────────┘
               │  Confirmed Collisions (normal, penetration depth)
               ▼
┌─────────────────────────────┐
│  RESOLUTION (Impulse)       │  ← Velocity correction + positional separation
│  Mass-weighted response     │
└─────────────────────────────┘
```

---

### Stage 1 — Broad Phase

The naive approach — checking every entity against every other — is **O(n²)**. At 200 entities that's 20,000 checks per frame.

The broad phase uses a **uniform spatial grid**: the world is divided into fixed 8-unit cells. Each entity with a collider registers into every cell its AABB overlaps. Only entities sharing at least one cell become candidate pairs.

```
World Grid (8-unit cells):

[ ][ ][ ][ ][ ][ ]
[ ][A][A][ ][ ][ ]   ← Entity A spans two cells
[ ][A][B][B][ ][ ]   ← A and B share a cell → candidate pair
[ ][ ][ ][B][ ][ ]
[ ][ ][ ][ ][ ][ ]
```

Since an entity can overlap multiple cells, the same pair might be generated more than once. De-duplication uses an `unordered_set` with **canonical ID ordering** — the lower entity ID always goes first:

```cpp
if (e1.id > e2.id) std::swap(e1, e2);
m_uniquePotentialPairsSet.insert({ e1, e2 });
```

The grid also has a **live debug visualiser**: cells highlight yellow (one entity) or red (two or more) at runtime, toggled from the editor debug panel.

---

### Stage 2 — Narrow Phase

The narrow phase does exact AABB overlap tests on candidate pairs. The interesting part is **collision normal resolution** — choosing the right axis to push entities apart.

The naive approach (smallest overlap axis) breaks at corners. If an entity hits the corner of a wall, the minimum axis can flip between X and Y frame to frame, producing jittery normals.

IronHammer solves this by checking what was overlapping **on the previous frame**, stored in `CRigidBody::previousPosition`:

```cpp
bool lastXCollide = abs(lastFrameDistance.x) <= (e1Col->halfSize.x + e2Col->halfSize.x);
bool lastYCollide = abs(lastFrameDistance.y) <= (e1Col->halfSize.y + e2Col->halfSize.y);

if (lastXCollide && !lastYCollide)
{
    // Already overlapping on X → collision entered from Y direction
    normal = (distance.y < 0) ? Vect2f(0, -1) : Vect2f(0, 1);
    penetration = overlap.y;
}
else if (!lastXCollide && lastYCollide)
{
    // Already overlapping on Y → collision entered from X direction
    normal = (distance.x < 0) ? Vect2f(-1, 0) : Vect2f(1, 0);
    penetration = overlap.x;
}
else
{
    // Fresh contact — fall back to minimum penetration axis
    if (overlap.x < overlap.y) { normal = ...; penetration = overlap.x; }
    else                        { normal = ...; penetration = overlap.y; }
}
```

Stable, correct normals at corners — no special-case geometry needed.

---

### Stage 3 — Resolution

Applies the standard rigid body impulse formula. The response scales by each entity's **inverse mass** — static bodies have `inverseMass = 0`, so they absorb no impulse and never move:

```
impulse = -(1 + restitution) × relativeVelocity · normal
          ─────────────────────────────────────────────────
                  invMass₁ + invMass₂

velocity₁ -= impulse × invMass₁ × normal
velocity₂ += impulse × invMass₂ × normal
```

A positional correction pass follows to prevent slow sinking due to floating-point accumulation across frames.

---

## Rendering System

The render system uses **batched vertex arrays** to minimise GPU draw calls. All entities of the same type are accumulated into a single `sf::VertexArray` and flushed together.

For sprites, the batch flushes on **texture changes** — a new draw is only issued when the texture pointer changes:

```cpp
void RenderSprites(sf::RenderTarget& target)
{
    const sf::Texture* currentTexture = nullptr;

    for (auto& archetype : spriteQuery->GetMatchingArchetypes())
        for (auto& chunk : archetype->GetChunks())
        {
            auto sprites    = chunk.GetComponentRow<CSprite>();
            auto transforms = chunk.GetComponentRow<CTransform>();

            for (size_t i = 0; i < chunk.size; ++i)
            {
                if (currentTexture != sprites[i].texture)
                {
                    target.draw(batch, currentTexture); // flush
                    batch.clear();
                    currentTexture = sprites[i].texture;
                }
                AddSpriteToBatch(sprites[i], transforms[i], batch);
            }
        }

    target.draw(batch, currentTexture); // final flush
}
```

Sprite geometry is computed manually per-entity. Rotation is applied using a precomputed `cos`/`sin` pair:

```cpp
auto Rotate = [&](float x, float y) {
    return sf::Vector2f(
        center.x + x * cosRad - y * sinRad,
        center.y + x * sinRad + y * cosRad
    );
};
```

Polygon shapes use triangle-fan decomposition with a separate outer-ring pass for outlines. Collider wireframes use line primitives. All three have independent batch flush thresholds to avoid unbounded buffer growth.

The render system respects the `CNotDrawable` tag — any entity tagged with it is excluded from all render queries at zero runtime cost, since the exclusion is baked into the query mask at creation time.

---

## Debug Logger

Resolving a stack trace via `backward-cpp` requires symbol lookup, debug info parsing, and regex cleanup — expensive enough to cause a measurable frame stall if done synchronously.

IronHammer offloads the entire resolution process to a **dedicated background thread** using a three-queue architecture:

```
Game Thread                   Logger Thread              Main Thread
     │                              │                         │
LOG_INFO() ──[push]──► pending      │                         │
     │                 queue        │                         │
     │         [notify CV] ────────►│                         │
     │                              │ pop & resolve           │
     │                              │ (expensive)             │
     │                              │──[push]──► buffer       │
     │                              │           queue         │
     │                              │                   FlushLogs()
     │                              │               [move buffer → read]
     │                              │                         │
     │                              │                  ImGui reads
     │                              │                  (no lock held)
```

The logger thread blocks on a `condition_variable` when idle — zero CPU cost between frames. `ShutdownLoggerThread()` sets the running flag, notifies the CV, and **joins** — guaranteeing all in-flight logs are resolved before the process exits.

Display strings are pre-built at construction time so the ImGui log window never formats strings during rendering:

```cpp
// result string computed once at construction — never again
TraceBreakdown(const std::string& func, const std::string& file,
               unsigned int line, unsigned int col)
    : objectFunction(func), path(file), line(line), column(col)
    , result(objectFunction + " at " + path + " line " + std::to_string(line))
{}
```

Usage from anywhere in the codebase:

```cpp
LOG_INFO("Scene loaded");
LOG_WARNING("Entity has no Transform");
LOG_ERROR("Failed to open file: " + path);
```

---

## Editor

The editor is built entirely in **Dear ImGui** and laid out as five docked panels computed from window dimensions at startup.

All panels share a single `EditorContext&` reference — no globals, no singletons for editor state. Every panel declares exactly what it needs through the one parameter it receives.

```
┌──────────────────────────────────────────────────────┐
│                    Menu Bar                          │
├────────────┬──────────────────────────┬──────────────┤
│            │                          │              │
│   Debug    │        Viewport          │  Inspector   │
│   Panel    │                          │   Panel      │
│            │                          │              │
├────────────┴──────────────────────────┤              │
│         Log Window (Bottom)           │              │
└───────────────────────────────────────┴──────────────┘
```

---

### Viewport & Coordinate Systems

The game world renders into an `sf::RenderTexture` displayed as an `ImGui::Image`. Three coordinate systems coexist and need to be converted between correctly:

```
World Space          Texture Pixel Space      ImGui Screen Space
(game coords,   →   (Y-down, origin at    →  (Y-down, origin at
 Y-up origin)        render texture TL)        monitor TL)
```

`Viewport::WorldToViewportGui` handles the full chain. `mapCoordsToPixel` converts world → texture pixels (handling the Y-flip via the SFML view). The result is then scaled by `displaySize / textureSize` and offset by the image's actual screen position — captured via `ImGui::GetCursorScreenPos()` immediately before the `ImGui::Image()` call each frame:

```cpp
inline static ImVec2 WorldToViewportGui(const Vect2f& worldPos)
{
    sf::Vector2i texPixel = m_renderTexture->mapCoordsToPixel({ worldPos.x, worldPos.y });

    sf::Vector2u texSize = m_renderTexture->getSize();
    float scaleX = m_viewportImageDrawSize.x / (float)texSize.x;
    float scaleY = m_viewportImageDrawSize.y / (float)texSize.y;

    return ImVec2(
        m_viewportImagePos.x + (float)texPixel.x * scaleX,
        // Y-flip: texture Y goes down, but world Y goes up
        m_viewportImagePos.y + m_viewportImageDrawSize.y - (float)texPixel.y * scaleY
    );
}
```

---

### Transform Gizmo

Three invisible `ImageButton` widgets are positioned in screen space over the selected entity each frame:

| Handle | Axis |
|--------|------|
| Center square | XY — translate or uniform drag |
| Right arrow | X only |
| Up arrow | Y only |

**Translation** captures a `dragOffset` on first activation — the difference between entity world position and mouse world position at the moment of click. The offset is reapplied every frame the button is held, so the entity tracks the cursor regardless of where on the gizmo the grab happened.

**Scale** captures the mouse position and current scale at drag start, then accumulates a delta — no jump discontinuities when switching between fast and slow movement:

```cpp
// Every frame while held:
Vect2f delta = mouse - mouseDragOffset;
scale.x = std::max(scaleDragOffset.x + delta.x * 0.04f, 0.01f);
scale.y = std::max(scaleDragOffset.y + delta.y * 0.04f, 0.01f);
```

---

### Prefab System

Entities are authored as **JSON template files** stored in `src/assets/entityTemplates/`. A template captures the full component set of an entity:

```json
{
  "Transform": { "position": {"x": 0, "y": 0}, "scale": {"x": 1, "y": 1}, "rotation": 0 },
  "Sprite":    { "textureName": "Square", "size": [32, 32], "color": [55, 90, 161, 255] },
  "Collider":  { "size": {"x": 32, "y": 32}, "offset": {"x": 0, "y": 0}, "isTrigger": false },
  "RigidBody": { "mass": 10, "bounciness": 0.1, "isStatic": false, "velocity": {"x": 10, "y": 10} },
  "Enemy":     {}
}
```

Tag components (`Enemy`, `Tower`) serialize as empty objects — their presence in the JSON is enough to restore the entity's archetype signature on load.

Prefabs are dragged from the asset panel into the viewport to instantiate them. The drop handler converts mouse position to world space with optional grid snapping:

```cpp
if (m_editorContext.editorGrid.GetCanSnapToGrid())
{
    int cellSize = m_editorContext.editorGrid.GetCellSize();
    transform->position.x = std::round(mousePos.x / cellSize) * cellSize;
    transform->position.y = std::round(mousePos.y / cellSize) * cellSize;
}
```

`EntityTemplateInstance` defers add/remove operations to pending vectors flushed at the end of `DrawInspector()` — avoiding iterator invalidation while components are displayed — and re-serializes to disk only when `m_isDirty` is set.

---

### Scene Serialization

The engine maintains two worlds simultaneously:

| World | Purpose |
|-------|---------|
| `m_editorWorld` | Persistent authored scene — never touched during play |
| `m_tempWorld` | Created fresh from the scene JSON every time play is entered |

Entering play mode loads the scene file into `m_tempWorld`. Exiting is a **single pointer swap** back to `m_editorWorld`. No rollback, no snapshot, no undo stack — the editor state was never modified.

The `World` serializes through the type-erased `SerializeComponent` function pointer in `ComponentInfo` — no templates, no type knowledge required at the call site:

```cpp
archetype.ForEachComponent(location, [&](ComponentId id, void* ptr)
{
    ComponentRegistry::GetComponentInfoById(id)
        .SerializeComponent(entityJson, ptr);
});
```

---

## Project Structure

```
src/
├── app/                    ← IronHammer entry point and main loop
├── assets/                 ← Entity templates, scene data
├── core/
│   ├── memory/             ← TemplatedSlabAllocator, TypeErasedBlockAllocator, TypeErasedSlabAllocator
│   ├── saving/             ← JsonUtility
│   └── utils/              ← Debug logger, Time, Random, Vect2, Colors, CrashHandler
├── ecs/
│   ├── archetype/          ← Archetype, ArchetypeChunk, ArchetypeRegistry, ArchetypeDebugger
│   ├── common/             ← ECSCommon (Entity, ComponentId, EntityStorageLocation)
│   ├── component/          ← ComponentRegistry, Components
│   ├── entity/             ← CommandBuffer, EntityManager, EntityInspector, EntityTemplateInstance
│   ├── query/              ← Query
│   ├── system/             ← ISystem
│   └── World.hpp           ← World (entity creation, queries, serialization)
├── editor/
│   ├── panels/             ← ViewportPanel, InspectorPanel, DebugPanel, BottomPanel, MenuBarPanel
│   ├── Editor.h            ← Editor root
│   ├── EditorContext.h     ← Shared editor state
│   ├── EditorConfig.h      ← Layout computation
│   ├── EditorGrid.h        ← Grid snapping
│   ├── Viewport.h          ← Coordinate conversions
│   └── ViewportGizmo.h     ← Transform/scale gizmo
├── engine/                 ← Engine init, play/edit mode, frame loop
├── gui/                    ← LogWindow
├── input/                  ← InputManager, InputSystem
├── physics/                ← BroadPhase, NarrowPhase, CollisionResolution, MovementSystem
├── rendering/              ← RenderSystem
└── scene/                  ← SceneManager, BaseScene, GameScene
```
