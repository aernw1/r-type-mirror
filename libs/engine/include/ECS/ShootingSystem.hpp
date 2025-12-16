#pragma once

#include "ISystem.hpp"
#include "../Renderer/IRenderer.hpp"

namespace RType {
    namespace ECS {

        class ShootingSystem : public ISystem {
        public:
            ShootingSystem(Renderer::SpriteId bulletSprite);
            ~ShootingSystem() override = default;

            const char* GetName() const override { return "ShootingSystem"; }

            void Update(Registry& registry, float deltaTime) override;
        private:
            void CreateSpreadShot(Registry& registry, Entity shooter, const Position& pos, int damage);
            void CreateLaserShot(Registry& registry, Entity shooter, const Position& pos, int damage);

            Renderer::SpriteId m_bulletSprite;
        };
    }
}