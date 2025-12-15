/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ForcePodSystem
*/

#pragma once

#include "ISystem.hpp"

namespace RType {
    namespace ECS {

        class ForcePodSystem : public ISystem {
        public:
            ForcePodSystem() = default;
            ~ForcePodSystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "ForcePodSystem"; }
        };

    }
}
