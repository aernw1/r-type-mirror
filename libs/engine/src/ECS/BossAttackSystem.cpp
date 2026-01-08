/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BossAttackSystem - Handles boss attack patterns
*/

#include "ECS/BossAttackSystem.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
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

                if (registry.HasComponent<Scrollable>(bossEntity)) {
                    continue;
                }

                if (!registry.HasComponent<BossAttack>(bossEntity) ||
                    !registry.HasComponent<Position>(bossEntity)) {
                    continue;
                }

                auto& attack = registry.GetComponent<BossAttack>(bossEntity);
                const auto& pos = registry.GetComponent<Position>(bossEntity);

                attack.timeSinceLastAttack += deltaTime;

                if (attack.timeSinceLastAttack >= attack.attackCooldown) {
                    attack.timeSinceLastAttack = 0.0f;

                    switch (attack.currentPattern) {
                        case BossAttackPattern::FAN_SPRAY:
                            CreateFanSpray(registry, bossEntity, pos.x, pos.y);
                            attack.currentPattern = BossAttackPattern::BLACK_ORB;
                            break;
                        case BossAttackPattern::BLACK_ORB:
                            CreateBlackOrb(registry, bossEntity, pos.x, pos.y);
                            attack.currentPattern = BossAttackPattern::FAN_SPRAY;
                            break;
                        case BossAttackPattern::IDLE:
                        default:
                            break;
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

    }
}
