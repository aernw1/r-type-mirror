#include "ECS/HealthSystem.hpp"
#include "ECS/Component.hpp"
#include "ECS/CollisionSystem.hpp"
#include "Core/Logger.hpp"

namespace RType {

    namespace ECS {

        void HealthSystem::Update(Registry& registry, float deltaTime) {
            (void)deltaTime;

            handleEnemyPlayerCollisions(registry);
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

        void HealthSystem::handleEnemyPlayerCollisions(Registry& registry) {
            auto enemies = registry.GetEntitiesWithComponent<Enemy>();
            auto players = registry.GetEntitiesWithComponent<Player>();

            if (enemies.empty() || players.empty()) {
                return;
            }

            for (Entity enemy : enemies) {
                if (!registry.IsEntityAlive(enemy)) {
                    continue;
                }

                if (!registry.HasComponent<Position>(enemy) ||
                    !registry.HasComponent<BoxCollider>(enemy) ||
                    !registry.HasComponent<Damage>(enemy)) {
                    continue;
                }

                for (Entity player : players) {
                    if (!registry.IsEntityAlive(player)) {
                        continue;
                    }

                    if (!registry.HasComponent<Position>(player) ||
                        !registry.HasComponent<BoxCollider>(player) ||
                        !registry.HasComponent<Health>(player)) {
                        continue;
                    }

                    if (CollisionSystem::CheckCollision(registry, enemy, player)) {
                        const auto& damageComp = registry.GetComponent<Damage>(enemy);
                        auto& playerHealth = registry.GetComponent<Health>(player);

                        playerHealth.current -= damageComp.amount;

                        if (playerHealth.current < 0) {
                            playerHealth.current = 0;
                        }

                        registry.DestroyEntity(enemy);
                        break;
                    }
                }
            }
        }

    }

}

