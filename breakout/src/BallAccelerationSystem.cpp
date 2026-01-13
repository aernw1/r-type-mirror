#include "BallAccelerationSystem.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"
#include <cmath>

namespace Breakout {
    namespace ECS {

        void BallAccelerationSystem::Update(RType::ECS::Registry& registry, float deltaTime) {
            m_elapsedTime += deltaTime;

            RType::ECS::Entity ballEntity = RType::ECS::NULL_ENTITY;
            auto entities = registry.GetEntitiesWithComponent<RType::ECS::CircleCollider>();
            for (auto entity : entities) {
                if (registry.HasComponent<RType::ECS::Velocity>(entity)) {
                    ballEntity = entity;
                    break;
                }
            }

            if (ballEntity == RType::ECS::NULL_ENTITY || !registry.HasComponent<RType::ECS::Velocity>(ballEntity)) {
                return;
            }

            auto& ballVel = registry.GetComponent<RType::ECS::Velocity>(ballEntity);

            float speedMultiplier = 1.0f + (m_elapsedTime * ACCELERATION_RATE / 50.0f);
            if (speedMultiplier > MAX_SPEED_MULTIPLIER) {
                speedMultiplier = MAX_SPEED_MULTIPLIER;
            }

            float currentSpeed = std::sqrt(ballVel.dx * ballVel.dx + ballVel.dy * ballVel.dy);
            float targetSpeed = 200.0f * speedMultiplier;

            if (currentSpeed < targetSpeed && currentSpeed > 0.0f) {
                float newSpeed = currentSpeed + (targetSpeed - currentSpeed) * deltaTime;
                float dirX = ballVel.dx / currentSpeed;
                float dirY = ballVel.dy / currentSpeed;
                ballVel.dx = dirX * newSpeed;
                ballVel.dy = dirY * newSpeed;
            }
        }

    }
}
