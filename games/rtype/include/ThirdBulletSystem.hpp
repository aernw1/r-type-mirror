#pragma once

#include "ECS/ISystem.hpp"
#include "ECS/Registry.hpp"

namespace RType {
    namespace ECS {

        class ThirdBulletSystem : public ISystem {
        public:
            struct Direction {
                float vx;
                float vy;
            };

            ThirdBulletSystem() = default;
            ~ThirdBulletSystem() override = default;

            const char* GetName() const override { return "ThirdBulletSystem"; }

            void Update(Registry& registry, float deltaTime) override;

        private:
            void SpawnSmallProjectile(Registry& registry, float x, float y);
        };

    }
}
