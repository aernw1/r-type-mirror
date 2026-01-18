/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MineSystem - Handles mine proximity detection and explosions
*/

#include "ECS/MineSystem.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include <cmath>

namespace RType {
    namespace ECS {

        float MineSystem::Distance(float x1, float y1, float x2, float y2) {
            float dx = x2 - x1;
            float dy = y2 - y1;
            return std::sqrt(dx * dx + dy * dy);
        }

        void MineSystem::Update(Registry& registry, float deltaTime) {
            auto mines = registry.GetEntitiesWithComponent<Mine>();

            for (auto mineEntity : mines) {
                if (!registry.IsEntityAlive(mineEntity)) {
                    continue;
                }

                auto& mine = registry.GetComponent<Mine>(mineEntity);
                const auto& minePos = registry.GetComponent<Position>(mineEntity);

                mine.timer += deltaTime;

                if (mine.timer >= mine.lifeTime) {
                    Core::Logger::Debug("[MineSystem] Mine {} expired", static_cast<uint32_t>(mineEntity));
                    registry.DestroyEntity(mineEntity);
                    continue;
                }

                if (mine.isExploding) {
                    mine.explosionTimer += deltaTime;
                    if (mine.explosionTimer >= 0.6f) {
                        Core::Logger::Debug("[MineSystem] Mine {} explosion finished", static_cast<uint32_t>(mineEntity));
                        registry.DestroyEntity(mineEntity);
                    }
                    continue;
                }

                auto players = registry.GetEntitiesWithComponent<Player>();
                for (auto playerEntity : players) {
                    if (!registry.IsEntityAlive(playerEntity)) {
                        continue;
                    }

                    if (!registry.HasComponent<Position>(playerEntity)) {
                        continue;
                    }

                    const auto& playerPos = registry.GetComponent<Position>(playerEntity);
                    float distance = Distance(minePos.x, minePos.y, playerPos.x, playerPos.y);

                    if (distance <= mine.proximityRadius) {
                        mine.isExploding = true;
                        mine.explosionTimer = 0.0f;

                        Core::Logger::Info("[MineSystem] Mine {} triggered by player at distance {}",
                                          static_cast<uint32_t>(mineEntity), distance);

                        for (auto targetPlayer : players) {
                            if (!registry.IsEntityAlive(targetPlayer)) {
                                continue;
                            }

                            if (!registry.HasComponent<Position>(targetPlayer)) {
                                continue;
                            }

                            const auto& targetPos = registry.GetComponent<Position>(targetPlayer);
                            float explosionDist = Distance(minePos.x, minePos.y, targetPos.x, targetPos.y);

                            if (explosionDist <= mine.explosionRadius) {
                                if (registry.HasComponent<Health>(targetPlayer)) {
                                    auto& health = registry.GetComponent<Health>(targetPlayer);
                                    int damage = registry.HasComponent<Damage>(mineEntity) ?
                                                registry.GetComponent<Damage>(mineEntity).amount : 30;
                                    health.current -= damage;

                                    Core::Logger::Info("[MineSystem] Player {} took {} damage from mine explosion",
                                                      static_cast<uint32_t>(targetPlayer), damage);
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }

    }
}
