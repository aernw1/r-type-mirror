/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PowerUpFactory
*/

#include "ECS/PowerUpFactory.hpp"
#include "Core/Logger.hpp"
#include <array>
#include <atomic>
#include <cmath>

namespace RType {
    namespace ECS {

        namespace {
            std::atomic<uint32_t> s_powerupIdCounter{1};

            struct PowerUpData {
                Math::Color color;
                const char* spritePath;
                const char* name;
            };

            const std::array<PowerUpData, 6> POWERUP_DATA_TABLE = {{
                // FIRE_RATE_BOOST - Red
                {Math::Color(1.0f, 0.2f, 0.2f, 1.0f),
                 "../assets/powerups/spread.png", "Fire Rate Boost"},
                // SPREAD_SHOT - Yellow
                {Math::Color(1.0f, 1.0f, 0.2f, 1.0f),
                 "../assets/powerups/spread.png", "Spread Shot"},
                // LASER_BEAM - Cyan
                {Math::Color(0.2f, 1.0f, 1.0f, 1.0f),
                 "../assets/powerups/laser.png", "Laser Beam"},
                // FORCE_POD - Orange (iconic R-Type color)
                {Math::Color(1.0f, 0.6f, 0.0f, 1.0f),
                 "../assets/powerups/force_pod.png", "Force Pod"},
                // SPEED_BOOST - Green
                {Math::Color(0.2f, 1.0f, 0.2f, 1.0f),
                 "../assets/powerups/speed.png", "Speed Boost"},
                // SHIELD - Blue
                {Math::Color(0.4f, 0.4f, 1.0f, 1.0f),
                 "../assets/powerups/shield.png", "Shield"}
            }};
        }

        Entity PowerUpFactory::CreatePowerUp(
            Registry& registry,
            PowerUpType type,
            float startX, float startY,
            Renderer::IRenderer* renderer
        ) {
            Entity powerup = registry.CreateEntity();

            registry.AddComponent<Position>(powerup, Position(startX, startY));
            registry.AddComponent<Velocity>(powerup, Velocity(-50.0f, 0.0f));

            uint32_t uniqueId = s_powerupIdCounter.fetch_add(1);
            registry.AddComponent<PowerUp>(powerup, PowerUp(type, uniqueId));

            registry.AddComponent<BoxCollider>(powerup, BoxCollider(32.0f, 32.0f));
            registry.AddComponent<CollisionLayer>(powerup,
                CollisionLayer(CollisionLayers::POWERUP,
                               CollisionLayers::PLAYER));

            if (renderer) {
                const PowerUpData& data = POWERUP_DATA_TABLE[static_cast<size_t>(type)];

                Renderer::TextureId textureId = renderer->LoadTexture(data.spritePath);
                if (textureId == Renderer::INVALID_TEXTURE_ID) {
                    textureId = renderer->LoadTexture("../assets/powerups/laser.png");
                }

                if (textureId != Renderer::INVALID_TEXTURE_ID) {
                    Renderer::SpriteId spriteId = renderer->CreateSprite(
                        textureId,
                        Renderer::Rectangle{{0.0f, 0.0f}, {64.0f, 64.0f}}
                    );

                    auto& drawable = registry.AddComponent<Drawable>(powerup, Drawable(spriteId, 5));
                    drawable.tint = data.color;
                    drawable.scale = Math::Vector2(2.5f, 2.5f);
                    registry.AddComponent<PowerUpGlow>(powerup);
                }
            }

            return powerup;
        }

