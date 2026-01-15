#include "BreakoutPhysicsSystem.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"
#include <cmath>
#include <random>

namespace Breakout {
    namespace ECS {

        void BreakoutPhysicsSystem::Update(RType::ECS::Registry& registry, float deltaTime) {
            constexpr float SCREEN_WIDTH = 800.0f;
            constexpr float SCREEN_HEIGHT = 900.0f;

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

            if (!registry.HasComponent<RType::ECS::Position>(ballEntity) ||
                !registry.HasComponent<RType::ECS::CircleCollider>(ballEntity)) {
                return;
            }

            const auto& ballPos = registry.GetComponent<RType::ECS::Position>(ballEntity);
            const auto& ballCollider = registry.GetComponent<RType::ECS::CircleCollider>(ballEntity);
            float ballRadius = ballCollider.radius;

            RType::ECS::Entity paddleEntity = RType::ECS::NULL_ENTITY;
            auto controllableEntities = registry.GetEntitiesWithComponent<RType::ECS::Controllable>();
            for (auto entity : controllableEntities) {
                if (registry.HasComponent<RType::ECS::BoxCollider>(entity)) {
                    paddleEntity = entity;
                    break;
                }
            }

            if (paddleEntity != RType::ECS::NULL_ENTITY &&
                registry.HasComponent<RType::ECS::Position>(paddleEntity) &&
                registry.HasComponent<RType::ECS::BoxCollider>(paddleEntity)) {
                const auto& paddlePos = registry.GetComponent<RType::ECS::Position>(paddleEntity);
                const auto& paddleCollider = registry.GetComponent<RType::ECS::BoxCollider>(paddleEntity);

                float paddleLeft = paddlePos.x - paddleCollider.width / 2.0f;
                float paddleRight = paddlePos.x + paddleCollider.width / 2.0f;
                float paddleTop = paddlePos.y - paddleCollider.height / 2.0f;
                float paddleBottom = paddlePos.y + paddleCollider.height / 2.0f;

                float closestX = std::max(paddleLeft, std::min(ballPos.x, paddleRight));
                float closestY = std::max(paddleTop, std::min(ballPos.y, paddleBottom));

                float dx = ballPos.x - closestX;
                float dy = ballPos.y - closestY;
                float distanceSquared = dx * dx + dy * dy;

                if (distanceSquared < (ballRadius * ballRadius)) {
                    HandleBallPaddleCollision(registry, ballEntity, paddleEntity);
                }
            }

            auto brickEntities = registry.GetEntitiesWithComponent<RType::ECS::Health>();
            bool ballCollided = false;
            for (auto brick : brickEntities) {
                if (ballCollided) {
                    break;
                }
                if (registry.HasComponent<RType::ECS::Controllable>(brick)) {
                    continue;
                }
                if (!registry.HasComponent<RType::ECS::Position>(brick) ||
                    !registry.HasComponent<RType::ECS::BoxCollider>(brick) ||
                    !registry.IsEntityAlive(brick)) {
                    continue;
                }

                const auto& brickPos = registry.GetComponent<RType::ECS::Position>(brick);
                const auto& brickCollider = registry.GetComponent<RType::ECS::BoxCollider>(brick);

                float brickLeft = brickPos.x - brickCollider.width / 2.0f;
                float brickRight = brickPos.x + brickCollider.width / 2.0f;
                float brickTop = brickPos.y - brickCollider.height / 2.0f;
                float brickBottom = brickPos.y + brickCollider.height / 2.0f;

                float closestX = std::max(brickLeft, std::min(ballPos.x, brickRight));
                float closestY = std::max(brickTop, std::min(ballPos.y, brickBottom));

                float dx = ballPos.x - closestX;
                float dy = ballPos.y - closestY;
                float distanceSquared = dx * dx + dy * dy;

                if (distanceSquared < (ballRadius * ballRadius)) {
                    HandleBallBrickCollision(registry, ballEntity, brick);
                    ballCollided = true;
                }
            }

            HandleBallWallCollision(registry, ballEntity, SCREEN_WIDTH, SCREEN_HEIGHT);
        }

