#pragma once

#include "ISystem.hpp"
#include "ECS/CollisionSystem.hpp"

namespace RType {
    namespace ECS {

        class BulletCollisionSystem : public ISystem {
        public:
            BulletCollisionSystem() = default;
            ~BulletCollisionSystem() override = default;

            const char* GetName() const override { return "BulletCollisionSystem"; }

            void Update(Registry& registry, float deltaTime) override;
        };
    }
}