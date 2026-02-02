/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** PhysicsSystem - Handles gravity, drag, and velocity integration
*/

#pragma once

#include "ECS/ISystem.hpp"
#include "Math/Types.hpp"

namespace RType {
    namespace ECS {

        /**
         * @brief System responsible for applying physics forces and integrating movement.
         * 
         * This system handles:
         * - Gravity application
         * - Linear drag (air resistance)
         * - Velocity integration
         * 
         * It does NOT handle collision detection or resolution (see CollisionDetectionSystem and PhysicsCollisionSystem).
         */
        class PhysicsSystem : public ISystem {
        public:
            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "PhysicsSystem"; }

            /**
             * @brief Set global gravity vector
             * @param gravity New gravity vector (default: 0, 980 for 2D platformer feels)
             */
            void SetGravity(float x, float y) { m_gravity = {x, y}; }

        private:
            Math::Vector2 m_gravity{0.0f, 980.0f}; // Default gravity (pixels/s^2)
        };

    }
}
