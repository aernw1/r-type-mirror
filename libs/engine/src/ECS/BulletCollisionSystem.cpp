#include "../include/ECS/BulletCollisionSystem.hpp"
#include "../include/ECS/Registry.hpp"
#include "../include/ECS/Component.hpp"

namespace RType {
    namespace ECS {

        void BulletCollisionSystem::Update(Registry& registry, float deltaTime) {
            (void)deltaTime;
            auto bullets = registry.GetEntitiesWithComponent<Velocity>();
            auto enemies = registry.GetEntitiesWithComponent<Enemy>();

            if (bullets.empty() || enemies.empty()) {
                return;
            }

            for (auto bullet : bullets) {
                if (!registry.IsEntityAlive(bullet)) {
                    continue;
                }

                for (auto enemy : enemies) {
                    if (!registry.IsEntityAlive(enemy)) {
                        continue;
                    }

                    if (CollisionSystem::CheckCollision(registry, bullet, enemy)) {
                        if (!registry.HasComponent<Health>(enemy) || !registry.HasComponent<Damage>(bullet)) {
                            continue;
                        }
                        auto& healthComp = registry.GetComponent<Health>(enemy);
                        const auto& damageComp = registry.GetComponent<Damage>(bullet);
                        
                        healthComp.current = healthComp.current - damageComp.amount;
                        int healthEnemy = healthComp.current;
                        
                        
                        if (healthEnemy <= 0) {
                            if (!registry.HasComponent<Enemy>(enemy) || !registry.HasComponent<Bullet>(bullet)) {
                                continue;
                            }
                            const auto& enemyComp = registry.GetComponent<Enemy>(enemy);
                            const auto& bulletComp = registry.GetComponent<Bullet>(bullet);
                            
                            auto& enemyKilledComp = registry.AddComponent<EnemyKilled>(enemy, EnemyKilled(enemyComp.id, bulletComp.owner));
                        }

                        registry.DestroyEntity(bullet);
                        break;
                    }
                }
            }
        }
    }
}