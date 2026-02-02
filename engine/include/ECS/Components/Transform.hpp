#pragma once
/**
 * @file Transform.hpp
 * @brief Generic transform-related components for any 2D/3D game engine.
 * 
 * These components are game-agnostic and can be used by any project.
 */

#include <cstdint>
#include "Math/Types.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Components/IComponent.hpp"

namespace RType {
namespace ECS {

    /**
     * @brief 2D position component.
     */
    struct Position : public IComponent {
        float x = 0.0f;
        float y = 0.0f;

        Position() = default;
        Position(float x, float y) : x(x), y(y) {}
    };

    /**
     * @brief 2D velocity component.
     */
    struct Velocity : public IComponent {
        float dx = 0.0f;
        float dy = 0.0f;

        Velocity() = default;
        Velocity(float dx, float dy) : dx(dx), dy(dy) {}
    };

    /**
     * @brief Full 2D transform (position + rotation + scale).
     * Use this when you need full transform capabilities.
     */
    struct Transform2D : public IComponent {
        Math::Vector2 position{0.0f, 0.0f};
        float rotation = 0.0f;
        Math::Vector2 scale{1.0f, 1.0f};

        Transform2D() = default;
        Transform2D(float x, float y, float rot = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f)
            : position{x, y}, rotation(rot), scale{scaleX, scaleY} {}
    };

    /**
     * @brief Component for entities that can be controlled by input.
     */
    struct Controllable : public IComponent {
        float speed = 200.0f;

        Controllable() = default;
        Controllable(float moveSpeed) : speed(moveSpeed) {}
    };

    /**
     * @brief Indicates scrolling behavior for backgrounds/parallax.
     */
    struct Scrollable : public IComponent {
        float speed = -100.0f;

        Scrollable() = default;
        Scrollable(float scrollSpeed) : speed(scrollSpeed) {}
    };

    /**
     * @brief Stable network identifier for server->client entity mirroring.
     * 
     * IMPORTANT: This must live on the entity as a component, because raw ECS 
     * entity IDs are recycled. If we map "ECS entity id -> network id" in a 
     * hash map, then destroying and reusing an ECS id in the same tick can 
     * cause a new entity to inherit the old network id (type confusion).
     */
    struct NetworkId : public IComponent {
        uint32_t id = 0;

        NetworkId() = default;
        explicit NetworkId(uint32_t networkId) : id(networkId) {}
    };

} // namespace ECS
} // namespace RType
