/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ShieldSystem
*/

#include "ECS/ShieldSystem.hpp"
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

            // Remove expired shields
            for (Entity entity : expiredShields) {
                registry.RemoveComponent<Shield>(entity);

                // Reset tint if entity still has drawable
                if (registry.HasComponent<Drawable>(entity)) {
                    auto& drawable = registry.GetComponent<Drawable>(entity);
                    drawable.tint = Math::Color(1.0f, 1.0f, 1.0f, 1.0f);
                }
            }
        }

    }
}
