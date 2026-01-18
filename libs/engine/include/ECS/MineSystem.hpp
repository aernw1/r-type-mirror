/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MineSystem - Handles mine proximity detection and explosions
*/

#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"
#include <cmath>

namespace RType {
    namespace ECS {

        class MineSystem : public ISystem {
        public:
            MineSystem() = default;
            ~MineSystem() override = default;

            const char* GetName() const override { return "MineSystem"; }

            void Update(Registry& registry, float deltaTime) override;

        private:
            float Distance(float x1, float y1, float x2, float y2);
        };

    }
}
