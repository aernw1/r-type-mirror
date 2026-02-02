/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ShieldSystem
*/

#include "ShieldSystem.hpp"
#include "ECS/Component.hpp"

namespace RType {
    namespace ECS {

        void ShieldSystem::Update(Registry& registry, float deltaTime) {
            auto shields = registry.GetEntitiesWithComponent<Shield>();

            std::vector<Entity> expiredShields;

            for (Entity entity : shields) {
                if (!registry.IsEntityAlive(entity)) {
                    continue;
                }

                auto& shield = registry.GetComponent<Shield>(entity);

                if (shield.duration > 0.0f) {
                    shield.timeRemaining -= deltaTime;

                    if (shield.timeRemaining <= 0.0f) {
                        expiredShields.push_back(entity);
                    }
                }

                if (registry.HasComponent<Drawable>(entity)) {
                    auto& drawable = registry.GetComponent<Drawable>(entity);
                    drawable.tint = Math::Color(0.7f, 0.7f, 1.0f, 1.0f);
                }
            }

            for (Entity entity : expiredShields) {
                registry.RemoveComponent<Shield>(entity);

                if (registry.HasComponent<ActivePowerUps>(entity)) {
                    auto& active = registry.GetComponent<ActivePowerUps>(entity);
                    active.hasShield = false;
                }

                if (registry.HasComponent<Drawable>(entity)) {
                    auto& drawable = registry.GetComponent<Drawable>(entity);
                    drawable.tint = Math::Color(1.0f, 1.0f, 1.0f, 1.0f);
                }
            }

            auto players = registry.GetEntitiesWithComponent<Player>();
            for (Entity entity : players) {
                if (!registry.IsEntityAlive(entity)) {
                    continue;
                }

                if (!registry.HasComponent<Shield>(entity) && registry.HasComponent<Drawable>(entity)) {
                    auto& drawable = registry.GetComponent<Drawable>(entity);
                    if (registry.HasComponent<ActivePowerUps>(entity)) {
                        const auto& active = registry.GetComponent<ActivePowerUps>(entity);
                        if (!active.hasShield && drawable.tint.r < 0.8f && drawable.tint.b > 0.8f) {
                            drawable.tint = Math::Color(1.0f, 1.0f, 1.0f, 1.0f);
                        }
                    } else if (drawable.tint.r < 0.8f && drawable.tint.b > 0.8f) {
                        drawable.tint = Math::Color(1.0f, 1.0f, 1.0f, 1.0f);
                    }
                }
            }
        }

    }
}
