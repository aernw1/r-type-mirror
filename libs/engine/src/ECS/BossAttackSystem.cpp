/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BossAttackSystem - Handles boss attack patterns
*/

#include "ECS/BossAttackSystem.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace RType {
    namespace ECS {

        void BossAttackSystem::Update(Registry& registry, float deltaTime) {
            auto bosses = registry.GetEntitiesWithComponent<Boss>();

            for (auto bossEntity : bosses) {
                if (!registry.IsEntityAlive(bossEntity)) {
                    continue;
                }

                if (registry.HasComponent<BossKilled>(bossEntity)) {
                    continue;
                }

                if (registry.HasComponent<Scrollable>(bossEntity)) {
                    continue;
                }

                if (!registry.HasComponent<BossAttack>(bossEntity) ||
                    !registry.HasComponent<Position>(bossEntity) ||
                    !registry.HasComponent<Boss>(bossEntity)) {
                    continue;
                }

                auto& attack = registry.GetComponent<BossAttack>(bossEntity);
                const auto& pos = registry.GetComponent<Position>(bossEntity);
                const auto& boss = registry.GetComponent<Boss>(bossEntity);

                attack.timeSinceLastAttack += deltaTime;

                if (attack.timeSinceLastAttack >= attack.attackCooldown) {
                    attack.timeSinceLastAttack = 0.0f;

                    if (boss.bossId == 1) {
                        // Boss 1
                        switch (attack.currentPattern) {
                            case BossAttackPattern::FAN_SPRAY:
                                CreateFanSpray(registry, bossEntity, pos.x, pos.y);
                                attack.currentPattern = BossAttackPattern::THIRD_BULLET;
                                break;
                            case BossAttackPattern::THIRD_BULLET:
                                CreateThirdBullet(registry, bossEntity, pos.x, pos.y);
                                attack.currentPattern = BossAttackPattern::BLACK_ORB;
                                break;
                            case BossAttackPattern::BLACK_ORB:
                                CreateBlackOrb(registry, bossEntity, pos.x, pos.y);
                                attack.currentPattern = BossAttackPattern::FAN_SPRAY;
                                break;
                            default:
                                break;
                        }
                    } else if (boss.bossId == 2) {
                        // Boss 2
                        CreateAnimatedOrb(registry, bossEntity, pos.x, pos.y);
                    }
                }
            }
        }

        void BossAttackSystem::CreateFanSpray(Registry& registry, Entity bossEntity, float bossX, float bossY) {
            const int bulletCount = 10;
            const float spreadAngle = M_PI / 3;
            const float bulletSpeed = 300.0f;

            const float baseAngle = M_PI;
            const float startAngle = baseAngle - spreadAngle / 2.0f;

            const float shootX = bossX + 64.0f;
            const float shootY = bossY + 188.0f;

            Core::Logger::Info("[BossAttackSystem] Boss firing fan spray with {} bullets", bulletCount);

            for (int i = 0; i < bulletCount; i++) {
                float angle = startAngle + (spreadAngle * i / (bulletCount - 1));
                CreateBossBullet(registry, shootX, shootY, angle, bulletSpeed);
            }
        }

        void BossAttackSystem::CreateBossBullet(Registry& registry, float x, float y, float angle, float speed) {
            Entity bullet = registry.CreateEntity();

            // CRITICAL FIX: Clean up obstacle components from entity ID reuse
            if (registry.HasComponent<Obstacle>(bullet)) {
                std::cerr << "[BOSS CLEANUP] Removing Obstacle from bullet entity " << bullet << std::endl;
                registry.RemoveComponent<Obstacle>(bullet);
            }
            if (registry.HasComponent<ObstacleMetadata>(bullet)) {
                std::cerr << "[BOSS CLEANUP] Removing ObstacleMetadata from bullet entity " << bullet << std::endl;
                registry.RemoveComponent<ObstacleMetadata>(bullet);
            }

            registry.AddComponent<Position>(bullet, Position{x, y});

            float vx = std::cos(angle) * speed;
            float vy = std::sin(angle) * speed;
            registry.AddComponent<Velocity>(bullet, Velocity{vx, vy});

            registry.AddComponent<Bullet>(bullet, Bullet{false});

            registry.AddComponent<BossBullet>(bullet, BossBullet{});

            registry.AddComponent<Damage>(bullet, Damage{5});

            registry.AddComponent<CircleCollider>(bullet, CircleCollider{10.0f});

            registry.AddComponent<CollisionLayer>(bullet,
                CollisionLayer(CollisionLayers::ENEMY_BULLET,
                              CollisionLayers::PLAYER));

            Core::Logger::Debug("[BossAttackSystem] Created bullet at ({}, {}) angle={} speed={}",
                               x, y, angle, speed);
        }

        void BossAttackSystem::CreateBlackOrb(Registry& registry, Entity bossEntity, float bossX, float bossY) {
            Entity orb = registry.CreateEntity();

            // CRITICAL FIX: Clean up obstacle components from entity ID reuse
            if (registry.HasComponent<Obstacle>(orb)) {
                std::cerr << "[BOSS CLEANUP] Removing Obstacle from orb entity " << orb << std::endl;
                registry.RemoveComponent<Obstacle>(orb);
            }
            if (registry.HasComponent<ObstacleMetadata>(orb)) {
                std::cerr << "[BOSS CLEANUP] Removing ObstacleMetadata from orb entity " << orb << std::endl;
                registry.RemoveComponent<ObstacleMetadata>(orb);
            }

            const float spawnX = bossX + 70.0f;
            const float spawnY = bossY + 160.0f;

            registry.AddComponent<Position>(orb, Position{spawnX, spawnY});
            

            float vx, vy;
            int trajectory = rand() % 3;
            const float speed = 70.0f;

            switch(trajectory) {
                case 0:
                    vx = -speed;
                    vy = 0.0f;
                    break;
                case 1:
                    vx = -speed * 0.657f;
                    vy = speed * 0.657f;
                    break;
                case 2:
                     vx = -speed;
                    vy = 30.0f;
                    break;
                default:
                    vx = -speed * 0.707f;
                    vy = speed * 0.707f;
                    break;
            }

            registry.AddComponent<Velocity>(orb, Velocity{vx, vy});

            registry.AddComponent<Bullet>(orb, Bullet{bossEntity});

            registry.AddComponent<BlackOrb>(orb, BlackOrb{900.0f, 110.0f, 1900.0f});
            registry.AddComponent<ProximityDamage>(orb, ProximityDamage{150.0f, 2.0f, 0.2f});

            Core::Logger::Info("[BossAttackSystem] Created Black Orb at ({}, {}) trajectory={}",
                              spawnX, spawnY, trajectory);
        }

        void BossAttackSystem::CreateThirdBullet(Registry& registry, Entity bossEntity, float bossX, float bossY) {
            Entity thirdBullet = registry.CreateEntity();

            // CRITICAL FIX: Clean up obstacle components from entity ID reuse
            if (registry.HasComponent<Obstacle>(thirdBullet)) {
                std::cerr << "[BOSS CLEANUP] Removing Obstacle from third bullet entity " << thirdBullet << std::endl;
                registry.RemoveComponent<Obstacle>(thirdBullet);
            }
            if (registry.HasComponent<ObstacleMetadata>(thirdBullet)) {
                std::cerr << "[BOSS CLEANUP] Removing ObstacleMetadata from third bullet entity " << thirdBullet << std::endl;
                registry.RemoveComponent<ObstacleMetadata>(thirdBullet);
            }

            const float spawnX = bossX + 350.0f;
            const float spawnY = bossY + -30.0f;

            registry.AddComponent<Position>(thirdBullet, Position{spawnX, spawnY});

            const float speed = 150.0f;
            registry.AddComponent<Velocity>(thirdBullet, Velocity{-speed, 0.0f});

            registry.AddComponent<Bullet>(thirdBullet, Bullet{bossEntity});

            registry.AddComponent<ThirdBullet>(thirdBullet, ThirdBullet{0.4f, 50});

            registry.AddComponent<CircleCollider>(thirdBullet, CircleCollider{30.0f});
            registry.AddComponent<CollisionLayer>(thirdBullet,
                CollisionLayer(CollisionLayers::OBSTACLE, CollisionLayers::PLAYER));
            registry.AddComponent<Damage>(thirdBullet, Damage{50});

            registry.AddComponent<BossBullet>(thirdBullet, BossBullet{});

            Core::Logger::Info("[BossAttackSystem] Created Third Bullet at ({}, {})", spawnX, spawnY);
        }

        void BossAttackSystem::CreateAnimatedOrb(Registry& registry, Entity bossEntity, float bossX, float bossY) {
            const int orbCount = 4;
            const float orbSpacing = 100.0f;
            const float baseSpeed = 350.0f;

            const float spawnX = bossX + 50.0f;
            const float startY = bossY + 50.0f;

            for (int i = 0; i < orbCount; i++) {
                Entity orb = registry.CreateEntity();

                if (registry.HasComponent<Obstacle>(orb)) {
                    registry.RemoveComponent<Obstacle>(orb);
                }
                if (registry.HasComponent<ObstacleMetadata>(orb)) {
                    registry.RemoveComponent<ObstacleMetadata>(orb);
                }

                float orbY = startY + (i * orbSpacing);
                registry.AddComponent<Position>(orb, Position{spawnX, orbY});

                float speedVariation = baseSpeed + (i * 10.0f);
                float angle = -M_PI + (i * 0.1f);
                float vx = std::cos(angle) * speedVariation;
                float vy = std::sin(angle) * speedVariation * 0.3f;

                registry.AddComponent<Velocity>(orb, Velocity{vx, vy});

                registry.AddComponent<Bullet>(orb, Bullet{bossEntity});
                registry.AddComponent<BossBullet>(orb, BossBullet{});
                registry.AddComponent<WaveAttack>(orb, WaveAttack{});

                registry.AddComponent<CircleCollider>(orb, CircleCollider{15.0f});
                registry.AddComponent<CollisionLayer>(orb,
                    CollisionLayer(CollisionLayers::ENEMY_BULLET, CollisionLayers::PLAYER));
                registry.AddComponent<Damage>(orb, Damage{8});
            }

            Core::Logger::Info("[BossAttackSystem] Boss 2 created wave attack burst with {} orbs", orbCount);
        }

    }
}
