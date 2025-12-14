/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PlayerCollisionResponseSystem implementation
*/

#include "ECS/PlayerCollisionResponseSystem.hpp"
#include <cmath>
#include <algorithm>

namespace RType {
    namespace ECS {

        void PlayerCollisionResponseSystem::Update(Registry& registry, float deltaTime) {
            auto players = registry.GetEntitiesWithComponent<Player>();

            for (auto player : players) {
                if (!registry.IsEntityAlive(player)) {
                    continue;
                }

                if (registry.HasComponent<Invincibility>(player)) {
                    auto& invincibility = registry.GetComponent<Invincibility>(player);
                    invincibility.remainingTime -= deltaTime;
                    if (invincibility.remainingTime <= 0.0f) {
                        invincibility.remainingTime = 0.0f;
                    }
                }

                if (!registry.HasComponent<CollisionEvent>(player)) {
                    continue;
                }

                auto& event = registry.GetComponent<CollisionEvent>(player);
                Entity other = event.other;

                if (!registry.IsEntityAlive(other)) {
                    continue;
                }

                bool hitEnemy = registry.HasComponent<Enemy>(other);
                bool hitObstacle = registry.HasComponent<Obstacle>(other);

                if (hitEnemy) {
                    if (registry.HasComponent<Damage>(other) && registry.HasComponent<Health>(player)) {
                        const auto& damageComp = registry.GetComponent<Damage>(other);
                        auto& playerHealth = registry.GetComponent<Health>(player);

                        playerHealth.current -= damageComp.amount;
                        if (playerHealth.current < 0) {
                            playerHealth.current = 0;
                        }
                    }

                    registry.DestroyEntity(other);
                }

                if (hitObstacle) {
                    const auto& obstacle = registry.GetComponent<Obstacle>(other);

                    if (obstacle.blocking) {
                        bool isInvincible = false;
                        if (registry.HasComponent<Invincibility>(player)) {
                            auto& invincibility = registry.GetComponent<Invincibility>(player);
                            if (invincibility.remainingTime > 0.0f) {
                                isInvincible = true;
                            }
                        }

                        if (!isInvincible && registry.HasComponent<Health>(player)) {
                            auto& playerHealth = registry.GetComponent<Health>(player);
                            constexpr int OBSTACLE_DAMAGE = 10;
                            playerHealth.current -= OBSTACLE_DAMAGE;

                            if (playerHealth.current < 0) {
                                playerHealth.current = 0;
                            }

                            constexpr float INVINCIBILITY_DURATION = 1.0f;
                            if (registry.HasComponent<Invincibility>(player)) {
                                auto& invincibility = registry.GetComponent<Invincibility>(player);
                                invincibility.remainingTime = INVINCIBILITY_DURATION;
                            } else {
                                registry.AddComponent<Invincibility>(player, Invincibility(INVINCIBILITY_DURATION));
                            }
                        }

                        if (registry.HasComponent<Position>(player) &&
                            registry.HasComponent<Position>(other)) {

                            bool resolved = false;
                            const bool hasPlayerBox = registry.HasComponent<BoxCollider>(player);
                            const bool hasObstacleBox = registry.HasComponent<BoxCollider>(other);

                            if (hasPlayerBox && hasObstacleBox) {
                                auto& playerPosRef = registry.GetComponent<Position>(player);
                                const auto& obstaclePos = registry.GetComponent<Position>(other);
                                const auto& playerBox = registry.GetComponent<BoxCollider>(player);
                                const auto& obstacleBox = registry.GetComponent<BoxCollider>(other);

                                float playerLeft = playerPosRef.x;
                                float playerRight = playerPosRef.x + playerBox.width;
                                float playerTop = playerPosRef.y;
                                float playerBottom = playerPosRef.y + playerBox.height;

                                float obstacleLeft = obstaclePos.x;
                                float obstacleRight = obstaclePos.x + obstacleBox.width;
                                float obstacleTop = obstaclePos.y;
                                float obstacleBottom = obstaclePos.y + obstacleBox.height;

                                float penetrationRight = obstacleRight - playerLeft;
                                float penetrationLeft = playerRight - obstacleLeft;
                                float penetrationBottom = obstacleBottom - playerTop;
                                float penetrationTop = playerBottom - obstacleTop;

                                if (penetrationLeft > 0.0f && penetrationRight > 0.0f &&
                                    penetrationTop > 0.0f && penetrationBottom > 0.0f) {
                                    const float separationBias = 0.5f;

                                    if (std::min(penetrationLeft, penetrationRight) <
                                        std::min(penetrationTop, penetrationBottom)) {
                                        if (penetrationLeft < penetrationRight) {
                                            playerPosRef.x -= penetrationLeft + separationBias;
                                        } else {
                                            playerPosRef.x += penetrationRight + separationBias;
                                        }
                                        if (registry.HasComponent<Velocity>(player)) {
                                            auto& playerVel = registry.GetComponent<Velocity>(player);
                                            playerVel.dx = 0.0f;
                                        }
                                    } else {
                                        if (penetrationTop < penetrationBottom) {
                                            playerPosRef.y -= penetrationTop + separationBias;
                                        } else {
                                            playerPosRef.y += penetrationBottom + separationBias;
                                        }
                                        if (registry.HasComponent<Velocity>(player)) {
                                            auto& playerVel = registry.GetComponent<Velocity>(player);
                                            playerVel.dy = 0.0f;
                                        }
                                    }
                                    resolved = true;
                                }
                            }

                            if (!resolved) {
                                // Fallback to radial push when AABB data is missing
                                if (registry.HasComponent<Velocity>(player)) {
                                    auto& playerVel = registry.GetComponent<Velocity>(player);
                                    playerVel.dx = 0.0f;
                                    playerVel.dy = 0.0f;
                                }

                                const auto& playerPos = registry.GetComponent<Position>(player);
                                const auto& obstaclePos = registry.GetComponent<Position>(other);

                                float dx = playerPos.x - obstaclePos.x;
                                float dy = playerPos.y - obstaclePos.y;
                                float distance = std::sqrt(dx * dx + dy * dy);

                                if (distance > 0.0f) {
                                    dx /= distance;
                                    dy /= distance;

                                    float pushDistance = 5.0f;
                                    if (registry.HasComponent<BoxCollider>(other)) {
                                        const auto& box = registry.GetComponent<BoxCollider>(other);
                                        pushDistance = std::max(box.width, box.height) * 0.5f + 10.0f;
                                    } else if (registry.HasComponent<CircleCollider>(other)) {
                                        const auto& circle = registry.GetComponent<CircleCollider>(other);
                                        pushDistance = circle.radius + 10.0f;
                                    }

                                    auto& playerPosRef = registry.GetComponent<Position>(player);
                                    playerPosRef.x = obstaclePos.x + dx * pushDistance;
                                    playerPosRef.y = obstaclePos.y + dy * pushDistance;
                                }
                            }
                        }
                    }
                }
            }
        }

    }
}
