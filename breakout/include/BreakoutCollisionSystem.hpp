#pragma once

#include "ECS/ISystem.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"

namespace Breakout {
    namespace ECS {

        class BreakoutCollisionSystem : public RType::ECS::ISystem {
        public:
            BreakoutCollisionSystem() = default;
            ~BreakoutCollisionSystem() override = default;

            const char* GetName() const override { return "BreakoutCollisionSystem"; }
            void Update(RType::ECS::Registry& registry, float deltaTime) override;

        private:
            void HandleBallPaddleCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, RType::ECS::Entity paddle);
            void HandleBallBrickCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, RType::ECS::Entity brick);
            void HandleBallWallCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, float screenWidth, float screenHeight);
            bool CheckBallPaddleCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, RType::ECS::Entity paddle);
            bool CheckBallBrickCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, RType::ECS::Entity brick);
        };

    }
}
