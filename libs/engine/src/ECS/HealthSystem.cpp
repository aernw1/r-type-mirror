#include "ECS/HealthSystem.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"

namespace RType {

    namespace ECS {

        void HealthSystem::Update(Registry& registry, float deltaTime) {
            (void)deltaTime;

            checkAndDestroyDeadEntities(registry);
        }

        void HealthSystem::checkAndDestroyDeadEntities(Registry& registry) {
            auto entitiesWithHealth = registry.GetEntitiesWithComponent<Health>();

            for (Entity entity : entitiesWithHealth) {
                if (!registry.IsEntityAlive(entity)) {
                    continue;
                }

                if (!registry.HasComponent<Health>(entity)) {
                    continue;
                }

                const auto& health = registry.GetComponent<Health>(entity);

                if (health.current <= 0) {
                    registry.DestroyEntity(entity);
                }
            }
        }

    }

}
