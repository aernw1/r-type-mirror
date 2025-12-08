#include "../../include/ECS/BulletCollisionSystem.hpp"
#include "../../include/ECS/ScoreSystem.hpp"
#include "../../include/ECS/Component.hpp"
#include "../../include/ECS/ShootingSystem.hpp"
#include "../../include/ECS/RenderingSystem.hpp"

namespace RType {
    namespace ECS {

        ShootingSystem::ShootingSystem(Renderer::SpriteId bulletSprite)
            : m_bulletSprite(bulletSprite) {}

        void ShootingSystem::Update(Registry& registry, float deltaTime) {

            auto bullets = registry.GetEntitiesWithComponent<Bullet>();
            std::vector<Entity> bulletsToDestroy;

            // Cleanup
            for (auto bulletEntity : bullets) {
                 if (registry.IsEntityAlive(bulletEntity) && registry.HasComponent<Position>(bulletEntity)) {
                     auto& pos = registry.GetComponent<Position>(bulletEntity);
                     if (pos.x > 1380.0f || pos.x < -100.0f || pos.y > 820.0f || pos.y < -100.0f) {
                         bulletsToDestroy.push_back(bulletEntity);
                     }
                 }
            }

            for (auto entity : bulletsToDestroy) {
                registry.DestroyEntity(entity);
            }

            auto shooters = registry.GetEntitiesWithComponent<Shooter>();
            struct BulletSpawn {
                float x, y;
                Entity shooter;
            };
            std::vector<BulletSpawn> spawns;

            for (auto shooterEntity : shooters) {
                if (!registry.IsEntityAlive(shooterEntity) || !registry.HasComponent<Shooter>(shooterEntity)) {
                    continue;
                }
                auto& shooterComp = registry.GetComponent<Shooter>(shooterEntity);

                shooterComp.cooldown = shooterComp.cooldown - deltaTime;

                if (shooterComp.cooldown < 0.0f) {
                    shooterComp.cooldown = 0.0f;
                }

                if (registry.HasComponent<ShootCommand>(shooterEntity)) {
                    auto& shootCmd = registry.GetComponent<ShootCommand>(shooterEntity);

                    if (shootCmd.wantsToShoot && shooterComp.cooldown <= 0.0f) {
                        if (registry.HasComponent<Position>(shooterEntity)) {
                            const auto& positionComp = registry.GetComponent<Position>(shooterEntity);

                            spawns.push_back({
                                positionComp.x + shooterComp.offsetX,
                                positionComp.y + shooterComp.offsetY,
                                shooterEntity
                            });

                            shooterComp.cooldown = shooterComp.fireRate;
                        }
                    }
                }
            }

            for (const auto& spawn : spawns) {
                auto bulletEntity = registry.CreateEntity();
                registry.AddComponent<Position>(bulletEntity, Position(spawn.x, spawn.y));
                registry.AddComponent<Velocity>(bulletEntity, Velocity(600.0f, 0.0f));
                registry.AddComponent<Bullet>(bulletEntity, Bullet(spawn.shooter));
                
                if (m_bulletSprite != 0) {
                    auto& d = registry.AddComponent<Drawable>(bulletEntity, Drawable(m_bulletSprite, 2));
                    d.scale = {0.1f, 0.1f};
                }
                
                registry.AddComponent<Damage>(bulletEntity, Damage(25));
                registry.AddComponent<BoxCollider>(bulletEntity, BoxCollider(10.0f, 5.0f));
            }
        }
    }
}

