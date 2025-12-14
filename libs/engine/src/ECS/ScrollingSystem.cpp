/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScrollingSystem
*/

#include "../../include/ECS/ScrollingSystem.hpp"
#include "../../include/ECS/Registry.hpp"
#include "../../include/ECS/Component.hpp"

namespace RType {
    namespace ECS {

        void ScrollingSystem::Update(Registry& registry, float deltaTime) {
            auto scrollables = registry.GetEntitiesWithComponent<Scrollable>();

            for (auto entity : scrollables) {
                if (!registry.IsEntityAlive(entity)) {
                    continue;
                }

                const auto& scrollable = registry.GetComponent<Scrollable>(entity);

                if (!registry.HasComponent<Position>(entity)) {
                    continue;
                }

                auto& pos = registry.GetComponent<Position>(entity);
                pos.x = pos.x + scrollable.speed * deltaTime;

                bool isObstacleCollider = registry.HasComponent<Obstacle>(entity) && 
                                         !registry.HasComponent<Drawable>(entity);
                
                if (!isObstacleCollider) {
                    float destroyThreshold = -1500.0f;
                    if (pos.x < destroyThreshold) {
                        registry.DestroyEntity(entity);
                    }
                } else {
                    float destroyThreshold = -2000.0f;
                    if (pos.x < destroyThreshold) {
                        registry.DestroyEntity(entity);
                    }
                }
            }
        }
    }
}
