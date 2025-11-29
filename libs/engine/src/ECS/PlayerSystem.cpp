#include "ECS/PlayerSystem.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <string>

namespace RType {

    namespace ECS {

        PlayerSystem::PlayerSystem(Renderer::IRenderer* renderer)
            : m_renderer(renderer) {}

        void PlayerSystem::Update(Registry& registry, float /* deltaTime */) {
            auto players = registry.GetEntitiesWithComponent<Player>();

            for (Entity player : players) {
                if (!registry.HasComponent<Position>(player)) {
                    continue;
                }

                ClampPlayerToScreen(registry, player, 1280.0f, 720.0f);
            }
        }

        Entity PlayerSystem::CreatePlayer(Registry& registry, uint8_t playerNumber, uint64_t playerHash,
                                        float startX, float startY, Renderer::IRenderer* renderer) {
            Entity player = registry.CreateEntity();

            float yPos = startY + (playerNumber - 1) * 150.0f;

            registry.AddComponent<Position>(player, Position(startX, yPos));

            registry.AddComponent<Velocity>(player, Velocity(0.0f, 0.0f));

            registry.AddComponent<Player>(player, Player(playerNumber, playerHash, false));

            registry.AddComponent<Controllable>(player, Controllable(200.0f));

            registry.AddComponent<ScoreValue>(player, ScoreValue(0));

            registry.AddComponent<Shooter>(player, Shooter(0.2f));

            registry.AddComponent<Health>(player, Health(100, 100));

            registry.AddComponent<BoxCollider>(player, BoxCollider(50.0f, 50.0f));

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
                } else {
                    Core::Logger::Error("Failed to load any player texture for player {}", playerNumber);
                }
            }

            Core::Logger::Info("Created player #{} at position ({}, {})", playerNumber, startX, yPos);

            return player;
        }

        void PlayerSystem::ClampPlayerToScreen(Registry& registry, Entity player, float screenWidth,
                                             float screenHeight) {
            if (!registry.HasComponent<Position>(player)) {
                return;
            }

            auto& pos = registry.GetComponent<Position>(player);

            float minX = 0.0f;
            float maxX = screenWidth;
            float minY = 0.0f;
            float maxY = screenHeight;

            if (registry.HasComponent<BoxCollider>(player)) {
                const auto& collider = registry.GetComponent<BoxCollider>(player);
                maxX -= collider.width;
                maxY -= collider.height;
            }

            pos.x = std::clamp(pos.x, minX, maxX);
            pos.y = std::clamp(pos.y, minY, maxY);
        }

        Math::Color PlayerSystem::GetPlayerColor(uint8_t playerNumber) {
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

        std::string PlayerSystem::GetPlayerSpritePath(uint8_t playerNumber) {
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

