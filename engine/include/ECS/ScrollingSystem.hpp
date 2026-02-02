/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScrollingSystem
*/

#pragma once
#include "ISystem.hpp"

namespace RType {
    namespace ECS {
        class ScrollingSystem : public ISystem {
        public:
            ScrollingSystem() = default;
            const char* GetName() const override { return "ScrollingSystem"; }
            void Update(Registry& registry, float deltaTime) override;
        };
    }
}
