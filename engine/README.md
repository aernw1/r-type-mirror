# R-Type Engine & ECS - Developer Documentation

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Core Engine](#core-engine)
- [Module System](#module-system)
- [ECS Fundamentals](#ecs-fundamentals)
- [Registry API](#registry-api)
- [Components](#components)
- [Systems](#systems)
- [SparseArray](#sparsearray)
- [Quick Start Guide](#quick-start-guide)
- [Best Practices](#best-practices)

---

## Overview

The R-Type Engine is a modular, data-oriented game engine built with C++17. It features:

- **Plugin-based architecture**: Load modules dynamically at runtime
- **Entity Component System (ECS)**: Data-oriented design for efficient game object management
- **Cross-platform**: Works on Linux, macOS, and Windows
- **Type-safe**: Template-based component and system registration

### Project Structure

```
libs/engine/
├── include/
│   ├── Core/
│   │   ├── Engine.hpp        # Main engine coordinator
│   │   ├── Module.hpp        # IModule interface
│   │   ├── ModuleLoader.hpp  # Dynamic plugin loader
│   │   └── Logger.hpp        # Logging utility
│   └── ECS/
│       ├── Entity.hpp        # Entity type (uint32_t)
│       ├── Component.hpp     # All component definitions
│       ├── SparseArray.hpp   # Component storage container
│       ├── Registry.hpp      # ECS registry
│       ├── ISystem.hpp       # System interface
│       └── *System.hpp       # Game logic systems
└── src/
    ├── Core/
    └── ECS/
```

---

## Architecture

The engine follows a layered architecture:

```
┌─────────────────────────────────────┐
│      GAME APPLICATION               │
│   (Uses engine as static library)   │
└─────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────┐
│         ENGINE CORE                 │
│  ┌──────────┐  ┌─────────────────┐ │
│  │  Engine  │  │     Registry    │ │
│  │          │  │   (ECS Manager) │ │
│  └──────────┘  └─────────────────┘ │
│  ┌──────────┐  ┌─────────────────┐ │
│  │ Module   │  │     Systems     │ │
│  │ Loader   │  │   (Game Logic)  │ │
│  └──────────┘  └─────────────────┘ │
└─────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────┐
│      MODULES / PLUGINS              │
│  ┌──────────┐ ┌─────────┐ ┌──────┐ │
│  │ Renderer │ │ Physics │ │Audio │ │
│  └──────────┘ └─────────┘ └──────┘ │
└─────────────────────────────────────┘
```

---

## Core Engine

The `Engine` class is the central coordinator that manages modules, plugins, and the ECS registry.

### Basic Usage

```cpp
#include <Core/Engine.hpp>
#include <ECS/Registry.hpp>

int main() {
    // Create engine with default config
    RType::Core::EngineConfig config;
    config.pluginPath = "./plugins";
    
    auto engine = std::make_unique<RType::Core::Engine>(config);
    
    // Load plugins (optional)
    engine->LoadPlugin("plugins/libSFMLRenderer.so");
    
    // Register built-in modules
    engine->RegisterModule(std::make_unique<MyModule>());
    
    // Initialize all modules (in priority order)
    if (!engine->Initialize()) {
        RType::Core::Logger::Error("Engine initialization failed");
        return 1;
    }
    
    // Access the ECS registry
    auto& registry = engine->GetRegistry();
    
    // Register systems
    engine->RegisterSystem(std::make_unique<RType::ECS::MovementSystem>());
    
    // Game loop
    while (running) {
        float deltaTime = getDeltaTime();
        engine->UpdateSystems(deltaTime);
        // ... render, etc.
    }
    
    // Shutdown (modules shutdown in reverse priority order)
    engine->Shutdown();
    return 0;
}
```

### Engine API

| Method | Description |
|--------|-------------|
| `Initialize()` | Initialize all registered modules (returns false on failure) |
| `Shutdown()` | Shutdown all modules in reverse priority order |
| `LoadPlugin(path)` | Load a plugin from shared library (.so/.dll/.dylib) |
| `UnloadPlugin(name)` | Unload a plugin by name |
| `RegisterModule<T>(module)` | Register a built-in module |
| `GetModule<T>()` | Get module by type (searches built-in and plugins) |
| `GetModuleByName(name)` | Get module by name string |
| `GetRegistry()` | Access the ECS registry |
| `RegisterSystem<T>(system)` | Register an ECS system |
| `UpdateSystems(deltaTime)` | Update all registered systems |

---

## Module System

Modules are the building blocks of the engine. They can be built-in (compiled with engine) or plugins (loaded dynamically).

### IModule Interface

All modules must implement `IModule`:

```cpp
class IModule {
public:
    virtual ~IModule() = default;
    virtual const char* GetName() const = 0;
    virtual ModulePriority GetPriority() const = 0;
    virtual bool Initialize(Engine* engine) = 0;
    virtual void Shutdown() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual bool ShouldUpdateInRenderThread() const { return false; }
    virtual bool IsOverridable() const { return true; }
};
```

### Module Priority

Modules initialize in priority order (lowest value first):

| Priority | Value | Use Case |
|----------|-------|----------|
| `Critical` | 0 | Core systems that others depend on |
| `High` | 1 | Rendering, Physics |
| `Normal` | 2 | Audio, Input |
| `Low` | 3 | Non-essential features |

**Shutdown occurs in reverse priority order.**

### Creating a Module

```cpp
// MyModule.hpp
#pragma once
#include <Core/Module.hpp>
#include <Core/Engine.hpp>

class MyModule : public RType::Core::IModule {
public:
    const char* GetName() const override { return "MyModule"; }
    
    RType::Core::ModulePriority GetPriority() const override {
        return RType::Core::ModulePriority::Normal;
    }
    
    bool Initialize(RType::Core::Engine* engine) override {
        m_engine = engine;
        RType::Core::Logger::Info("MyModule initialized");
        return true;
    }
    
    void Shutdown() override {
        RType::Core::Logger::Info("MyModule shutdown");
    }
    
    void Update(float deltaTime) override {
        // Per-frame update
    }
    
private:
    RType::Core::Engine* m_engine = nullptr;
};
```

### Plugin Export Functions

Plugins must export these C functions:

```cpp
extern "C" {
    RTYPE_MODULE_EXPORT RType::Core::IModule* CreateModule() {
        return new MyModule();
    }
    
    RTYPE_MODULE_EXPORT void DestroyModule(RType::Core::IModule* module) {
        delete module;
    }
}
```

The `extern "C"` prevents C++ name mangling, allowing `dlsym()`/`GetProcAddress()` to find the functions.

---

## ECS Fundamentals

The Entity Component System provides a data-oriented approach to game object management.

### Core Concepts

- **Entity**: A unique identifier (`uint32_t`). Represents a game object.
- **Component**: Plain data struct attached to entities. Contains no logic.
- **System**: Logic that processes entities with specific component combinations.
- **Registry**: Central manager that stores entities and their components.

### Entity

```cpp
using Entity = uint32_t;
constexpr Entity NULL_ENTITY = 0;
```

Entities are just IDs. They have no data or behavior themselves.

---

## Registry API

The `Registry` manages all entities and components.

### Entity Management

```cpp
// Create entity
Entity player = registry.CreateEntity();

// Check if entity exists
if (registry.IsEntityAlive(player)) {
    // Entity is valid
}

// Destroy entity (removes all components)
registry.DestroyEntity(player);

// Get total entity count
size_t count = registry.GetEntityCount();
```

### Component Management

```cpp
// Add component (by copy)
registry.AddComponent(player, RType::ECS::Position{100.0f, 200.0f});

// Add component (by move)
auto velocity = RType::ECS::Velocity{5.0f, 0.0f};
registry.AddComponent(player, std::move(velocity));

// Add component (default constructed)
registry.AddComponent<RType::ECS::Health>(player);

// Get component (non-const)
auto& position = registry.GetComponent<RType::ECS::Position>(player);
position.x = 150.0f;

// Get component (const)
const auto& health = registry.GetComponent<RType::ECS::Health>(player);

// Check if entity has component
if (registry.HasComponent<RType::ECS::Position>(player)) {
    // Safe to access
}

// Remove component
registry.RemoveComponent<RType::ECS::Velocity>(player);

// Get all entities with a component
auto entities = registry.GetEntitiesWithComponent<RType::ECS::Position>();
for (Entity e : entities) {
    // Process entities with Position
}
```

### Error Handling

- `GetComponent<T>(entity)` throws `std::runtime_error` if component doesn't exist
- `AddComponent<T>(entity)` throws if entity is `NULL_ENTITY` or doesn't exist
- Always check with `HasComponent<T>()` before accessing

**Best Practice:**
```cpp
if (registry.HasComponent<Position>(entity)) {
    auto& pos = registry.GetComponent<Position>(entity);
    // Safe to use
}
```

---

## Components

Components are plain data structs that inherit from `IComponent`. They contain **no logic**.

### Built-in Components

The engine provides many built-in components:

```cpp
// Transform
struct Position { float x, y; };
struct Velocity { float dx, dy; };

// Rendering
struct Drawable {
    Renderer::SpriteId spriteId;
    Math::Vector2 scale{1.0f, 1.0f};
    float rotation = 0.0f;
    int layer = 0;
};

// Gameplay
struct Health { int current, max; };
struct Player { uint8_t playerNumber; uint64_t playerHash; };
struct Enemy { EnemyType type; uint32_t id; };
struct Bullet { Entity owner; };
struct PowerUp { PowerUpType type; uint32_t id; };

// Physics
struct BoxCollider { float width, height; };
struct CircleCollider { float radius; };
struct CollisionLayer { uint16_t layer; uint16_t mask; };

// And many more...
```

### Creating Custom Components

```cpp
// In Component.hpp or your own header
struct MyCustomComponent : public RType::ECS::IComponent {
    int value = 0;
    std::string name;
    
    MyCustomComponent() = default;
    MyCustomComponent(int v, const std::string& n) 
        : value(v), name(n) {}
};

// Usage
registry.AddComponent(entity, MyCustomComponent{42, "test"});
```

### Component Guidelines

- ✅ Use plain structs with public members
- ✅ Provide default constructor
- ✅ Keep components small and focused
- ✅ No virtual functions (unless inheriting from IComponent)
- ❌ Don't put logic in components
- ❌ Don't store pointers to other entities (use `Entity` IDs instead)

---

## Systems

Systems contain the game logic. They process entities with specific component combinations.

### ISystem Interface

```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void Update(Registry& registry, float deltaTime) = 0;
    virtual const char* GetName() const = 0;
    virtual bool Initialize(Registry& registry) { return true; }
    virtual void Shutdown() {}
};
```

### Creating a System

```cpp
// MovementSystem.hpp
#pragma once
#include "ECS/ISystem.hpp"

namespace RType::ECS {
    class MovementSystem : public ISystem {
    public:
        void Update(Registry& registry, float deltaTime) override;
        const char* GetName() const override { return "MovementSystem"; }
    };
}

// MovementSystem.cpp
#include "ECS/MovementSystem.hpp"
#include "ECS/Component.hpp"

void MovementSystem::Update(Registry& registry, float deltaTime) {
    // Get all entities with Velocity
    auto entities = registry.GetEntitiesWithComponent<Velocity>();
    
    for (Entity entity : entities) {
        // Check if entity also has Position
        if (!registry.HasComponent<Position>(entity)) {
            continue;
        }
        
        // Update position based on velocity
        auto& position = registry.GetComponent<Position>(entity);
        const auto& velocity = registry.GetComponent<Velocity>(entity);
        
        position.x += velocity.dx * deltaTime;
        position.y += velocity.dy * deltaTime;
    }
}
```

### Registering Systems

```cpp
// In your game initialization
auto engine = std::make_unique<RType::Core::Engine>();

engine->RegisterSystem(std::make_unique<RType::ECS::MovementSystem>());
engine->RegisterSystem(std::make_unique<RType::ECS::RenderingSystem>());
engine->RegisterSystem(std::make_unique<RType::ECS::CollisionSystem>());

engine->Initialize();

// In game loop
while (running) {
    float deltaTime = getDeltaTime();
    engine->UpdateSystems(deltaTime);  // Updates all systems
}
```

### System Patterns

**Query Multiple Components:**
```cpp
void MySystem::Update(Registry& registry, float deltaTime) {
    // Get entities with first component
    auto entities = registry.GetEntitiesWithComponent<ComponentA>();
    
    for (Entity e : entities) {
        // Check for additional components
        if (registry.HasComponent<ComponentB>(e) && 
            registry.HasComponent<ComponentC>(e)) {
            
            auto& a = registry.GetComponent<ComponentA>(e);
            auto& b = registry.GetComponent<ComponentB>(e);
            auto& c = registry.GetComponent<ComponentC>(e);
            
            // Process...
        }
    }
}
```

**Create/Modify Entities:**
```cpp
void SpawnSystem::Update(Registry& registry, float deltaTime) {
    if (shouldSpawn) {
        Entity enemy = registry.CreateEntity();
        registry.AddComponent(enemy, Position{100.0f, 50.0f});
        registry.AddComponent(enemy, Velocity{0.0f, 50.0f});
        registry.AddComponent(enemy, Enemy{EnemyType::BASIC, nextId++});
    }
}
```

---

## SparseArray

`SparseArray` is the internal container used by the Registry to store components efficiently.

### Concept

A sparse array is a vector where the **index directly corresponds to an Entity ID**. This provides:
- **O(1)** direct access by entity ID
- **Cache-friendly** sequential iteration
- **Holes allowed** (indices can be empty via `std::nullopt`)

### Structure

```cpp
template <typename Component>
class SparseArray {
    std::vector<std::optional<Component>> _data;
    // Index = Entity ID
};
```

### Visual Example

```
Entities: 0, 2, 5 have Position components

Index:  0    1    2    3    4    5
       ┌────┬────┬────┬────┬────┬────┐
Data:  │{x,y}│null│{x,y}│null│null│{x,y}│
       └────┴────┴────┴────┴────┴────┘
```

### Usage

```cpp
SparseArray<Position> positions;

// Insert component at entity ID 5
positions.insert_at(5, Position{10.0f, 20.0f});

// Access directly
auto& pos_opt = positions[5];
if (pos_opt.has_value()) {
    Position& pos = pos_opt.value();
    pos.x = 100.0f;
}

// Emplace (constructs in-place, more efficient)
positions.emplace_at(7, 30.0f, 40.0f);

// Erase
positions.erase(5);

// Iterate
for (size_t i = 0; i < positions.size(); ++i) {
    if (positions[i].has_value()) {
        Entity e = static_cast<Entity>(i);
        Position& pos = positions[i].value();
        // Process...
    }
}
```

---

## Quick Start Guide

### 1. Create an Entity with Components

```cpp
#include <Core/Engine.hpp>
#include <ECS/Component.hpp>

auto& registry = engine->GetRegistry();

// Create player entity
Entity player = registry.CreateEntity();

// Add components
registry.AddComponent(player, RType::ECS::Position{400.0f, 300.0f});
registry.AddComponent(player, RType::ECS::Velocity{0.0f, 0.0f});
registry.AddComponent(player, RType::ECS::Health{100, 100});
registry.AddComponent(player, RType::ECS::Player{1, 0x1234, true});
```

### 2. Create a System

```cpp
// MySystem.hpp
class MySystem : public RType::ECS::ISystem {
public:
    void Update(RType::ECS::Registry& registry, float deltaTime) override {
        auto entities = registry.GetEntitiesWithComponent<RType::ECS::Position>();
        for (Entity e : entities) {
            auto& pos = registry.GetComponent<RType::ECS::Position>(e);
            // Update logic...
        }
    }
    const char* GetName() const override { return "MySystem"; }
};
```

### 3. Register and Run

```cpp
engine->RegisterSystem(std::make_unique<MySystem>());
engine->Initialize();

// Game loop
while (running) {
    engine->UpdateSystems(deltaTime);
}
```

### 4. Complete Example

```cpp
#include <Core/Engine.hpp>
#include <ECS/MovementSystem.hpp>
#include <ECS/Component.hpp>

int main() {
    auto engine = std::make_unique<RType::Core::Engine>();
    
    // Register systems
    engine->RegisterSystem(std::make_unique<RType::ECS::MovementSystem>());
    
    // Initialize
    engine->Initialize();
    auto& registry = engine->GetRegistry();
    
    // Create entities
    Entity player = registry.CreateEntity();
    registry.AddComponent(player, RType::ECS::Position{100.0f, 200.0f});
    registry.AddComponent(player, RType::ECS::Velocity{50.0f, 0.0f});
    
    // Game loop
    float deltaTime = 0.016f; // ~60 FPS
    for (int i = 0; i < 100; ++i) {
        engine->UpdateSystems(deltaTime);
        
        // Check updated position
        const auto& pos = registry.GetComponent<RType::ECS::Position>(player);
        RType::Core::Logger::Info("Player position: ({}, {})", pos.x, pos.y);
    }
    
    engine->Shutdown();
    return 0;
}
```

---

## Best Practices

### Component Design

1. **Keep components small and focused**
   ```cpp
   // ✅ Good: Single responsibility
   struct Position { float x, y; };
   struct Velocity { float dx, dy; };
   
   // ❌ Bad: Too much data in one component
   struct Transform { float x, y, dx, dy, rotation, scale; };
   ```

2. **Use Entity IDs, not pointers**
   ```cpp
   // ✅ Good
   struct Bullet { Entity owner; };
   
   // ❌ Bad
   struct Bullet { Entity* owner; };
   ```

3. **Provide sensible defaults**
   ```cpp
   struct Health {
       int current = 100;
       int max = 100;
   };
   ```

### System Design

1. **Process entities efficiently**
   ```cpp
   // ✅ Good: Get entities with most selective component first
   auto entities = registry.GetEntitiesWithComponent<RareComponent>();
   for (Entity e : entities) {
       if (registry.HasComponent<CommonComponent>(e)) {
           // Process...
       }
   }
   ```

2. **Don't modify component pools during iteration**
   ```cpp
   // ❌ Bad: Modifying while iterating
   for (Entity e : entities) {
       registry.DestroyEntity(e);  // Can cause issues
   }
   
   // ✅ Good: Collect first, then modify
   std::vector<Entity> toDestroy;
   for (Entity e : entities) {
       if (shouldDestroy) {
           toDestroy.push_back(e);
       }
   }
   for (Entity e : toDestroy) {
       registry.DestroyEntity(e);
   }
   ```

3. **Use const references when possible**
   ```cpp
   // ✅ Good
   const auto& velocity = registry.GetComponent<Velocity>(entity);
   
   // Only use non-const when modifying
   auto& position = registry.GetComponent<Position>(entity);
   ```

### Performance Tips

1. **Batch component access**: Get all entities once, then iterate
2. **Cache component references**: Don't call `GetComponent()` multiple times per entity
3. **Use `HasComponent()` before `GetComponent()`** to avoid exceptions
4. **Consider component layout**: Group frequently accessed components together

### Error Handling

```cpp
// Always check before accessing
if (registry.HasComponent<Health>(entity)) {
    auto& health = registry.GetComponent<Health>(entity);
    health.current -= damage;
}

// Or use try-catch for critical paths
try {
    auto& pos = registry.GetComponent<Position>(entity);
    // Use pos...
} catch (const std::runtime_error& e) {
    Logger::Error("Failed to get Position: {}", e.what());
}
```

---

## API Reference Summary

### RType::Core::Engine

- `Initialize()` → `bool`
- `Shutdown()` → `void`
- `LoadPlugin(path)` → `IModule*`
- `RegisterModule<T>(module)` → `void`
- `GetModule<T>()` → `T*`
- `GetRegistry()` → `Registry&`
- `RegisterSystem<T>(system)` → `void`
- `UpdateSystems(deltaTime)` → `void`

### RType::ECS::Registry

- `CreateEntity()` → `Entity`
- `DestroyEntity(entity)` → `void`
- `IsEntityAlive(entity)` → `bool`
- `AddComponent<T>(entity, component)` → `T&`
- `GetComponent<T>(entity)` → `T&` / `const T&`
- `HasComponent<T>(entity)` → `bool`
- `RemoveComponent<T>(entity)` → `void`
- `GetEntitiesWithComponent<T>()` → `std::vector<Entity>`
- `GetEntityCount()` → `size_t`

### RType::Core::Logger

- `Logger::Debug(format, ...)`
- `Logger::Info(format, ...)`
- `Logger::Warning(format, ...)`
- `Logger::Error(format, ...)`
- `Logger::Critical(format, ...)`

---

## Additional Resources

- Check `CODING_STYLE.md` for code style guidelines
- Review existing systems in `libs/engine/include/ECS/*System.hpp` for examples
- Examine component definitions in `libs/engine/include/ECS/Component.hpp`

---