        void BreakoutPhysicsSystem::HandleBallPaddleCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, RType::ECS::Entity paddle) {
            if (!registry.HasComponent<RType::ECS::Position>(ball) || !registry.HasComponent<RType::ECS::Position>(paddle) ||
                !registry.HasComponent<RType::ECS::Velocity>(ball) || !registry.HasComponent<RType::ECS::BoxCollider>(paddle) ||
                !registry.HasComponent<RType::ECS::CircleCollider>(ball)) {
                return;
            }

            const auto& ballPos = registry.GetComponent<RType::ECS::Position>(ball);
            const auto& paddlePos = registry.GetComponent<RType::ECS::Position>(paddle);
            const auto& paddleCollider = registry.GetComponent<RType::ECS::BoxCollider>(paddle);
            const auto& ballCollider = registry.GetComponent<RType::ECS::CircleCollider>(ball);
            auto& ballVel = registry.GetComponent<RType::ECS::Velocity>(ball);
            auto& ballPosRef = registry.GetComponent<RType::ECS::Position>(ball);

            float ballRadius = ballCollider.radius;
            float paddleLeft = paddlePos.x - paddleCollider.width / 2.0f;
            float paddleRight = paddlePos.x + paddleCollider.width / 2.0f;
            float paddleTop = paddlePos.y - paddleCollider.height / 2.0f;
            float paddleBottom = paddlePos.y + paddleCollider.height / 2.0f;

            float relativeX = (ballPos.x - paddlePos.x) / (paddleCollider.width / 2.0f);
            relativeX = std::max(-1.0f, std::min(1.0f, relativeX));

            float angle = relativeX * 60.0f * (3.14159f / 180.0f);
            float speed = std::sqrt(ballVel.dx * ballVel.dx + ballVel.dy * ballVel.dy);
            if (speed < 250.0f) {
                speed = 300.0f;
            }

            ballVel.dx = speed * std::sin(angle);
            ballVel.dy = -std::abs(speed * std::cos(angle));

            if (ballPos.y > paddlePos.y) {
                ballPosRef.y = paddleBottom + ballRadius + 0.1f;
            } else {
                ballPosRef.y = paddleTop - ballRadius - 0.1f;
            }

            if (ballPos.x < paddleLeft) {
                ballPosRef.x = paddleLeft - ballRadius - 0.1f;
            } else if (ballPos.x > paddleRight) {
                ballPosRef.x = paddleRight + ballRadius + 0.1f;
            }
        }

        void BreakoutPhysicsSystem::HandleBallBrickCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, RType::ECS::Entity brick) {
            if (!registry.HasComponent<RType::ECS::Position>(ball) || !registry.HasComponent<RType::ECS::Position>(brick) ||
                !registry.HasComponent<RType::ECS::Velocity>(ball) || !registry.HasComponent<RType::ECS::Health>(brick) ||
                !registry.HasComponent<RType::ECS::BoxCollider>(brick) || !registry.HasComponent<RType::ECS::CircleCollider>(ball)) {
                return;
            }

            const auto& ballPos = registry.GetComponent<RType::ECS::Position>(ball);
            const auto& brickPos = registry.GetComponent<RType::ECS::Position>(brick);
            const auto& brickCollider = registry.GetComponent<RType::ECS::BoxCollider>(brick);
            const auto& ballCollider = registry.GetComponent<RType::ECS::CircleCollider>(ball);
            auto& ballVel = registry.GetComponent<RType::ECS::Velocity>(ball);
            auto& ballPosRef = registry.GetComponent<RType::ECS::Position>(ball);
            auto& brickHealth = registry.GetComponent<RType::ECS::Health>(brick);

            float brickLeft = brickPos.x - brickCollider.width / 2.0f;
            float brickRight = brickPos.x + brickCollider.width / 2.0f;
            float brickTop = brickPos.y - brickCollider.height / 2.0f;
            float brickBottom = brickPos.y + brickCollider.height / 2.0f;

            float ballRadius = ballCollider.radius;
            float overlapLeft = (ballPos.x + ballRadius) - brickLeft;
            float overlapRight = brickRight - (ballPos.x - ballRadius);
            float overlapTop = (ballPos.y + ballRadius) - brickTop;
            float overlapBottom = brickBottom - (ballPos.y - ballRadius);

            float minOverlap = std::min({overlapLeft, overlapRight, overlapTop, overlapBottom});

            if (minOverlap == overlapLeft || minOverlap == overlapRight) {
                ballVel.dx = -ballVel.dx;
                if (minOverlap == overlapLeft) {
                    ballPosRef.x = brickLeft - ballRadius - 0.1f;
                } else {
                    ballPosRef.x = brickRight + ballRadius + 0.1f;
                }
            } else {
                ballVel.dy = std::abs(ballVel.dy);
                if (minOverlap == overlapTop) {
                    ballPosRef.y = brickTop - ballRadius - 0.1f;
                } else {
                    ballPosRef.y = brickBottom + ballRadius + 0.1f;
                }
            }

            brickHealth.current--;
            if (brickHealth.current <= 0) {
                registry.DestroyEntity(brick);
            }
        }

        void BreakoutPhysicsSystem::HandleBallWallCollision(RType::ECS::Registry& registry, RType::ECS::Entity ball, float screenWidth, float screenHeight) {
            if (ball == RType::ECS::NULL_ENTITY || !registry.IsEntityAlive(ball)) {
                return;
            }

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
                ballVel.dx = 250.0f * std::sin(angle);
                ballVel.dy = 250.0f * std::abs(std::cos(angle));
            }
        }

    }
}

