#include "ECS/PlayerFactory.hpp"
#include "Core/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <string>

namespace RType {

    namespace ECS {

        Entity PlayerFactory::CreatePlayer(Registry& registry, uint8_t playerNumber, uint64_t playerHash,
                                           float startX, float startY, Renderer::IRenderer* renderer) {
            Entity player = registry.CreateEntity();

            float yPos = startY + (playerNumber - 1) * 150.0f;

            registry.AddComponent<Position>(player, Position(startX, yPos));

            registry.AddComponent<Velocity>(player, Velocity(0.0f, 0.0f));

            registry.AddComponent<Player>(player, Player(playerNumber, playerHash, false));

            registry.AddComponent<Controllable>(player, Controllable(200.0f));

            registry.AddComponent<Health>(player, Health(300, 300));
            registry.AddComponent<ScoreValue>(player, ScoreValue(0));
            registry.AddComponent<ScoreTimer>(player, ScoreTimer(0.0f));
            registry.AddComponent<Shooter>(player, Shooter(0.2f, 50.0f, 0.0f));
            registry.AddComponent<ShootCommand>(player, ShootCommand());
            registry.AddComponent<BoxCollider>(player, BoxCollider(25.0f, 25.0f));
            registry.AddComponent<CircleCollider>(player, CircleCollider(12.5f));
            registry.AddComponent<CollisionLayer>(player,
                                                  CollisionLayer(CollisionLayers::PLAYER,
                                                                 CollisionLayers::ENEMY | CollisionLayers::ENEMY_BULLET | CollisionLayers::OBSTACLE));

            if (renderer) {
                std::string spritePath = GetPlayerSpritePath(playerNumber);
                Renderer::TextureId textureId = renderer->LoadTexture(spritePath);

                if (textureId == Renderer::INVALID_TEXTURE_ID) {
                    Core::Logger::Warning("Failed to load player texture: {}, using default", spritePath);
                    textureId = renderer->LoadTexture("../assets/spaceships/nave2.png");
                }

                if (textureId != Renderer::INVALID_TEXTURE_ID) {
                    Renderer::SpriteId spriteId = renderer->CreateSprite(
                        textureId, Renderer::Rectangle{{0.0f, 0.0f}, {256.0f, 256.0f}});

                    auto& drawable = registry.AddComponent<Drawable>(player, Drawable(spriteId, 2));
                    drawable.tint = GetPlayerColor(playerNumber);
                    drawable.scale = Math::Vector2(0.5f, 0.5f);
                    drawable.origin = Math::Vector2(128.0f, 128.0f);
                } else {
                    Core::Logger::Error("Failed to load any player texture for player {}", playerNumber);
                }
            }

            Core::Logger::Info("Created player #{} at position ({}, {})", playerNumber, startX, yPos);

            return player;
        }

        Math::Color PlayerFactory::GetPlayerColor(uint8_t playerNumber) {
            switch (playerNumber) {
            case 1:
                return Math::Color(0.2f, 0.6f, 1.0f, 1.0f);
            case 2:
                return Math::Color(1.0f, 0.2f, 0.2f, 1.0f);
            case 3:
                return Math::Color(0.2f, 1.0f, 0.2f, 1.0f);
            case 4:
                return Math::Color(1.0f, 0.8f, 0.2f, 1.0f);
            default:
                return Math::Color(1.0f, 1.0f, 1.0f, 1.0f);
            }
        }

        std::string PlayerFactory::GetPlayerSpritePath(uint8_t playerNumber) {
            switch (playerNumber) {
            case 1:
                return "../assets/spaceships/player_blue.png";
            case 2:
                return "../assets/spaceships/player_red.png";
            case 3:
                return "../assets/spaceships/player_green.png";
            case 4:
                return "../assets/spaceships/nave2.png";
            default:
                return "../assets/spaceships/nave2.png";
            }
        }

    }

}
