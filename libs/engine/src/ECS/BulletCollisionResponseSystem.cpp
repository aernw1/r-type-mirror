/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BulletCollisionResponseSystem implementation
*/

#include "ECS/BulletCollisionResponseSystem.hpp"

namespace RType {
    namespace ECS {

        void BulletCollisionResponseSystem::Update(Registry& registry, float deltaTime) {
            auto bullets = registry.GetEntitiesWithComponent<Bullet>();

            for (auto bullet : bullets) {
                if (!registry.IsEntityAlive(bullet)) {
                    continue;
                }

                if (!registry.HasComponent<CollisionEvent>(bullet)) {
                    continue;
                }

                auto& event = registry.GetComponent<CollisionEvent>(bullet);
                Entity other = event.other;

                if (!registry.IsEntityAlive(other)) {
                    continue;
                }

                bool hitEnemy = registry.HasComponent<Enemy>(other);
                bool hitPlayer = registry.HasComponent<Player>(other);
                bool hitObstacle = registry.HasComponent<Obstacle>(other);

                if (hitEnemy && registry.HasComponent<Health>(other)) {
                    auto& health = registry.GetComponent<Health>(other);
                    const auto& damage = registry.GetComponent<Damage>(bullet);
                    health.current -= damage.amount;

                    if (health.current <= 0) {
                        const auto& enemyComp = registry.GetComponent<Enemy>(other);
                        const auto& bulletComp = registry.GetComponent<Bullet>(bullet);
                        registry.AddComponent<EnemyKilled>(other,
                            EnemyKilled(enemyComp.id, bulletComp.owner));
                    }
                }

                if (hitPlayer && registry.HasComponent<Health>(other)) {
                    auto& health = registry.GetComponent<Health>(other);
                    const auto& damage = registry.GetComponent<Damage>(bullet);
                    health.current -= damage.amount;

                    if (health.current < 0) {
                        health.current = 0;
                    }
                }

                if (hitObstacle) {
                    const auto& obstacle = registry.GetComponent<Obstacle>(other);
                    if (!obstacle.blocking) {
                        continue;
                    }
                }

                if (hitEnemy || hitPlayer || (hitObstacle && registry.GetComponent<Obstacle>(other).blocking)) {
                    registry.DestroyEntity(bullet);
                }
            }
        }

    }
}
