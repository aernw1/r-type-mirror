/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScrollingSystem
*/

#include "../../include/ECS/ScrollingSystem.hpp"
#include "../../include/ECS/Registry.hpp"
#include "../../include/ECS/Component.hpp"
#include <iostream>

namespace RType {
    namespace ECS {

        void ScrollingSystem::Update(Registry& registry, float deltaTime) {
            auto scrollables = registry.GetEntitiesWithComponent<Scrollable>();

            static bool loggedScrollingSystem = false;
            if (!loggedScrollingSystem) {
                std::cout << "[ScrollingSystem] Two-pass update running" << std::endl;
                loggedScrollingSystem = true;
            }

            // First pass: scroll visual entities (obstacles, backgrounds, etc.)
            for (auto entity : scrollables) {
                if (!registry.IsEntityAlive(entity)) {
                    continue;
                }

                // Skip obstacle colliders in first pass (they'll be synced in second pass)
                bool isObstacleCollider = registry.HasComponent<Obstacle>(entity) &&
                                          !registry.HasComponent<Drawable>(entity);
                if (isObstacleCollider) {
                    continue;
                }

                const auto& scrollable = registry.GetComponent<Scrollable>(entity);

                if (!registry.HasComponent<Position>(entity)) {
                    continue;
                }

                auto& pos = registry.GetComponent<Position>(entity);
                pos.x = pos.x + scrollable.speed * deltaTime;

                float destroyThreshold = -1500.0f;
                if (pos.x < destroyThreshold) {
                    registry.DestroyEntity(entity);
                }
            }

            // Second pass: synchronize obstacle colliders to their visual entities
            static int serverSyncDebug = 0;
            for (auto entity : scrollables) {
                if (!registry.IsEntityAlive(entity)) {
                    continue;
                }

                bool isObstacleCollider = registry.HasComponent<Obstacle>(entity) &&
                                          !registry.HasComponent<Drawable>(entity);

                if (!isObstacleCollider) {
                    continue;
                }

                if (!registry.HasComponent<Position>(entity) ||
                    !registry.HasComponent<ObstacleMetadata>(entity)) {
                    continue;
                }

                const auto& metadata = registry.GetComponent<ObstacleMetadata>(entity);
                auto& colliderPos = registry.GetComponent<Position>(entity);

                if (metadata.visualEntity != NULL_ENTITY &&
                    registry.IsEntityAlive(metadata.visualEntity) &&
                    registry.HasComponent<Position>(metadata.visualEntity) &&
                    registry.HasComponent<ObstacleVisual>(metadata.visualEntity)) {

                    const auto& visualPos = registry.GetComponent<Position>(metadata.visualEntity);

                    // Debug first few server syncs
                    if (serverSyncDebug < 3) {
                        std::cout << "[ScrollingSystem SYNC] Collider " << entity
                                  << ": visual=(" << visualPos.x << "," << visualPos.y << ")"
                                  << " offset=(" << metadata.offsetX << "," << metadata.offsetY << ")"
                                  << " -> collider=(" << visualPos.x + metadata.offsetX << ","
                                  << visualPos.y + metadata.offsetY << ")" << std::endl;
                        serverSyncDebug++;
                    }

                    // Collider position = visual position + stored offset
                    colliderPos.x = visualPos.x + metadata.offsetX;
                    colliderPos.y = visualPos.y + metadata.offsetY;
                }
                else if (metadata.visualEntity != NULL_ENTITY &&
                         registry.IsEntityAlive(metadata.visualEntity) &&
                         !registry.HasComponent<ObstacleVisual>(metadata.visualEntity)) {
                    registry.DestroyEntity(entity);
                    continue;
                }
                else if (registry.HasComponent<Scrollable>(entity)) {
                    const auto& scrollable = registry.GetComponent<Scrollable>(entity);
                    colliderPos.x = colliderPos.x + scrollable.speed * deltaTime;
                }

                // Destroy threshold for obstacle colliders
                float destroyThreshold = -2000.0f;
                if (colliderPos.x < destroyThreshold) {
                    registry.DestroyEntity(entity);
                }
            }
        }
    }
}
