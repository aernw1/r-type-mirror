#include "ECS/MovementSystem.hpp"
#include "ECS/Component.hpp"
#include <iostream>

namespace RType {

    namespace ECS {

        void MovementSystem::Update(Registry& registry, float deltaTime) {
            auto entities = registry.GetEntitiesWithComponent<Velocity>();

            for (Entity entity : entities) {
                if (!registry.HasComponent<Position>(entity)) {
                    continue;
                }

                // CRITICAL FIX: Skip obstacles - they should NEVER move!
                if (registry.HasComponent<Obstacle>(entity)) {
                    static int obstacleVelocityLog = 0;
                    if (obstacleVelocityLog < 10) {
                        std::cerr << "[MOVEMENT BUG] Obstacle entity " << entity
                                  << " has Velocity component! Removing it." << std::endl;
                        obstacleVelocityLog++;
                    }
                    registry.RemoveComponent<Velocity>(entity);
                    continue;
                }

                auto& position = registry.GetComponent<Position>(entity);
                const auto& velocity = registry.GetComponent<Velocity>(entity);

                position.x += velocity.dx * deltaTime;
                position.y += velocity.dy * deltaTime;
            }
        }
    }

}
