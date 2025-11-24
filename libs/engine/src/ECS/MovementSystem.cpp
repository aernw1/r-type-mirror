#include "ECS/MovementSystem.hpp"
#include "ECS/Component.hpp"

namespace RType {

    namespace ECS {

        void MovementSystem::Update(Registry& registry, float deltaTime) {
            auto entities = registry.GetEntitiesWithComponent<Velocity>();

            for (Entity entity : entities) {
                if (!registry.HasComponent<Position>(entity)) {
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
