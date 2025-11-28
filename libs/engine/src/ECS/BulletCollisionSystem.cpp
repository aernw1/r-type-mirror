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
                        auto& healthComp = registry.GetComponent<Health>(enemy);
                        const auto& damageComp = registry.GetComponent<Damage>(bullet);
                        
                        int healthEnnemy = healthComp.current -= damageComp.amount;
                        
                        if (healthEnnemy <= 0) {
                            const auto& enemyComp = registry.GetComponent<Enemy>(enemy);
                            const auto& bulletComp = registry.GetComponent<Bullet>(bullet);
                            
                            auto& ennemyKilledComp = registry.AddComponent<EnnemyKilled>(enemy, EnnemyKilled(enemyComp.id, bulletComp.owner));
                        }

                        registry.DestroyEntity(bullet);
                        break;
                    }
                }
            }
        }
    }
}