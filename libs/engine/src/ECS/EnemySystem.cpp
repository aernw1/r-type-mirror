#include "ECS/EnemySystem.hpp"
#include "ECS/EnemyFactory.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include <algorithm>
#include <random>
#include <cmath>

namespace RType {

    namespace ECS {

        constexpr float ENEMY_SPAWN_OFFSET_X = 50.0f;
        constexpr float ENEMY_SPAWN_MARGIN_Y = 50.0f;
        constexpr float ENEMY_DESTROY_OFFSET_X = -100.0f;
        constexpr float ENEMY_FAST_SIN_AMPLITUDE = 50.0f;
        constexpr float ENEMY_FAST_SIN_FREQUENCY = 0.01f;
        constexpr float ENEMY_SPAWN_MIN_Y = 50.0f;

        EnemySystem::EnemySystem(Renderer::IRenderer* renderer, float screenWidth, float screenHeight)
            : m_renderer(renderer), m_screenWidth(screenWidth), m_screenHeight(screenHeight),
              m_rng(std::random_device{}()) {}

        void EnemySystem::Update(Registry& registry, float deltaTime) {
            m_spawnTimer += deltaTime;

            if (m_spawnTimer >= m_spawnInterval) {
                SpawnRandomEnemy(registry);
                m_spawnTimer = 0.0f;
            }

            auto enemies = registry.GetEntitiesWithComponent<Enemy>();

            for (Entity enemy : enemies) {
                if (!registry.HasComponent<Position>(enemy)) {
                    continue;
                }

                ApplyMovementPattern(registry, enemy, deltaTime);
            }

            DestroyEnemiesOffScreen(registry, m_screenWidth);
        }

        void EnemySystem::SpawnRandomEnemy(Registry& registry) {
            std::uniform_real_distribution<float> yDist(ENEMY_SPAWN_MIN_Y, m_screenHeight - ENEMY_SPAWN_MARGIN_Y);
            std::uniform_int_distribution<int> typeDist(0, 2);

            float spawnX = m_screenWidth + ENEMY_SPAWN_OFFSET_X;
            float spawnY = yDist(m_rng);

            EnemyType type = static_cast<EnemyType>(typeDist(m_rng));

            EnemyFactory::CreateEnemy(registry, type, spawnX, spawnY, m_renderer);
        }

        void EnemySystem::DestroyEnemiesOffScreen(Registry& registry, float /* screenWidth */) {
            auto enemies = registry.GetEntitiesWithComponent<Enemy>();

            for (Entity enemy : enemies) {
                if (!registry.HasComponent<Position>(enemy)) {
                    continue;
                }

                const auto& pos = registry.GetComponent<Position>(enemy);

                if (pos.x < ENEMY_DESTROY_OFFSET_X) {
                    registry.DestroyEntity(enemy);
                }
            }
        }

        void EnemySystem::ApplyMovementPattern(Registry& registry, Entity enemy, float deltaTime) {
            if (!registry.HasComponent<Enemy>(enemy) || !registry.HasComponent<Velocity>(enemy)) {
                return;
            }

            const auto& enemyComp = registry.GetComponent<Enemy>(enemy);
            auto& velocity = registry.GetComponent<Velocity>(enemy);

            switch (enemyComp.type) {
            case EnemyType::BASIC:
                velocity.dx = -EnemyFactory::GetEnemySpeed(EnemyType::BASIC);
                velocity.dy = 0.0f;
                break;

            case EnemyType::FAST: {
                if (registry.HasComponent<Position>(enemy)) {
                    const auto& pos = registry.GetComponent<Position>(enemy);
                    float speed = EnemyFactory::GetEnemySpeed(EnemyType::FAST);
                    velocity.dx = -speed;
                    velocity.dy = std::sin(pos.x * ENEMY_FAST_SIN_FREQUENCY) * ENEMY_FAST_SIN_AMPLITUDE;
                }
                break;
            }

            case EnemyType::TANK:
                velocity.dx = -EnemyFactory::GetEnemySpeed(EnemyType::TANK);
                velocity.dy = 0.0f;
                break;

            case EnemyType::BOSS:
                velocity.dx = -EnemyFactory::GetEnemySpeed(EnemyType::BOSS);
                velocity.dy = 0.0f;
                break;

            case EnemyType::FORMATION:
                velocity.dx = -EnemyFactory::GetEnemySpeed(EnemyType::FORMATION);
                velocity.dy = 0.0f;
                break;

            default:
                break;
            }
        }

    }

}
