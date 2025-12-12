/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ObstacleCollisionResponseSystem implementation
*/

#include "ECS/ObstacleCollisionResponseSystem.hpp"

namespace RType {
    namespace ECS {

        void ObstacleCollisionResponseSystem::Update(Registry& registry, float deltaTime) {
            auto obstacles = registry.GetEntitiesWithComponent<Obstacle>();

            for (auto obstacle : obstacles) {
                if (!registry.IsEntityAlive(obstacle)) {
                    continue;
                }

                if (!registry.HasComponent<CollisionEvent>(obstacle)) {
                    continue;
                }

                auto& event = registry.GetComponent<CollisionEvent>(obstacle);
                Entity other = event.other;

                if (!registry.IsEntityAlive(other)) {
                    continue;
                }

                // TODO: implement obstacle collision response logic here
            }
        }

    }
}
