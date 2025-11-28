#include "ECS/EnemyFactory.hpp"
#include "Core/Logger.hpp"
#include <algorithm>
#include <cmath>

namespace RType {

    namespace ECS {

        Entity EnemyFactory::CreateEnemy(Registry& registry, EnemyType type, float startX, float startY,
                                       Renderer::IRenderer* renderer) {
            Entity enemy = registry.CreateEntity();

            registry.AddComponent<Position>(enemy, Position(startX, startY));

            float speed = GetEnemySpeed(type);
            registry.AddComponent<Velocity>(enemy, Velocity(-speed, 0.0f));

            registry.AddComponent<Enemy>(enemy, Enemy(type, 0));

            int health = GetEnemyHealth(type);
            registry.AddComponent<Health>(enemy, Health(health, health));

            int damage = GetEnemyDamage(type);
            registry.AddComponent<Damage>(enemy, Damage(damage));

            uint32_t score = GetEnemyScore(type);
            registry.AddComponent<ScoreValue>(enemy, ScoreValue(score));

            registry.AddComponent<BoxCollider>(enemy, BoxCollider(50.0f, 50.0f));

            if (renderer) {
                std::string spritePath = GetEnemySpritePath(type);
                Renderer::TextureId textureId = renderer->LoadTexture(spritePath);

                if (textureId == Renderer::INVALID_TEXTURE_ID) {
                    Core::Logger::Warning("Failed to load enemy texture: {}, using default", spritePath);
                    textureId = renderer->LoadTexture("../assets/spaceships/nave2.png");
                }

                if (textureId != Renderer::INVALID_TEXTURE_ID) {
                    Renderer::SpriteId spriteId = renderer->CreateSprite(
                        textureId, Renderer::Rectangle{{0.0f, 0.0f}, {256.0f, 256.0f}});

                    auto& drawable = registry.AddComponent<Drawable>(enemy, Drawable(spriteId, 1));
                    drawable.tint = GetEnemyColor(type);
                    drawable.scale = Math::Vector2(0.5f, 0.5f);
                } else {
                    Core::Logger::Error("Failed to load any enemy texture for type {}", static_cast<int>(type));
                }
            }

            Core::Logger::Info("Created enemy type {} at position ({}, {})", static_cast<int>(type), startX, startY);

            return enemy;
        }

        Math::Color EnemyFactory::GetEnemyColor(EnemyType type) {
            switch (type) {
            case EnemyType::BASIC:
                return Math::Color(1.0f, 1.0f, 1.0f, 1.0f);
            case EnemyType::FAST:
                return Math::Color(1.0f, 0.3f, 0.3f, 1.0f);
            case EnemyType::TANK:
                return Math::Color(0.3f, 0.3f, 1.0f, 1.0f);
            case EnemyType::BOSS:
                return Math::Color(1.0f, 0.0f, 1.0f, 1.0f);
            case EnemyType::FORMATION:
                return Math::Color(0.5f, 0.5f, 0.5f, 1.0f);
            default:
                return Math::Color(1.0f, 1.0f, 1.0f, 1.0f);
            }
        }

        std::string EnemyFactory::GetEnemySpritePath(EnemyType type) {
            switch (type) {
            case EnemyType::BASIC:
                return "../assets/spaceships/nave2.png";
            case EnemyType::FAST:
                return "../assets/spaceships/nave2_red.png";
            case EnemyType::TANK:
                return "../assets/spaceships/nave2_blue.png";
            case EnemyType::BOSS:
                return "../assets/spaceships/nave2.png";
            case EnemyType::FORMATION:
                return "../assets/spaceships/nave2.png";
            default:
                return "../assets/spaceships/nave2.png";
            }
        }

        float EnemyFactory::GetEnemySpeed(EnemyType type) {
            switch (type) {
            case EnemyType::BASIC:
                return 100.0f;
            case EnemyType::FAST:
                return 200.0f;
            case EnemyType::TANK:
                return 50.0f;
            case EnemyType::BOSS:
                return 75.0f;
            case EnemyType::FORMATION:
                return 100.0f;
            default:
                return 100.0f;
            }
        }

        int EnemyFactory::GetEnemyHealth(EnemyType type) {
            switch (type) {
            case EnemyType::BASIC:
                return 100;
            case EnemyType::FAST:
                return 50;
            case EnemyType::TANK:
                return 200;
            case EnemyType::BOSS:
                return 1000;
            case EnemyType::FORMATION:
                return 100;
            default:
                return 100;
            }
        }

        int EnemyFactory::GetEnemyDamage(EnemyType type) {
            switch (type) {
            case EnemyType::BASIC:
                return 10;
            case EnemyType::FAST:
                return 5;
            case EnemyType::TANK:
                return 20;
            case EnemyType::BOSS:
                return 50;
            case EnemyType::FORMATION:
                return 10;
            default:
                return 10;
            }
        }

        uint32_t EnemyFactory::GetEnemyScore(EnemyType type) {
            switch (type) {
            case EnemyType::BASIC:
                return 100;
            case EnemyType::FAST:
                return 150;
            case EnemyType::TANK:
                return 200;
            case EnemyType::BOSS:
                return 1000;
            case EnemyType::FORMATION:
                return 100;
            default:
                return 100;
            }
        }

    }

}


