#pragma once
/**
 * @file Physics.hpp
 * @brief Generic physics and collision components for any 2D game engine.
 */

#include <cstdint>
#include "ECS/Entity.hpp"
#include "ECS/Components/IComponent.hpp"
#include "Math/Types.hpp"

namespace RType {
namespace ECS {

    /**
     * @brief Axis-aligned box collider.
     */
    struct BoxCollider : public IComponent {
        float width = 0.0f;
        float height = 0.0f;

        BoxCollider() = default;
        BoxCollider(float width, float height)
            : width(width), height(height) {}
    };

    /**
     * @brief Circle collider.
     */
    struct CircleCollider : public IComponent {
        float radius = 0.0f;

        CircleCollider() = default;
        CircleCollider(float r) : radius(r) {}
    };

    /**
     * @brief Collision layer for filtering collisions.
     */
    struct CollisionLayer : public IComponent {
        uint16_t layer = 0;     // What layer this entity is on
        uint16_t mask = 0xFFFF; // Which layers this entity collides with

        CollisionLayer() = default;
        CollisionLayer(uint16_t l, uint16_t m) : layer(l), mask(m) {}
    };

    /**
     * @brief Standard collision layer masks.
     */
    namespace CollisionLayers {
        constexpr uint16_t NONE = 0;
        constexpr uint16_t PLAYER = 1 << 0;        // 0x0001
        constexpr uint16_t ENEMY = 1 << 1;         // 0x0002
        constexpr uint16_t PLAYER_BULLET = 1 << 2; // 0x0004
        constexpr uint16_t ENEMY_BULLET = 1 << 3;  // 0x0008
        constexpr uint16_t OBSTACLE = 1 << 4;      // 0x0010
        constexpr uint16_t POWERUP = 1 << 5;       // 0x0020
        constexpr uint16_t GROUND = 1 << 6;        // 0x0040 (for platformers)
        constexpr uint16_t TRIGGER = 1 << 7;       // 0x0080 (for trigger zones)
        constexpr uint16_t ALL = 0xFFFF;
    }

    /**
     * @brief Collision event marker component.
     */
    struct CollisionEvent : public IComponent {
        Entity other = NULL_ENTITY;

        CollisionEvent() = default;
        CollisionEvent(Entity e) : other(e) {}
    };

    /**
     * @brief Static obstacle that blocks movement.
     */
    struct Obstacle : public IComponent {
        bool blocking = true;

        Obstacle() = default;
        Obstacle(bool isBlocking) : blocking(isBlocking) {}
    };

    /**
     * @brief Invincibility frames/state.
     */
    struct Invincibility : public IComponent {
        float remainingTime = 0.0f;

        Invincibility() = default;
        Invincibility(float duration) : remainingTime(duration) {}
    };

    /**
     * @brief 2D Rigidbody with physics properties (NEW for metroidvania).
     */
    struct Rigidbody2D : public IComponent {
        float mass = 1.0f;
        float gravityScale = 1.0f;
        float drag = 0.0f;
        bool isKinematic = false;
        bool isGrounded = false;
        bool useGravity = true;

        Rigidbody2D() = default;
        Rigidbody2D(float m, float gravity = 1.0f)
            : mass(m), gravityScale(gravity) {}
    };

    /**
     * @brief Physics material for collision response.
     */
    struct PhysicsMaterial : public IComponent {
        float friction = 0.5f;
        float bounciness = 0.0f;

        PhysicsMaterial() = default;
        PhysicsMaterial(float f, float b) : friction(f), bounciness(b) {}
    };

    /**
     * @brief Result of a raycast operation.
     */
    struct RaycastHit {
        Entity entity = NULL_ENTITY;
        Math::Vector2 point{0.0f, 0.0f};
        Math::Vector2 normal{0.0f, 0.0f};
        float distance = 0.0f;
    };

} // namespace ECS
} // namespace RType
