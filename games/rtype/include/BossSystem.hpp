/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BossSystem - Handles boss behavior
*/

#pragma once

#include "ECS/ISystem.hpp"
#include "ECS/Registry.hpp"

namespace RType {
    namespace ECS {

        class BossSystem : public ISystem {
        public:
            BossSystem() = default;
            ~BossSystem() override = default;

            const char* GetName() const override { return "BossSystem"; }

            void Update(Registry& registry, float deltaTime) override;
        };

    }
}
