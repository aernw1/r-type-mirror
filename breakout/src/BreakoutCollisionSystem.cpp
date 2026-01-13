#include "BreakoutCollisionSystem.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"
#include <cmath>
#include <random>

namespace Breakout {
    namespace ECS {

        void BreakoutCollisionSystem::Update(RType::ECS::Registry& registry, float /*deltaTime*/) {
            constexpr float SCREEN_WIDTH = 800.0f;
            constexpr float SCREEN_HEIGHT = 600.0f;

            RType::ECS::Entity ballEntity = RType::ECS::NULL_ENTITY;
            auto entities = registry.GetEntitiesWithComponent<RType::ECS::CircleCollider>();
            for (auto entity : entities) {
                if (registry.HasComponent<RType::ECS::Velocity>(entity)) {
                    ballEntity = entity;
                    break;
                }
            }

            if (ballEntity == RType::ECS::NULL_ENTITY) {
                return;
            }

            RType::ECS::Entity paddleEntity = RType::ECS::NULL_ENTITY;
            auto controllableEntities = registry.GetEntitiesWithComponent<RType::ECS::Controllable>();
            for (auto entity : controllableEntities) {
                paddleEntity = entity;
                break;
            }

            if (paddleEntity != RType::ECS::NULL_ENTITY && CheckBallPaddleCollision(registry, ballEntity, paddleEntity)) {
                HandleBallPaddleCollision(registry, ballEntity, paddleEntity);
            }

            auto brickEntities = registry.GetEntitiesWithComponent<RType::ECS::Health>();
            for (auto brick : brickEntities) {
                if (registry.HasComponent<RType::ECS::Controllable>(brick)) {
                    continue;
                }
                if (CheckBallBrickCollision(registry, ballEntity, brick)) {
                    HandleBallBrickCollision(registry, ballEntity, brick);
                }
            }

            HandleBallWallCollision(registry, ballEntity, SCREEN_WIDTH, SCREEN_HEIGHT);
        }

        bool BreakoutCollisionSystem::CheckBallPaddleCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, RType::ECS::Entity paddle) {
            if (!registry.HasComponent<RType::ECS::Position>(ball) || !registry.HasComponent<RType::ECS::Position>(paddle) ||
                !registry.HasComponent<RType::ECS::CircleCollider>(ball) || !registry.HasComponent<RType::ECS::BoxCollider>(paddle)) {
                return false;
            }

            const auto& ballPos = registry.GetComponent<RType::ECS::Position>(ball);
            const auto& paddlePos = registry.GetComponent<RType::ECS::Position>(paddle);
            const auto& ballCollider = registry.GetComponent<RType::ECS::CircleCollider>(ball);
            const auto& paddleCollider = registry.GetComponent<RType::ECS::BoxCollider>(paddle);

            float ballRadius = ballCollider.radius;
            float paddleLeft = paddlePos.x - paddleCollider.width / 2.0f;
            float paddleRight = paddlePos.x + paddleCollider.width / 2.0f;
            float paddleTop = paddlePos.y - paddleCollider.height / 2.0f;
            float paddleBottom = paddlePos.y + paddleCollider.height / 2.0f;

            float closestX = std::max(paddleLeft, std::min(ballPos.x, paddleRight));
            float closestY = std::max(paddleTop, std::min(ballPos.y, paddleBottom));

            float dx = ballPos.x - closestX;
            float dy = ballPos.y - closestY;
            return (dx * dx + dy * dy) < (ballRadius * ballRadius);
        }

