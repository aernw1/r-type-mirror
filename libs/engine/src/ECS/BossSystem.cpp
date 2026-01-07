/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BossSystem - Handles boss behavior
*/

#include "ECS/BossSystem.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"

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
            }
        }

    }
}
