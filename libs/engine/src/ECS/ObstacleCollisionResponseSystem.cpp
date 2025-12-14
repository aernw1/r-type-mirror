/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ObstacleCollisionResponseSystem implementation
*/

#include "ECS/ObstacleCollisionResponseSystem.hpp"
#include <cmath>
#include <algorithm>

namespace RType {
    namespace ECS {

        void ObstacleCollisionResponseSystem::Update(Registry& registry, float deltaTime) {
            auto obstacles = registry.GetEntitiesWithComponent<Obstacle>();

            for (auto obstacle : obstacles) {
                if (!registry.IsEntityAlive(obstacle)) {
                    continue;
                }

                if (!registry.HasComponent<CollisionEvent>(obstacle)) {
                    continue;
                }

                const auto& obstacleComp = registry.GetComponent<Obstacle>(obstacle);
                if (!obstacleComp.blocking) {
                    continue;
                }

                auto& event = registry.GetComponent<CollisionEvent>(obstacle);
                Entity other = event.other;

                if (!registry.IsEntityAlive(other)) {
                    continue;
                }

                // Handle enemy-obstacle collisions (enemies take damage and are blocked)
                if (registry.HasComponent<Enemy>(other)) {
                    // Apply damage to enemy
                    if (registry.HasComponent<Health>(other)) {
                        auto& enemyHealth = registry.GetComponent<Health>(other);
                        // Obstacles deal damage to enemies
                        constexpr int OBSTACLE_DAMAGE = 50;
                        enemyHealth.current -= OBSTACLE_DAMAGE;
                        
                        if (enemyHealth.current <= 0) {
                            enemyHealth.current = 0;
                            // Enemy will be destroyed by HealthSystem
                        }
                    }

                    if (registry.HasComponent<Position>(other) &&
                        registry.HasComponent<Position>(obstacle)) {

                        bool resolved = false;
                        const bool hasEnemyBox = registry.HasComponent<BoxCollider>(other);
                        const bool hasObstacleBox = registry.HasComponent<BoxCollider>(obstacle);

                        if (hasEnemyBox && hasObstacleBox) {
                            auto& enemyPosRef = registry.GetComponent<Position>(other);
                            const auto& obstaclePos = registry.GetComponent<Position>(obstacle);
                            const auto& enemyBox = registry.GetComponent<BoxCollider>(other);
                            const auto& obstacleBox = registry.GetComponent<BoxCollider>(obstacle);

                            float enemyLeft = enemyPosRef.x;
                            float enemyRight = enemyPosRef.x + enemyBox.width;
                            float enemyTop = enemyPosRef.y;
                            float enemyBottom = enemyPosRef.y + enemyBox.height;

                            float obstacleLeft = obstaclePos.x;
                            float obstacleRight = obstaclePos.x + obstacleBox.width;
                            float obstacleTop = obstaclePos.y;
                            float obstacleBottom = obstaclePos.y + obstacleBox.height;

                            float penRight = obstacleRight - enemyLeft;
                            float penLeft = enemyRight - obstacleLeft;
                            float penBottom = obstacleBottom - enemyTop;
                            float penTop = enemyBottom - obstacleTop;

                            if (penLeft > 0.0f && penRight > 0.0f &&
                                penTop > 0.0f && penBottom > 0.0f) {
                                const float separationBias = 0.5f;

                                if (std::min(penLeft, penRight) <
                                    std::min(penTop, penBottom)) {
                                    if (penLeft < penRight) {
                                        enemyPosRef.x -= penLeft + separationBias;
                                    } else {
                                        enemyPosRef.x += penRight + separationBias;
                                    }
                                    if (registry.HasComponent<Velocity>(other)) {
                                        auto& enemyVel = registry.GetComponent<Velocity>(other);
                                        enemyVel.dx = 0.0f;
                                    }
                                } else {
                                    if (penTop < penBottom) {
                                        enemyPosRef.y -= penTop + separationBias;
                                    } else {
                                        enemyPosRef.y += penBottom + separationBias;
                                    }
                                    if (registry.HasComponent<Velocity>(other)) {
                                        auto& enemyVel = registry.GetComponent<Velocity>(other);
                                        enemyVel.dy = 0.0f;
                                    }
                                }
                                resolved = true;
                            }
                        }

                        if (!resolved) {
                            if (registry.HasComponent<Velocity>(other)) {
                                auto& enemyVel = registry.GetComponent<Velocity>(other);
                                enemyVel.dx = 0.0f;
                                enemyVel.dy = 0.0f;
                            }

                            const auto& enemyPos = registry.GetComponent<Position>(other);
                            const auto& obstaclePos = registry.GetComponent<Position>(obstacle);
                            float dx = enemyPos.x - obstaclePos.x;
                            float dy = enemyPos.y - obstaclePos.y;
                            float distance = std::sqrt(dx * dx + dy * dy);

                            if (distance > 0.0f) {
                                dx /= distance;
                                dy /= distance;

                                float pushDistance = 5.0f;
                                if (registry.HasComponent<BoxCollider>(obstacle)) {
                                    const auto& box = registry.GetComponent<BoxCollider>(obstacle);
                                    pushDistance = std::max(box.width, box.height) * 0.5f + 10.0f;
                                } else if (registry.HasComponent<CircleCollider>(obstacle)) {
                                    const auto& circle = registry.GetComponent<CircleCollider>(obstacle);
                                    pushDistance = circle.radius + 10.0f;
                                }

                                auto& enemyPosRef = registry.GetComponent<Position>(other);
                                enemyPosRef.x = obstaclePos.x + dx * pushDistance;
                                enemyPosRef.y = obstaclePos.y + dy * pushDistance;
                            }
                        }
                    }
                }
            }
        }

    }
}
