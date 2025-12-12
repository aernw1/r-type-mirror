/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BulletCollisionResponseSystem - Handles bullet collision responses
*/

#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"
#include "Component.hpp"

namespace RType {
    namespace ECS {

        class BulletCollisionResponseSystem : public ISystem {
        public:
            BulletCollisionResponseSystem() = default;
            ~BulletCollisionResponseSystem() override = default;

            const char* GetName() const override { return "BulletCollisionResponseSystem"; }
            void Update(Registry& registry, float deltaTime) override;
        };

    }
}
