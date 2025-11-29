#pragma once

#include "ISystem.hpp"
#include "../Renderer/IRenderer.hpp"

namespace RType {
    namespace ECS {

        class ShootingSystem : public ISystem {
        public:
            ShootingSystem() = default;
            ShootingSystem(Renderer::IRenderer* renderer, Renderer::SpriteId bulletSprite);
            ~ShootingSystem() override = default;

            const char* GetName() const override { return "ShootingSystem"; }

            void Update(Registry& registry, float deltaTime) override;
        private:
            Renderer::IRenderer* m_renderer;
            Renderer::SpriteId m_bulletSprite;
        };
    }
}