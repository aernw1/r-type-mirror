/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BossSystem - Handles boss behavior
*/

#include "BossSystem.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace RType {
    namespace ECS {


        void StopScrollingAndFixPosition(Registry& registry, Entity entity, float fixedX) {
            if (!registry.HasComponent<Scrollable>(entity)) {
                return;
            }

            // Remove scrollable component to stop movement
            registry.RemoveComponent<Scrollable>(entity);

            // Fix position
            if (registry.HasComponent<Position>(entity)) {
                auto& pos = registry.GetComponent<Position>(entity);
                pos.x = fixedX;
                Core::Logger::Info("[BossSystem] Entity {} stopped scrolling, fixed at x={}",
                                  static_cast<uint32_t>(entity), fixedX);
            }
        }

        bool HasEnteredScreen(Registry& registry, Entity entity, float screenWidth) {
            if (!registry.HasComponent<Position>(entity)) {
                return false;
            }

            const auto& pos = registry.GetComponent<Position>(entity);
            return pos.x < screenWidth;
        }

        void BossSystem::Update(Registry& registry, float deltaTime) {
            // Process all boss entities
            auto bosses = registry.GetEntitiesWithComponent<Boss>();

            for (auto bossEntity : bosses) {
                if (!registry.IsEntityAlive(bossEntity)) {
                    continue;
                }

                // If boss has entered screen and is still scrolling, stop it
                if (registry.HasComponent<Scrollable>(bossEntity) &&
                    HasEnteredScreen(registry, bossEntity, 1920.0f)) {
                    StopScrollingAndFixPosition(registry, bossEntity, 900.0f);
                }

                if (registry.HasComponent<BossMovementPattern>(bossEntity) &&
                    !registry.HasComponent<Scrollable>(bossEntity) &&
                    registry.HasComponent<Position>(bossEntity)) {

                    auto& movement = registry.GetComponent<BossMovementPattern>(bossEntity);
                    auto& pos = registry.GetComponent<Position>(bossEntity);

                    movement.timer += deltaTime;

                    const float pi = 3.14159265358979323846f;

                    // Vertical movement
                    float timeY = movement.timer * movement.frequencyY * 2.0f * pi;
                    float newY = movement.centerY + movement.amplitudeY * std::sin(static_cast<double>(timeY));

                    // Horizontal oscillation
                    float timeX = movement.timer * movement.frequencyX * 2.0f * pi;
                    float newX = movement.centerX + movement.amplitudeX * std::sin(static_cast<double>(timeX * 2.0));

                    const float minY = 100.0f;
                    const float maxY = 620.0f;
                    const float minX = 800.0f;
                    const float maxX = 1000.0f;

                    pos.y = std::max(minY, std::min(maxY, newY));
                    pos.x = std::max(minX, std::min(maxX, newX));
                }

                if (registry.HasComponent<DamageFlash>(bossEntity)) {
                    auto& flash = registry.GetComponent<DamageFlash>(bossEntity);
                    if (flash.isActive) {
                        flash.timeRemaining -= deltaTime;
                        if (flash.timeRemaining <= 0.0f) {
                            flash.isActive = false;
                            flash.timeRemaining = 0.0f;
                        }
                    }
                }
            }
        }

    }
}
