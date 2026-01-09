/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BlackOrbSystem - Handles black orb attraction and proximity damage
*/

#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"
#include <cmath>

namespace RType {
    namespace ECS {

        class BlackOrbSystem : public ISystem {
        public:
            BlackOrbSystem() = default;
            ~BlackOrbSystem() override = default;

            const char* GetName() const override { return "BlackOrbSystem"; }

            void Update(Registry& registry, float deltaTime) override;
        };

    }
}
