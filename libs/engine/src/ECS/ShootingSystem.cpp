#include "../../include/ECS/ScoreSystem.hpp"
#include "../../include/ECS/Component.hpp"
#include "../../include/ECS/ShootingSystem.hpp"
#include "../../include/ECS/RenderingSystem.hpp"
#include <cmath>

namespace RType {
    namespace ECS {

        ShootingSystem::ShootingSystem(Renderer::SpriteId bulletSprite)
            : m_bulletSprite(bulletSprite) {}

        void ShootingSystem::Update(Registry& registry, float deltaTime) {

            auto bullets = registry.GetEntitiesWithComponent<Bullet>();
            std::vector<Entity> bulletsToDestroy;

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

                            spawns.push_back({positionComp.x + shooterComp.offsetX,
                                              positionComp.y + shooterComp.offsetY,
                                              shooterEntity});

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

                registry.AddComponent<CircleCollider>(bulletEntity, CircleCollider(5.0f));
                registry.AddComponent<CollisionLayer>(bulletEntity,
                                                      CollisionLayer(CollisionLayers::PLAYER_BULLET,
                                                                     CollisionLayers::ENEMY | CollisionLayers::OBSTACLE));
            }

            // Handle WeaponSlot components (spread shot, laser, etc.)
            auto weaponSlots = registry.GetEntitiesWithComponent<WeaponSlot>();

            for (auto entity : weaponSlots) {
                if (!registry.IsEntityAlive(entity)) continue;

                auto& weapon = registry.GetComponent<WeaponSlot>(entity);
                if (!weapon.enabled) continue;

                weapon.cooldown -= deltaTime;
                if (weapon.cooldown < 0.0f) weapon.cooldown = 0.0f;

                if (registry.HasComponent<ShootCommand>(entity)) {
                    auto& shootCmd = registry.GetComponent<ShootCommand>(entity);

                    if (shootCmd.wantsToShoot && weapon.cooldown <= 0.0f) {
                        if (registry.HasComponent<Position>(entity)) {
                            const auto& pos = registry.GetComponent<Position>(entity);

                            switch (weapon.type) {
                                case WeaponType::SPREAD:
                                    CreateSpreadShot(registry, entity, pos, weapon.damage);
                                    break;
                                case WeaponType::LASER:
                                    CreateLaserShot(registry, entity, pos, weapon.damage);
                                    break;
                                default:
                                    break;
                            }

                            weapon.cooldown = weapon.fireRate;
                        }
                    }
                }
            }
        }

        void ShootingSystem::CreateSpreadShot(Registry& registry, Entity shooter, const Position& pos, int damage) {
            // Create 3 bullets at different angles
            float angles[] = {-15.0f, 0.0f, 15.0f}; // degrees

            for (float angle : angles) {
                float radians = angle * 3.14159f / 180.0f;
                float vx = 600.0f * std::cos(radians);
                float vy = 600.0f * std::sin(radians);

                auto bullet = registry.CreateEntity();
                registry.AddComponent<Position>(bullet, Position(pos.x + 50, pos.y + 25));
                registry.AddComponent<Velocity>(bullet, Velocity(vx, vy));
                registry.AddComponent<Bullet>(bullet, Bullet(shooter));
                registry.AddComponent<Damage>(bullet, Damage(damage));
                registry.AddComponent<BoxCollider>(bullet, BoxCollider(8.0f, 4.0f));

                if (m_bulletSprite != 0) {
                    auto& d = registry.AddComponent<Drawable>(bullet, Drawable(m_bulletSprite, 2));
                    d.scale = {0.08f, 0.08f};
                    d.tint = Math::Color(1.0f, 1.0f, 0.0f, 1.0f); // Yellow for spread
                }
            }
        }

        void ShootingSystem::CreateLaserShot(Registry& registry, Entity shooter, const Position& pos, int damage) {
            auto bullet = registry.CreateEntity();
            registry.AddComponent<Position>(bullet, Position(pos.x + 50, pos.y + 25));
            registry.AddComponent<Velocity>(bullet, Velocity(800.0f, 0.0f)); // Faster
            registry.AddComponent<Bullet>(bullet, Bullet(shooter));
            registry.AddComponent<Damage>(bullet, Damage(damage));
            registry.AddComponent<BoxCollider>(bullet, BoxCollider(30.0f, 3.0f)); // Longer

            if (m_bulletSprite != 0) {
                auto& d = registry.AddComponent<Drawable>(bullet, Drawable(m_bulletSprite, 2));
                d.scale = {0.3f, 0.05f}; // Long and thin
                d.tint = Math::Color(0.0f, 1.0f, 1.0f, 1.0f); // Cyan for laser
            }
        }
    }
}
