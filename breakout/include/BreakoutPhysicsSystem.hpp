#pragma once

#include "ECS/ISystem.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"

namespace Breakout {
    namespace ECS {

        class BreakoutPhysicsSystem : public RType::ECS::ISystem {
        public:
            BreakoutPhysicsSystem() = default;
            ~BreakoutPhysicsSystem() override = default;

            const char* GetName() const override { return "BreakoutPhysicsSystem"; }
            void Update(RType::ECS::Registry& registry, float deltaTime) override;

        private:
            void HandleBallPaddleCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, RType::ECS::Entity paddle);
            void HandleBallBrickCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, RType::ECS::Entity brick);
            void HandleBallWallCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, float screenWidth, float screenHeight);
        };

    }
}

