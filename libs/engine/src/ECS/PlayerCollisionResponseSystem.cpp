/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PlayerCollisionResponseSystem implementation
*/

#include "ECS/PlayerCollisionResponseSystem.hpp"

namespace RType {
    namespace ECS {

        void PlayerCollisionResponseSystem::Update(Registry& registry, float deltaTime) {
            auto players = registry.GetEntitiesWithComponent<Player>();

            for (auto player : players) {
                if (!registry.IsEntityAlive(player)) {
                    continue;
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
                        // TODO: implement player-obstacle collision response logic here
                    }
                }

            }
        }

    }
}
