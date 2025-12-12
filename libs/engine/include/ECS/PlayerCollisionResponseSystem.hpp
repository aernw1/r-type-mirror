/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PlayerCollisionResponseSystem - Handles player collision responses
*/

#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"
#include "Component.hpp"

namespace RType {
    namespace ECS {

        /**
         * @brief Handles responses to player collisions
         *
         * Processes CollisionEvent components on players and:
         * - Applies damage from enemy collisions
         * - Destroys enemies on collision
         * - Handles player-obstacle collisions (optional physics)
         * - Applies powerup effects (future enhancement)
         *
         * This system should run after CollisionDetectionSystem
         */
        class PlayerCollisionResponseSystem : public ISystem {
        public:
            PlayerCollisionResponseSystem() = default;
            ~PlayerCollisionResponseSystem() override = default;

            const char* GetName() const override { return "PlayerCollisionResponseSystem"; }
            void Update(Registry& registry, float deltaTime) override;
        };

    } // namespace ECS
} // namespace RType
