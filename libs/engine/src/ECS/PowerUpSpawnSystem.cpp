/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PowerUpSpawnSystem
*/

#include "ECS/PowerUpSpawnSystem.hpp"
#include "ECS/PowerUpFactory.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include <chrono>

namespace RType {
    namespace ECS {

        PowerUpSpawnSystem::PowerUpSpawnSystem(
            Renderer::IRenderer* renderer,
            float screenWidth,
            float screenHeight
        )
            : m_renderer(renderer),
              m_screenWidth(screenWidth),
              m_screenHeight(screenHeight),
              m_rng(std::chrono::steady_clock::now().time_since_epoch().count())
        {
        }

        void PowerUpSpawnSystem::Update(Registry& registry, float deltaTime) {
            m_spawnTimer += deltaTime;

            if (m_spawnTimer >= m_spawnInterval) {
                SpawnRandomPowerUp(registry);
                m_spawnTimer = 0.0f;
            }

            DestroyPowerUpsOffScreen(registry);
        }

        void PowerUpSpawnSystem::SpawnRandomPowerUp(Registry& registry) {
            // Random powerup type (0-5)
            std::uniform_int_distribution<int> typeDist(0, 5);
            PowerUpType type = static_cast<PowerUpType>(typeDist(m_rng));

            // Random Y position (50 to screenHeight - 50)
            std::uniform_real_distribution<float> yDist(50.0f, m_screenHeight - 50.0f);
            float spawnY = yDist(m_rng);

            // Spawn at right edge of screen
            float spawnX = m_screenWidth + 50.0f;

            PowerUpFactory::CreatePowerUp(
                registry,
                type,
                spawnX,
                spawnY,
                m_renderer
            );

            Core::Logger::Info("[PowerUpSpawnSystem] Spawned {} powerup at ({}, {})",
                               PowerUpFactory::GetPowerUpName(type), spawnX, spawnY);
        }

        void PowerUpSpawnSystem::DestroyPowerUpsOffScreen(Registry& registry) {
            auto powerups = registry.GetEntitiesWithComponent<PowerUp>();

            for (Entity powerup : powerups) {
                if (!registry.IsEntityAlive(powerup)) {
                    continue;
                }

                if (registry.HasComponent<Position>(powerup)) {
                    const auto& pos = registry.GetComponent<Position>(powerup);
                    if (pos.x < -100.0f) {
                        registry.DestroyEntity(powerup);
                    }
                }
            }
        }

    }
}