        bool BreakoutCollisionSystem::CheckBallBrickCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, RType::ECS::Entity brick) {
            if (!registry.HasComponent<RType::ECS::Position>(ball) || !registry.HasComponent<RType::ECS::Position>(brick) ||
                !registry.HasComponent<RType::ECS::CircleCollider>(ball) || !registry.HasComponent<RType::ECS::BoxCollider>(brick)) {
                return false;
            }

            const auto& ballPos = registry.GetComponent<RType::ECS::Position>(ball);
            const auto& brickPos = registry.GetComponent<RType::ECS::Position>(brick);
            const auto& ballCollider = registry.GetComponent<RType::ECS::CircleCollider>(ball);
            const auto& brickCollider = registry.GetComponent<RType::ECS::BoxCollider>(brick);

            float ballRadius = ballCollider.radius;
            float brickLeft = brickPos.x - brickCollider.width / 2.0f;
            float brickRight = brickPos.x + brickCollider.width / 2.0f;
            float brickTop = brickPos.y - brickCollider.height / 2.0f;
            float brickBottom = brickPos.y + brickCollider.height / 2.0f;

            float closestX = std::max(brickLeft, std::min(ballPos.x, brickRight));
            float closestY = std::max(brickTop, std::min(ballPos.y, brickBottom));

            float dx = ballPos.x - closestX;
            float dy = ballPos.y - closestY;
            return (dx * dx + dy * dy) < (ballRadius * ballRadius);
        }

        void BreakoutCollisionSystem::HandleBallPaddleCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, RType::ECS::Entity paddle) {
            if (!registry.HasComponent<RType::ECS::Position>(ball) || !registry.HasComponent<RType::ECS::Position>(paddle) ||
                !registry.HasComponent<RType::ECS::Velocity>(ball) || !registry.HasComponent<RType::ECS::BoxCollider>(paddle)) {
                return;
            }

            const auto& ballPos = registry.GetComponent<RType::ECS::Position>(ball);
            const auto& paddlePos = registry.GetComponent<RType::ECS::Position>(paddle);
            const auto& paddleCollider = registry.GetComponent<RType::ECS::BoxCollider>(paddle);
            auto& ballVel = registry.GetComponent<RType::ECS::Velocity>(ball);

            float relativeX = (ballPos.x - paddlePos.x) / (paddleCollider.width / 2.0f);
            relativeX = std::max(-1.0f, std::min(1.0f, relativeX));

            float angle = relativeX * 60.0f * (3.14159f / 180.0f);
            float speed = std::sqrt(ballVel.dx * ballVel.dx + ballVel.dy * ballVel.dy);
            if (speed < 200.0f) {
                speed = 300.0f;
            }

            ballVel.dx = speed * std::sin(angle);
            ballVel.dy = -std::abs(speed * std::cos(angle));

            if (registry.HasComponent<RType::ECS::CircleCollider>(ball)) {
                float ballRadius = registry.GetComponent<RType::ECS::CircleCollider>(ball).radius;
                auto& ballPosRef = registry.GetComponent<RType::ECS::Position>(ball);
                ballPosRef.y = paddlePos.y - paddleCollider.height / 2.0f - ballRadius - 2.0f;
            }
        }

        void BreakoutCollisionSystem::HandleBallBrickCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, RType::ECS::Entity brick) {
            if (!registry.HasComponent<RType::ECS::Position>(ball) || !registry.HasComponent<RType::ECS::Position>(brick) ||
                !registry.HasComponent<RType::ECS::Velocity>(ball) || !registry.HasComponent<RType::ECS::Health>(brick)) {
                return;
            }

            const auto& ballPos = registry.GetComponent<RType::ECS::Position>(ball);
            const auto& brickPos = registry.GetComponent<RType::ECS::Position>(brick);
            auto& ballVel = registry.GetComponent<RType::ECS::Velocity>(ball);
            auto& brickHealth = registry.GetComponent<RType::ECS::Health>(brick);

            float dx = std::abs(ballPos.x - brickPos.x);
            float dy = std::abs(ballPos.y - brickPos.y);

            if (dx > dy) {
                ballVel.dx = -ballVel.dx;
            } else {
                ballVel.dy = std::abs(ballVel.dy);
            }

            brickHealth.current--;
            if (brickHealth.current <= 0) {
                registry.DestroyEntity(brick);
            }
        }

        void BreakoutCollisionSystem::HandleBallWallCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, float screenWidth, float screenHeight) {
            if (!registry.HasComponent<RType::ECS::Position>(ball) || !registry.HasComponent<RType::ECS::Velocity>(ball) ||
                !registry.HasComponent<RType::ECS::CircleCollider>(ball)) {
                return;
            }

            auto& ballPos = registry.GetComponent<RType::ECS::Position>(ball);
            auto& ballVel = registry.GetComponent<RType::ECS::Velocity>(ball);
            float radius = registry.GetComponent<RType::ECS::CircleCollider>(ball).radius;

            if (ballPos.x - radius <= 0.0f) {
                ballPos.x = radius;
                ballVel.dx = std::abs(ballVel.dx);
            } else if (ballPos.x + radius >= screenWidth) {
                ballPos.x = screenWidth - radius;
                ballVel.dx = -std::abs(ballVel.dx);
            }

            if (ballPos.y - radius <= 0.0f) {
                ballPos.y = radius;
                ballVel.dy = std::abs(ballVel.dy);
            }

            if (ballPos.y + radius >= screenHeight) {
                ballPos.x = screenWidth / 2.0f;
                ballPos.y = screenHeight / 2.0f;

                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_real_distribution<float> angleDist(-45.0f, 45.0f);

                float angle = angleDist(gen) * (3.14159f / 180.0f);
                ballVel.dx = 200.0f * std::sin(angle);
                ballVel.dy = 200.0f * std::abs(std::cos(angle));
            }
        }

    }
}
