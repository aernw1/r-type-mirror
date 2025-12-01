#include "ECS/EnemyFactory.hpp"
#include "Core/Logger.hpp"
#include <array>
#include <atomic>
#include <string>

namespace RType {

    namespace ECS {

        namespace {

            std::atomic<uint32_t> s_enemyIdCounter{1};

            struct EnemyData {
                Math::Color color;
                const char* spritePath;
                float speed;
                int health;
                int damage;
                uint32_t score;
            };

            const std::array<EnemyData, 5> ENEMY_DATA_TABLE = {{// BASIC
                                                                {Math::Color(1.0f, 1.0f, 1.0f, 1.0f), "../assets/spaceships/nave2.png", 100.0f, 100, 10, 100},
                                                                // FAST
                                                                {Math::Color(1.0f, 0.3f, 0.3f, 1.0f), "../assets/spaceships/nave2_red.png", 200.0f, 50, 5, 150},
                                                                // TANK
                                                                {Math::Color(0.3f, 0.3f, 1.0f, 1.0f), "../assets/spaceships/nave2_blue.png", 50.0f, 200, 20, 200},
                                                                // BOSS
                                                                {Math::Color(1.0f, 0.0f, 1.0f, 1.0f), "../assets/spaceships/nave2.png", 75.0f, 1000, 50, 1000},
                                                                // FORMATION
                                                                {Math::Color(0.5f, 0.5f, 0.5f, 1.0f), "../assets/spaceships/nave2.png", 100.0f, 100, 10, 100}}};

            const EnemyData& GetEnemyData(EnemyType type) {
                size_t index = static_cast<size_t>(type);
                if (index >= ENEMY_DATA_TABLE.size()) {
                    return ENEMY_DATA_TABLE[0];
                }
                return ENEMY_DATA_TABLE[index];
            }
        }

        Entity EnemyFactory::CreateEnemy(Registry& registry, EnemyType type, float startX, float startY,
                                         Renderer::IRenderer* renderer) {
            Entity enemy = registry.CreateEntity();

            registry.AddComponent<Position>(enemy, Position(startX, startY));

            const EnemyData& data = GetEnemyData(type);
            registry.AddComponent<Velocity>(enemy, Velocity(-data.speed, 0.0f));

            uint32_t uniqueId = s_enemyIdCounter.fetch_add(1);
            registry.AddComponent<Enemy>(enemy, Enemy(type, uniqueId));

            registry.AddComponent<Health>(enemy, Health(data.health, data.health));
            registry.AddComponent<Damage>(enemy, Damage(data.damage));
            registry.AddComponent<ScoreValue>(enemy, ScoreValue(data.score));
            registry.AddComponent<BoxCollider>(enemy, BoxCollider(50.0f, 50.0f));

            if (renderer) {
                std::string spritePath(data.spritePath);
                Renderer::TextureId textureId = renderer->LoadTexture(spritePath);

                if (textureId == Renderer::INVALID_TEXTURE_ID) {
                    Core::Logger::Warning("Failed to load enemy texture: {}, using default", spritePath);
                    textureId = renderer->LoadTexture("../assets/spaceships/nave2.png");
                }

                if (textureId != Renderer::INVALID_TEXTURE_ID) {
                    Renderer::SpriteId spriteId =
                        renderer->CreateSprite(textureId, Renderer::Rectangle{{0.0f, 0.0f}, {256.0f, 256.0f}});

                    auto& drawable = registry.AddComponent<Drawable>(enemy, Drawable(spriteId, 1));
                    drawable.tint = data.color;
                    drawable.scale = Math::Vector2(0.5f, 0.5f);
                } else {
                    Core::Logger::Error("Failed to load any enemy texture for type {}", static_cast<int>(type));
                }
            }

            Core::Logger::Info("Created enemy type {} at position ({}, {})", static_cast<int>(type), startX, startY);

            return enemy;
        }

        float EnemyFactory::GetEnemySpeed(EnemyType type) {
            return GetEnemyData(type).speed;
        }

        int EnemyFactory::GetEnemyHealth(EnemyType type) {
            return GetEnemyData(type).health;
        }

        int EnemyFactory::GetEnemyDamage(EnemyType type) {
            return GetEnemyData(type).damage;
        }

        uint32_t EnemyFactory::GetEnemyScore(EnemyType type) {
            return GetEnemyData(type).score;
        }

    }

}