        void PowerUpFactory::ApplyPowerUpToPlayer(
            Registry& registry,
            Entity player,
            PowerUpType type,
            Renderer::IRenderer* renderer
        ) {
            if (!registry.HasComponent<ActivePowerUps>(player)) {
                registry.AddComponent<ActivePowerUps>(player, ActivePowerUps());
            }

            auto& activePowerUps = registry.GetComponent<ActivePowerUps>(player);

            switch (type) {
                case PowerUpType::FIRE_RATE_BOOST: {
                    if (!activePowerUps.hasFireRateBoost) {
                        activePowerUps.hasFireRateBoost = true;

                        if (registry.HasComponent<Shooter>(player)) {
                            auto& shooter = registry.GetComponent<Shooter>(player);
                            shooter.fireRate *= 0.5f;
                        }
                    }
                    break;
                }

                case PowerUpType::SPREAD_SHOT: {
                    if (!activePowerUps.hasSpreadShot) {
                        activePowerUps.hasSpreadShot = true;

                        registry.AddComponent<WeaponSlot>(
                            player,
                            WeaponSlot(WeaponType::SPREAD, 0.3f, 20)
                        );
                    }
                    break;
                }

                case PowerUpType::LASER_BEAM: {
                    if (!activePowerUps.hasLaserBeam) {
                        activePowerUps.hasLaserBeam = true;

                        registry.AddComponent<WeaponSlot>(
                            player,
                            WeaponSlot(WeaponType::LASER, 0.15f, 40)
                        );
                    }
                    break;
                }

                case PowerUpType::FORCE_POD: {
                    CreateForcePod(registry, player, renderer);
                    break;
                }

                case PowerUpType::SPEED_BOOST: {
                    activePowerUps.speedMultiplier += 0.3f;

                    if (registry.HasComponent<Controllable>(player)) {
                        auto& controllable = registry.GetComponent<Controllable>(player);
                        controllable.speed = 200.0f * activePowerUps.speedMultiplier;
                    }
                    break;
                }

                case PowerUpType::SHIELD: {
                    if (!activePowerUps.hasShield) {
                        activePowerUps.hasShield = true;
                        constexpr float SHIELD_DURATION_SECONDS = 5.0f;
                        registry.AddComponent<Shield>(player, Shield(SHIELD_DURATION_SECONDS));
                    }
                    break;
                }
            }
        }

        Entity PowerUpFactory::CreateForcePod(
            Registry& registry,
            Entity owner,
            Renderer::IRenderer* renderer
        ) {
            Entity forcePod = registry.CreateEntity();

            if (registry.HasComponent<Position>(owner)) {
                const auto& ownerPos = registry.GetComponent<Position>(owner);
                registry.AddComponent<Position>(forcePod, Position(ownerPos.x - 60.0f, ownerPos.y));
            } else {
                registry.AddComponent<Position>(forcePod, Position(0.0f, 0.0f));
            }

            registry.AddComponent<ForcePod>(forcePod, ForcePod(owner, -60.0f, 0.0f));
            registry.AddComponent<BoxCollider>(forcePod, BoxCollider(40.0f, 40.0f));

            registry.AddComponent<Shooter>(forcePod, Shooter(0.25f, 50.0f, 20.0f));
            registry.AddComponent<ShootCommand>(forcePod, ShootCommand());

            if (renderer) {
                Renderer::TextureId textureId = renderer->LoadTexture("../assets/powerups/force_pod.png");
                if (textureId == Renderer::INVALID_TEXTURE_ID) {
                    textureId = renderer->LoadTexture("../assets/spaceships/player_blue.png");
                }

                if (textureId != Renderer::INVALID_TEXTURE_ID) {
                    Renderer::SpriteId spriteId = renderer->CreateSprite(
                        textureId,
                        Renderer::Rectangle{{0.0f, 0.0f}, {128.0f, 128.0f}}
                    );

                    auto& drawable = registry.AddComponent<Drawable>(forcePod, Drawable(spriteId, 9));
                    drawable.tint = Math::Color(1.0f, 0.6f, 0.0f, 1.0f);
                    drawable.scale = Math::Vector2(0.4f, 0.4f);
                }
            }

            return forcePod;
        }

        Math::Color PowerUpFactory::GetPowerUpColor(PowerUpType type) {
            return POWERUP_DATA_TABLE[static_cast<size_t>(type)].color;
        }

        const char* PowerUpFactory::GetPowerUpSpritePath(PowerUpType type) {
            return POWERUP_DATA_TABLE[static_cast<size_t>(type)].spritePath;
        }

        const char* PowerUpFactory::GetPowerUpName(PowerUpType type) {
            return POWERUP_DATA_TABLE[static_cast<size_t>(type)].name;
        }
    }
}
