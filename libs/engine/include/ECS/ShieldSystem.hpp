/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ShieldSystem
*/

#pragma once

#include "ISystem.hpp"

namespace RType {
    namespace ECS {

        class ShieldSystem : public ISystem {
        public:
            ShieldSystem() = default;
            ~ShieldSystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "ShieldSystem"; }
        };

    }
}
