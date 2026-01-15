#include "../../include/ECS/ScoreSystem.hpp"
#include "../../include/ECS/Component.hpp"
#include "../../include/ECS/ShootingSystem.hpp"
#include "../../include/ECS/RenderingSystem.hpp"
#include "../../include/ECS/EffectFactory.hpp"
#include <cmath>
#include <iostream>

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

                // Skip entities with WeaponSlot - they use special weapons instead of base Shooter
                if (registry.HasComponent<WeaponSlot>(shooterEntity)) {
                    continue;
                }

                // CRITICAL FIX: Prevent obstacles from shooting due to entity ID reuse
                // Actively clean up contaminated components
                if (registry.HasComponent<Obstacle>(shooterEntity) ||
                    registry.HasComponent<ObstacleMetadata>(shooterEntity)) {
                    std::cerr << "[SHOOTING] BLOCKED obstacle entity " << shooterEntity
                              << " from shooting (has Shooter=" << registry.HasComponent<Shooter>(shooterEntity)
                              << " ShootCommand=" << registry.HasComponent<ShootCommand>(shooterEntity) << ")" << std::endl;
                    if (registry.HasComponent<Shooter>(shooterEntity)) {
                        registry.RemoveComponent<Shooter>(shooterEntity);
                    }
                    if (registry.HasComponent<ShootCommand>(shooterEntity)) {
                        registry.RemoveComponent<ShootCommand>(shooterEntity);
                    }
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

                            if (m_effectFactory) {
                                m_effectFactory->CreateShootingEffect(registry,
                                    positionComp.x + shooterComp.offsetX,
                                    positionComp.y + shooterComp.offsetY,
                                    shooterEntity);
                            }

                            shooterComp.cooldown = shooterComp.fireRate;
                            shootCmd.wantsToShoot = false;
                        }
                    }
                }
            }

            for (const auto& spawn : spawns) {
                auto bulletEntity = registry.CreateEntity();

                // CRITICAL FIX: Clean up obstacle components from entity ID reuse
                if (registry.HasComponent<Obstacle>(bulletEntity)) {
                    std::cerr << "[SHOOTING CLEANUP] Removing Obstacle from bullet entity " << bulletEntity << std::endl;
                    registry.RemoveComponent<Obstacle>(bulletEntity);
                }
                if (registry.HasComponent<ObstacleMetadata>(bulletEntity)) {
                    std::cerr << "[SHOOTING CLEANUP] Removing ObstacleMetadata from bullet entity " << bulletEntity << std::endl;
                    registry.RemoveComponent<ObstacleMetadata>(bulletEntity);
                }

                registry.AddComponent<Position>(bulletEntity, Position(spawn.x, spawn.y));
                registry.AddComponent<Velocity>(bulletEntity, Velocity(600.0f, 0.0f));
                registry.AddComponent<Bullet>(bulletEntity, Bullet(spawn.shooter));

                if (m_bulletSprite != 0) {
                    auto& d = registry.AddComponent<Drawable>(bulletEntity, Drawable(m_bulletSprite, 2));
                    d.scale = {0.1f, 0.1f};
                    d.origin = Math::Vector2(128.0f, 128.0f);
                }

                registry.AddComponent<Damage>(bulletEntity, Damage(25));
                registry.AddComponent<BoxCollider>(bulletEntity, BoxCollider(10.0f, 5.0f));

                registry.AddComponent<CircleCollider>(bulletEntity, CircleCollider(5.0f));
                registry.AddComponent<CollisionLayer>(bulletEntity,
                                                      CollisionLayer(CollisionLayers::PLAYER_BULLET,
                                                                     CollisionLayers::ENEMY | CollisionLayers::OBSTACLE));

                if (m_shootSound != Audio::INVALID_SOUND_ID) {
                    auto sfx = registry.CreateEntity();
                    auto& se = registry.AddComponent<SoundEffect>(sfx, SoundEffect(m_shootSound, 1.0f));
                    se.pitch = 1.0f;
                }
            }

            auto weaponSlots = registry.GetEntitiesWithComponent<WeaponSlot>();

            for (auto entity : weaponSlots) {
                if (!registry.IsEntityAlive(entity)) continue;

                // CRITICAL FIX: Prevent obstacles from shooting with weapon slots
                if (registry.HasComponent<Obstacle>(entity) ||
                    registry.HasComponent<ObstacleMetadata>(entity)) {
                    std::cerr << "[SHOOTING] BLOCKED obstacle entity " << entity
                              << " from using weapon slot (has WeaponSlot=" << registry.HasComponent<WeaponSlot>(entity)
                              << " ShootCommand=" << registry.HasComponent<ShootCommand>(entity) << ")" << std::endl;
                    if (registry.HasComponent<WeaponSlot>(entity)) {
                        registry.RemoveComponent<WeaponSlot>(entity);
                    }
                    if (registry.HasComponent<ShootCommand>(entity)) {
                        registry.RemoveComponent<ShootCommand>(entity);
                    }
                    continue;
                }

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
                            shootCmd.wantsToShoot = false;
                        }
                    }
                }
            }
        }

        void ShootingSystem::CreateSpreadShot(Registry& registry, Entity shooter, const Position& pos, int damage) {
            float offsetX = 50.0f;
            float offsetY = 20.0f;
            
            if (registry.HasComponent<Shooter>(shooter)) {
                const auto& shooterComp = registry.GetComponent<Shooter>(shooter);
                offsetX = shooterComp.offsetX;
                offsetY = shooterComp.offsetY;
            }
            
            if (m_effectFactory) {
                m_effectFactory->CreateShootingEffect(registry, pos.x + offsetX, pos.y + offsetY, shooter);
            }

            float angles[] = {-15.0f, 0.0f, 15.0f}; // degrees

            for (float angle : angles) {
                float radians = angle * 3.14159f / 180.0f;
                float vx = 600.0f * std::cos(radians);
                float vy = 600.0f * std::sin(radians);

                auto bullet = registry.CreateEntity();

                // CRITICAL FIX: Clean up obstacle components from entity ID reuse
                if (registry.HasComponent<Obstacle>(bullet)) {
                    registry.RemoveComponent<Obstacle>(bullet);
                }
                if (registry.HasComponent<ObstacleMetadata>(bullet)) {
                    registry.RemoveComponent<ObstacleMetadata>(bullet);
                }

                registry.AddComponent<Position>(bullet, Position(pos.x + offsetX, pos.y + offsetY));
                registry.AddComponent<Velocity>(bullet, Velocity(vx, vy));
                registry.AddComponent<Bullet>(bullet, Bullet(shooter));
                registry.AddComponent<Damage>(bullet, Damage(damage));
                registry.AddComponent<BoxCollider>(bullet, BoxCollider(8.0f, 4.0f));
                registry.AddComponent<CollisionLayer>(bullet,
                                                      CollisionLayer(CollisionLayers::PLAYER_BULLET,
                                                                     CollisionLayers::ENEMY | CollisionLayers::OBSTACLE));

                if (m_bulletSprite != 0) {
                    auto& d = registry.AddComponent<Drawable>(bullet, Drawable(m_bulletSprite, 2));
                    d.scale = {0.08f, 0.08f};
                    d.tint = Math::Color(1.0f, 1.0f, 0.0f, 1.0f); // Yellow for spread
                }
            }
        }

        void ShootingSystem::CreateLaserShot(Registry& registry, Entity shooter, const Position& pos, int damage) {
            float offsetX = 50.0f;
            float offsetY = 20.0f;
            
            if (registry.HasComponent<Shooter>(shooter)) {
                const auto& shooterComp = registry.GetComponent<Shooter>(shooter);
                offsetX = shooterComp.offsetX;
                offsetY = shooterComp.offsetY;
            }
            
            if (m_effectFactory) {
                m_effectFactory->CreateShootingEffect(registry, pos.x + offsetX, pos.y + offsetY, shooter);
            }

            auto bullet = registry.CreateEntity();

            // CRITICAL FIX: Clean up obstacle components from entity ID reuse
            if (registry.HasComponent<Obstacle>(bullet)) {
                registry.RemoveComponent<Obstacle>(bullet);
            }
            if (registry.HasComponent<ObstacleMetadata>(bullet)) {
                registry.RemoveComponent<ObstacleMetadata>(bullet);
            }

            registry.AddComponent<Position>(bullet, Position(pos.x + offsetX, pos.y + offsetY));
            registry.AddComponent<Velocity>(bullet, Velocity(800.0f, 0.0f)); // Faster
            registry.AddComponent<Bullet>(bullet, Bullet(shooter));
            registry.AddComponent<Damage>(bullet, Damage(damage));
            registry.AddComponent<BoxCollider>(bullet, BoxCollider(30.0f, 3.0f)); // Longer
            registry.AddComponent<CollisionLayer>(bullet,
                                                  CollisionLayer(CollisionLayers::PLAYER_BULLET,
                                                                 CollisionLayers::ENEMY | CollisionLayers::OBSTACLE));

            if (m_bulletSprite != 0) {
                auto& d = registry.AddComponent<Drawable>(bullet, Drawable(m_bulletSprite, 2));
                d.scale = {0.3f, 0.05f}; // Long and thin
                d.tint = Math::Color(0.0f, 1.0f, 1.0f, 1.0f); // Cyan for laser
            }
        }
    }
}
