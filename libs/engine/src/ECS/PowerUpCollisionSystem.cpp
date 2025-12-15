/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PowerUpCollisionSystem
*/

#include "ECS/PowerUpCollisionSystem.hpp"
#include "ECS/PowerUpFactory.hpp"
#include "ECS/CollisionDetectionSystem.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"

namespace RType {
    namespace ECS {

        PowerUpCollisionSystem::PowerUpCollisionSystem(Renderer::IRenderer* renderer)
            : m_renderer(renderer) {}

        void PowerUpCollisionSystem::Update(Registry& registry, float deltaTime) {
            (void)deltaTime;

            auto powerups = registry.GetEntitiesWithComponent<PowerUp>();
            auto players = registry.GetEntitiesWithComponent<Player>();

            if (powerups.empty() || players.empty()) {
                return;
            }

            std::vector<Entity> powerupsToDestroy;

            for (Entity powerup : powerups) {
                if (!registry.IsEntityAlive(powerup)) {
                    continue;
                }

                for (Entity player : players) {
                    if (!registry.IsEntityAlive(player)) {
                        continue;
                    }

                    if (CollisionDetectionSystem::CheckCollision(registry, powerup, player)) {
                        const auto& powerupComp = registry.GetComponent<PowerUp>(powerup);

                        PowerUpFactory::ApplyPowerUpToPlayer(
                            registry,
                            player,
                            powerupComp.type,
                            m_renderer
                        );

                        Core::Logger::Info(
                            "[PowerUpCollisionSystem] Player collected powerup: {}",
                            PowerUpFactory::GetPowerUpName(powerupComp.type)
                        );

                        powerupsToDestroy.push_back(powerup);
                        break;
                    }
                }
            }

            // Destroy collected powerups
            for (Entity powerup : powerupsToDestroy) {
                registry.DestroyEntity(powerup);
            }
        }

    }
}
