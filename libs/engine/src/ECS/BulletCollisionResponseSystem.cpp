/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BulletCollisionResponseSystem implementation
*/

#include "ECS/BulletCollisionResponseSystem.hpp"
#include "ECS/EffectFactory.hpp"
#include "Core/Logger.hpp"
#include <unordered_map>
#include <cstdint>

namespace RType {
    namespace ECS {

        void BulletCollisionResponseSystem::Update(Registry& registry, float deltaTime) {
            auto bullets = registry.GetEntitiesWithComponent<Bullet>();
            
            static std::unordered_map<uint64_t, float> lastBeamDamageTime;
            static float totalTime = 0.0f;
            totalTime += deltaTime;
            
            auto it = lastBeamDamageTime.begin();
            while (it != lastBeamDamageTime.end()) {
                uint64_t key = it->first;
                Entity bulletEntity = static_cast<Entity>(key & 0xFFFFFFFF);
                Entity otherEntity = static_cast<Entity>((key >> 32) & 0xFFFFFFFF);
                
                if (!registry.IsEntityAlive(bulletEntity) || !registry.IsEntityAlive(otherEntity)) {
                    it = lastBeamDamageTime.erase(it);
                } else {
                    ++it;
                }
            }

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

                bool isBeam = false;
                if (registry.HasComponent<BoxCollider>(bullet)) {
                    const auto& collider = registry.GetComponent<BoxCollider>(bullet);
                    isBeam = (collider.width > 500.0f);
                }

                bool hitEnemy = registry.HasComponent<Enemy>(other);
                bool hitBoss = registry.HasComponent<Boss>(other);
                bool hitPlayer = registry.HasComponent<Player>(other);
                bool hitObstacle = registry.HasComponent<Obstacle>(other);

                bool isEnemyBullet = false;
                if (registry.HasComponent<CollisionLayer>(bullet)) {
                    const auto& bulletLayer = registry.GetComponent<CollisionLayer>(bullet);
                    isEnemyBullet = (bulletLayer.layer == CollisionLayers::ENEMY_BULLET);
                }

                if (isEnemyBullet && (hitEnemy || hitBoss)) {
                    continue;
                }

                bool shouldApplyDamage = true;
                constexpr float BEAM_DAMAGE_TICK_INTERVAL = 0.1f;
                
                if (isBeam && (hitEnemy || hitBoss)) {
                    uint64_t damageKey = (static_cast<uint64_t>(other) << 32) | static_cast<uint64_t>(bullet);
                    
                    auto damageIt = lastBeamDamageTime.find(damageKey);
                    if (damageIt != lastBeamDamageTime.end()) {
                        float timeSinceLastDamage = totalTime - damageIt->second;
                        if (timeSinceLastDamage < BEAM_DAMAGE_TICK_INTERVAL) {
                            shouldApplyDamage = false;
                        }
                    }
                    
                    if (shouldApplyDamage) {
                        lastBeamDamageTime[damageKey] = totalTime;
                    }
                }

                // Handle enemy damage
                if (hitEnemy && registry.HasComponent<Health>(other) && shouldApplyDamage) {
                    auto& health = registry.GetComponent<Health>(other);
                    const auto& damage = registry.GetComponent<Damage>(bullet);
                    
                    int actualDamage = damage.amount;
                    if (isBeam) {
                        actualDamage = damage.amount / 10;
                    }
                    
                    health.current -= actualDamage;

                    // Create hit effect at collision point
                    if (m_effectFactory && registry.HasComponent<Position>(bullet) && !isBeam) {
                        const auto& bulletPos = registry.GetComponent<Position>(bullet);
                        Entity hitEntity = m_effectFactory->CreateHitEffect(registry, bulletPos.x, bulletPos.y);
                        Core::Logger::Info("[BulletCollision] Created hit effect at ({}, {})", bulletPos.x, bulletPos.y);
                    }

                    if (health.current <= 0) {
                        const auto& enemyComp = registry.GetComponent<Enemy>(other);
                        const auto& bulletComp = registry.GetComponent<Bullet>(bullet);
                        registry.AddComponent<EnemyKilled>(other,
                                                           EnemyKilled(enemyComp.id, bulletComp.owner));
                        if (isBeam) {
                            uint64_t damageKey = (static_cast<uint64_t>(other) << 32) | static_cast<uint64_t>(bullet);
                            lastBeamDamageTime.erase(damageKey);
                        }
                    }
                }

                // Handle boss damage
                if (hitBoss && registry.HasComponent<Health>(other) && shouldApplyDamage) {
                    auto& health = registry.GetComponent<Health>(other);
                    const auto& damage = registry.GetComponent<Damage>(bullet);
                    
                    int actualDamage = damage.amount;
                    if (isBeam) {
                        actualDamage = damage.amount / 10;
                    }
                    
                    health.current -= actualDamage;

                    if (registry.HasComponent<DamageFlash>(other)) {
                        auto& flash = registry.GetComponent<DamageFlash>(other);
                        flash.Trigger();
                    }

                    const auto& bulletComp = registry.GetComponent<Bullet>(bullet);
                    if (bulletComp.owner != NULL_ENTITY && registry.IsEntityAlive(bulletComp.owner)) {
                        if (registry.HasComponent<ScoreValue>(bulletComp.owner)) {
                            auto& score = registry.GetComponent<ScoreValue>(bulletComp.owner);
                            score.points += actualDamage * 5;
                        }
                    }

                    if (health.current <= 0) {
                        health.current = 0;
                    }
                }

                if (hitPlayer && registry.HasComponent<Health>(other)) {
                    if (registry.HasComponent<Shield>(other)) {
                        continue;
                    } else {
                        auto& health = registry.GetComponent<Health>(other);
                        const auto& damage = registry.GetComponent<Damage>(bullet);
                        health.current -= damage.amount;

                        if (health.current < 0) {
                            health.current = 0;
                        }
                    }
                }

                bool shouldDestroy = false;
                if (hitObstacle) {
                    const auto& obstacle = registry.GetComponent<Obstacle>(other);
                    if (obstacle.blocking) {
                        shouldDestroy = true;
                        
                        if (m_effectFactory && registry.HasComponent<Position>(bullet) && !isBeam) {
                            const auto& bulletPos = registry.GetComponent<Position>(bullet);
                            Entity hitEntity = m_effectFactory->CreateHitEffect(registry, bulletPos.x, bulletPos.y);
                            Core::Logger::Info("[BulletCollision] Created hit effect on obstacle at ({}, {})", bulletPos.x, bulletPos.y);
                        }
                    }
                }

                if (isBeam) {
                } else {
                    if (hitEnemy || hitBoss || hitPlayer || shouldDestroy) {
                        registry.DestroyEntity(bullet);
                    }
                }
            }
        }

    }
}
