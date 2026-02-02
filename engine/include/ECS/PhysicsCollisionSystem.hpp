/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** PhysicsCollisionSystem - Resolves collisions for physics-based entities (Rigidbodies)
*/

#pragma once

#include "ECS/ISystem.hpp"
#include "ECS/Components/Physics.hpp"
#include "ECS/Components/Transform.hpp"

namespace RType {
    namespace ECS {

        /**
         * @brief System responsible for resolving collisions between physical entities.
         * 
         * This system runs AFTER CollisionDetectionSystem. It iterates through
         * CollisionEvent components and resolves them for entities that have Rigidbody2D.
         * 
         * Resolution includes:
         * - Positional correction (separation) to prevent sinking
         * - Velocity impulse application (bounce/friction)
         */
        class PhysicsCollisionSystem : public ISystem {
        public:
            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "PhysicsCollisionSystem"; }

        private:
            void ResolveCollision(Registry& registry, Entity a, Entity b);
            
            // Helper to get overlap amount
            static Math::Vector2 GetCollisionNormalAndDepth(Registry& registry, Entity a, Entity b, float& depth);
        };

    }
}
