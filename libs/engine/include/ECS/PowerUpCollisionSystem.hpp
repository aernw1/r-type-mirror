/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PowerUpCollisionSystem
*/

#pragma once

#include "ISystem.hpp"
#include "CollisionSystem.hpp"
#include "Renderer/IRenderer.hpp"

namespace RType {
    namespace ECS {

        class PowerUpCollisionSystem : public ISystem {
        public:
            explicit PowerUpCollisionSystem(Renderer::IRenderer* renderer = nullptr);
            ~PowerUpCollisionSystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "PowerUpCollisionSystem"; }

        private:
            Renderer::IRenderer* m_renderer;
        };

    }
}
