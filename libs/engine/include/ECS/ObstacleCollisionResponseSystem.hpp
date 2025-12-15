/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ObstacleCollisionResponseSystem - Handles obstacle collision responses
*/

#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"
#include "Component.hpp"

namespace RType {
    namespace ECS {

        class ObstacleCollisionResponseSystem : public ISystem {
        public:
            ObstacleCollisionResponseSystem() = default;
            ~ObstacleCollisionResponseSystem() override = default;

            const char* GetName() const override { return "ObstacleCollisionResponseSystem"; }
            void Update(Registry& registry, float deltaTime) override;
        };

    }
}
