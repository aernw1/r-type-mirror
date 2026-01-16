/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameState - Helper functions (sprite configs, power-ups, player labels)
*/

#include "../../include/GameState.hpp"

#include "ECS/Components/TextLabel.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"

using namespace RType::ECS;

namespace RType {
    namespace Client {

        InGameState::EnemySpriteConfig InGameState::GetEnemySpriteConfig(uint8_t enemyType) const {
            const Renderer::SpriteId* sprites[] = {
                &m_enemyGreenSprite,
                &m_enemyRedSprite,
                &m_enemyBlueSprite,
                &m_enemyGreenSprite,
                &m_enemyGreenSprite};

            static const Math::Color tints[] = {
                {1.0f, 1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f, 1.0f}};
            static const float rotations[] = {
                180.0f,
                180.0f,
                180.0f,
                180.0f,
                180.0f};

            size_t index = (enemyType < 5) ? enemyType : 0;
            EnemySpriteConfig result;
            result.sprite = *sprites[index];
            result.tint = tints[index];
            result.rotation = rotations[index];
            return result;
        }

        Renderer::SpriteId InGameState::GetPowerUpSprite(ECS::PowerUpType type) const {
            switch (type) {
                case ECS::PowerUpType::FIRE_RATE_BOOST:
                case ECS::PowerUpType::SPREAD_SHOT:
                    return m_powerupSpreadSprite;
                case ECS::PowerUpType::LASER_BEAM:
                    return m_powerupLaserSprite;
                case ECS::PowerUpType::FORCE_POD:
                    return m_powerupForcePodSprite;
                case ECS::PowerUpType::SPEED_BOOST:
                    return m_powerupSpeedSprite;
                case ECS::PowerUpType::SHIELD:
                    return m_powerupShieldSprite;
                default:
                    return m_powerupSpreadSprite;
            }
        }

        InGameState::EnemyBulletSpriteConfig InGameState::GetEnemyBulletSpriteConfig(uint8_t enemyType) const {
            const Renderer::SpriteId* sprites[] = {
                &m_enemyBulletGreenSprite,
                &m_enemyBulletYellowSprite,
                &m_enemyBulletPurpleSprite};

            static const Math::Color tints[] = {
                {1.0f, 1.0f, 1.0f, 1.0f},
                {1.0f, 0.2f, 0.2f, 1.0f},
                {0.8f, 0.3f, 1.0f, 1.0f}};

            static const float scales[] = {
                0.14f,
                0.09f,
                0.18f};

            size_t index = (enemyType < 3) ? enemyType : 0;
            EnemyBulletSpriteConfig result;
            result.sprite = *sprites[index];
            result.tint = tints[index];
            result.scale = scales[index];
            return result;
        }

        void InGameState::ApplyPowerUpStateToPlayer(ECS::Entity playerEntity, const network::EntityState& entityState) {
            using namespace RType::ECS;
            using namespace network;

            if (!m_registry.IsEntityAlive(playerEntity)) {
                return;
            }

            if (!m_registry.HasComponent<ActivePowerUps>(playerEntity)) {
                m_registry.AddComponent<ActivePowerUps>(playerEntity, ActivePowerUps());
            }
            auto& activePowerUps = m_registry.GetComponent<ActivePowerUps>(playerEntity);

            bool gainedPowerUp = false;
            bool isLocal = (playerEntity == m_localPlayerEntity);

            if (isLocal) {
                bool newFireRate = (entityState.powerUpFlags & PowerUpFlags::POWERUP_FIRE_RATE_BOOST) != 0;
                bool newSpread = (entityState.powerUpFlags & PowerUpFlags::POWERUP_SPREAD_SHOT) != 0;
                bool newLaser = (entityState.powerUpFlags & PowerUpFlags::POWERUP_LASER_BEAM) != 0;
                bool newShield = (entityState.powerUpFlags & PowerUpFlags::POWERUP_SHIELD) != 0;
                
                if (!activePowerUps.hasFireRateBoost && newFireRate) gainedPowerUp = true;
                if (!activePowerUps.hasSpreadShot && newSpread) gainedPowerUp = true;
                if (!activePowerUps.hasLaserBeam && newLaser) gainedPowerUp = true;
                if (!activePowerUps.hasShield && newShield) gainedPowerUp = true;
                
            }

            activePowerUps.hasFireRateBoost = (entityState.powerUpFlags & PowerUpFlags::POWERUP_FIRE_RATE_BOOST) != 0;
            activePowerUps.hasSpreadShot = (entityState.powerUpFlags & PowerUpFlags::POWERUP_SPREAD_SHOT) != 0;
            activePowerUps.hasLaserBeam = (entityState.powerUpFlags & PowerUpFlags::POWERUP_LASER_BEAM) != 0;
            activePowerUps.hasShield = (entityState.powerUpFlags & PowerUpFlags::POWERUP_SHIELD) != 0;

            float speedMult = static_cast<float>(entityState.speedMultiplier) / 10.0f;
            activePowerUps.speedMultiplier = speedMult;

            if (m_registry.HasComponent<Controllable>(playerEntity)) {
                auto& controllable = m_registry.GetComponent<Controllable>(playerEntity);
                controllable.speed = 200.0f * speedMult;
            }

            WeaponType weaponType = static_cast<WeaponType>(entityState.weaponType);
            
            if (isLocal) {
                bool hadSpecialWeapon = m_registry.HasComponent<WeaponSlot>(playerEntity);
                bool hasSpecialWeaponNow = (weaponType != WeaponType::STANDARD);
                
                
                if (!hadSpecialWeapon && hasSpecialWeaponNow) {
                    std::cout << "[DEBUG] PowerUp: Acquired special weapon!" << std::endl;
                    gainedPowerUp = true;
                } else if (hadSpecialWeapon && hasSpecialWeaponNow) {
                    auto& currentWeapon = m_registry.GetComponent<WeaponSlot>(playerEntity);
                    if (currentWeapon.type != weaponType) {
                        std::cout << "[DEBUG] PowerUp: Changed special weapon!" << std::endl;
                        gainedPowerUp = true;
                    }
                }
                
                bool newFireRate = (entityState.powerUpFlags & PowerUpFlags::POWERUP_FIRE_RATE_BOOST) != 0;
                bool newSpread = (entityState.powerUpFlags & PowerUpFlags::POWERUP_SPREAD_SHOT) != 0;
                if (!activePowerUps.hasFireRateBoost && newFireRate) { 
                    std::cout << "[DEBUG] PowerUp: FireRate Boost!" << std::endl;
                    gainedPowerUp = true; 
                }
                if (!activePowerUps.hasSpreadShot && newSpread) {
                    std::cout << "[DEBUG] PowerUp: Spread Shot!" << std::endl;
                    gainedPowerUp = true;
                }
            }

            if (gainedPowerUp) {
                std::cout << "[DEBUG] PowerUp GAINED! Playing sound..." << std::endl;
                if (m_context.audio) {
                    if (m_powerUpMusic != Audio::INVALID_MUSIC_ID) {
                        Audio::PlaybackOptions opts;
                        opts.volume = 1.0f;
                        opts.loop = false;
                        m_context.audio->PlayMusic(m_powerUpMusic, opts);
                    } else if (m_powerUpSound != Audio::INVALID_SOUND_ID) {
                        Audio::PlaybackOptions opts;
                        opts.volume = 1.0f;
                        m_context.audio->PlaySound(m_powerUpSound, opts);
                    }
                }
            }

            float fireRate = static_cast<float>(entityState.fireRate) / 10.0f;

            if (weaponType != WeaponType::STANDARD) {
                if (m_registry.HasComponent<WeaponSlot>(playerEntity)) {
                    auto& weaponSlot = m_registry.GetComponent<WeaponSlot>(playerEntity);
                    weaponSlot.type = weaponType;
                    weaponSlot.fireRate = fireRate;
                } else {
                    int damage = (weaponType == WeaponType::LASER) ? 40 : 20;
                    m_registry.AddComponent<WeaponSlot>(playerEntity, WeaponSlot(weaponType, fireRate, damage));
                }
                activePowerUps.hasSpreadShot = (weaponType == WeaponType::SPREAD);
                activePowerUps.hasLaserBeam = (weaponType == WeaponType::LASER);
            } else {
                if (m_registry.HasComponent<WeaponSlot>(playerEntity)) {
                    m_registry.RemoveComponent<WeaponSlot>(playerEntity);
                }
                activePowerUps.hasSpreadShot = false;
                activePowerUps.hasLaserBeam = false;
            }

            if (!m_registry.HasComponent<WeaponSlot>(playerEntity) && m_registry.HasComponent<Shooter>(playerEntity)) {
                auto& shooter = m_registry.GetComponent<Shooter>(playerEntity);
                shooter.fireRate = fireRate;
            }

            constexpr float SHIELD_DURATION_SECONDS = 5.0f;
            if (activePowerUps.hasShield) {
                if (!m_registry.HasComponent<Shield>(playerEntity)) {
                    m_registry.AddComponent<Shield>(playerEntity, Shield(SHIELD_DURATION_SECONDS));
                } else {
                    auto& shield = m_registry.GetComponent<Shield>(playerEntity);
                    if (shield.timeRemaining <= 0.0f) {
                        shield.timeRemaining = SHIELD_DURATION_SECONDS;
                    }
                }
            } else {
                if (m_registry.HasComponent<Shield>(playerEntity)) {
                    m_registry.RemoveComponent<Shield>(playerEntity);
                    if (m_registry.HasComponent<Drawable>(playerEntity)) {
                        auto& drawable = m_registry.GetComponent<Drawable>(playerEntity);
                        drawable.tint = Math::Color(1.0f, 1.0f, 1.0f, 1.0f);
                    }
                }
            }

        }

        void InGameState::createBeamEntity() {
            if (m_localPlayerEntity == ECS::NULL_ENTITY || !m_registry.IsEntityAlive(m_localPlayerEntity)) {
                return;
            }

            if (!m_registry.HasComponent<ECS::Position>(m_localPlayerEntity) ||
                !m_registry.HasComponent<ECS::Shooter>(m_localPlayerEntity) ||
                !m_effectFactory) {
                return;
            }

            const auto& playerPos = m_registry.GetComponent<ECS::Position>(m_localPlayerEntity);
            const auto& shooter = m_registry.GetComponent<ECS::Shooter>(m_localPlayerEntity);

            float beamX = playerPos.x + shooter.offsetX;
            float beamY = playerPos.y + shooter.offsetY;

            float screenWidth = 1280.0f;
            if (m_levelData.config.screenWidth > 0.0f) {
                screenWidth = m_levelData.config.screenWidth;
            }

            float beamHeight = 40.0f;
            if (m_registry.HasComponent<ECS::Drawable>(m_localPlayerEntity)) {
                const auto& playerDrawable = m_registry.GetComponent<ECS::Drawable>(m_localPlayerEntity);
                beamHeight = 80.0f * playerDrawable.scale.y;
            }

            float savedChargeTime = m_chargeTime;
            m_beamEntity = m_effectFactory->CreateBeam(m_registry, beamX, beamY, m_localPlayerEntity, savedChargeTime, screenWidth, beamHeight);
        }

        void InGameState::updateBeam(float dt) {
            if (m_beamEntity == ECS::NULL_ENTITY) {
                return;
            }

            if (!m_registry.IsEntityAlive(m_beamEntity)) {
                m_beamEntity = ECS::NULL_ENTITY;
                return;
            }

            // Update beam width dynamically as player moves
            if (m_registry.HasComponent<ECS::Position>(m_beamEntity)) {
                const auto& beamPos = m_registry.GetComponent<ECS::Position>(m_beamEntity);
                
                float screenWidth = 1280.0f;
                if (m_levelData.config.screenWidth > 0.0f) {
                    screenWidth = m_levelData.config.screenWidth;
                }

                // Get frame width from config or use default
                float frameWidth = 200.0f;
                if (m_effectFactory) {
                    const auto& config = m_effectFactory->GetConfig();
                    if (config.beamFirstFrameRegion.size.x > 0.0f) {
                        frameWidth = config.beamFirstFrameRegion.size.x;
                    }
                }

                // Calculate new beam width (extend from current position to screen edge)
                float newBeamWidth = screenWidth - beamPos.x;
                if (newBeamWidth < frameWidth) {
                    newBeamWidth = frameWidth;
                }

                // Update Drawable scale if it exists
                if (m_registry.HasComponent<ECS::Drawable>(m_beamEntity)) {
                    auto& drawable = m_registry.GetComponent<ECS::Drawable>(m_beamEntity);
                    float newScaleX = newBeamWidth / frameWidth;
                    // Preserve the Y scale
                    drawable.scale.x = newScaleX;
                }

                // Update BoxCollider width if it exists
                if (m_registry.HasComponent<ECS::BoxCollider>(m_beamEntity)) {
                    auto& collider = m_registry.GetComponent<ECS::BoxCollider>(m_beamEntity);
                    // Get frame height to calculate the scaled height
                    float frameHeight = 64.0f;
                    if (m_effectFactory) {
                        const auto& config = m_effectFactory->GetConfig();
                        if (config.beamFirstFrameRegion.size.y > 0.0f) {
                            frameHeight = config.beamFirstFrameRegion.size.y;
                        }
                    }
                    
                    // Get current scale Y from drawable or use default
                    float scaleY = 1.0f;
                    if (m_registry.HasComponent<ECS::Drawable>(m_beamEntity)) {
                        scaleY = m_registry.GetComponent<ECS::Drawable>(m_beamEntity).scale.y;
                    }
                    
                    collider.width = newBeamWidth;
                    collider.height = frameHeight * scaleY;
                }
            }

            if (m_registry.HasComponent<ECS::SpriteAnimation>(m_beamEntity)) {
                auto& anim = m_registry.GetComponent<ECS::SpriteAnimation>(m_beamEntity);
                if (anim.looping && m_animationSystem) {
                    anim.destroyOnComplete = false;
                }
            }

            m_beamDuration -= dt;
            if (m_beamDuration <= 0.0f) {
                if (m_registry.IsEntityAlive(m_beamEntity)) {
                    m_registry.DestroyEntity(m_beamEntity);
                }
                m_beamEntity = ECS::NULL_ENTITY;
                m_beamDuration = 0.0f;
            }
        }

        std::pair<std::string, uint8_t> InGameState::FindPlayerNameAndNumber(uint64_t ownerHash, const std::unordered_set<uint8_t>& assignedNumbers) const {
            std::string playerName = "Player";
            uint8_t playerNum = 0;

            if (ownerHash != 0) {
                auto nameIt = m_playerNameMap.find(ownerHash);
                if (nameIt != m_playerNameMap.end()) {
                    playerName = nameIt->second;
                }
                for (const auto& p : m_context.allPlayers) {
                    if (p.hash == ownerHash) {
                        playerNum = p.number;
                        break;
                    }
                }
            }

            if (playerName == "Player" || playerNum == 0) {
                for (const auto& p : m_context.allPlayers) {
                    if (p.number > 0 && p.number <= MAX_PLAYERS) {
                        if (assignedNumbers.find(p.number) == assignedNumbers.end()) {
                            auto fallbackIt = m_playerNameMap.find(static_cast<uint64_t>(p.number));
                            if (fallbackIt != m_playerNameMap.end() && !fallbackIt->second.empty()) {
                                playerName = fallbackIt->second;
                                playerNum = p.number;
                                break;
                            }
                        }
                    }
                }
            }

            return {playerName, playerNum};
        }

        void InGameState::CreatePlayerNameLabel(Entity playerEntity, const std::string& playerName, float x, float y) {
            Renderer::FontId nameFont = (m_hudFontSmall != Renderer::INVALID_FONT_ID) ? m_hudFontSmall : m_hudFont;
            if (nameFont == Renderer::INVALID_FONT_ID) {
                return;
            }

            std::string displayName = playerName;
            if (displayName.length() > 16) {
                displayName = displayName.substr(0, 16);
            }

            Entity nameLabelEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(nameLabelEntity, Position{x, y});
            TextLabel nameLabel(displayName, nameFont, 12);
            nameLabel.color = {1.0f, 1.0f, 1.0f, 1.0f};
            nameLabel.centered = true;
            nameLabel.offsetY = -30.0f;
            nameLabel.offsetX = 0.0f;
            m_registry.AddComponent<TextLabel>(nameLabelEntity, std::move(nameLabel));
            m_playerNameLabels[playerEntity] = nameLabelEntity;
        }

        void InGameState::UpdatePlayerNameLabelPosition(Entity playerEntity, float x, float y) {
            auto labelIt = m_playerNameLabels.find(playerEntity);
            if (labelIt == m_playerNameLabels.end()) {
                return;
            }

            Entity labelEntity = labelIt->second;
            if (!m_registry.IsEntityAlive(labelEntity) || !m_registry.HasComponent<Position>(labelEntity)) {
                return;
            }

            auto& labelPos = m_registry.GetComponent<Position>(labelEntity);
            labelPos.x = x;
            labelPos.y = y;
        }

        void InGameState::DestroyPlayerNameLabel(Entity playerEntity) {
            auto labelIt = m_playerNameLabels.find(playerEntity);
            if (labelIt == m_playerNameLabels.end()) {
                return;
            }

            Entity labelEntity = labelIt->second;
            if (m_registry.IsEntityAlive(labelEntity)) {
                m_registry.DestroyEntity(labelEntity);
            }
            m_playerNameLabels.erase(labelIt);
        }

        void InGameState::CleanupInvalidComponents(ECS::Entity entity, network::EntityType expectedType) {
            if (!m_registry.IsEntityAlive(entity)) {
                return;
            }

            if (expectedType == network::EntityType::OBSTACLE) {
                if (m_registry.HasComponent<Shooter>(entity)) m_registry.RemoveComponent<Shooter>(entity);
                if (m_registry.HasComponent<ShootCommand>(entity)) m_registry.RemoveComponent<ShootCommand>(entity);
                if (m_registry.HasComponent<WeaponSlot>(entity)) m_registry.RemoveComponent<WeaponSlot>(entity);
                if (m_registry.HasComponent<Bullet>(entity)) m_registry.RemoveComponent<Bullet>(entity);
                if (m_registry.HasComponent<Enemy>(entity)) m_registry.RemoveComponent<Enemy>(entity);
                if (m_registry.HasComponent<Player>(entity)) m_registry.RemoveComponent<Player>(entity);
            } else if (expectedType == network::EntityType::BULLET) {
                if (m_registry.HasComponent<Drawable>(entity)) m_registry.RemoveComponent<Drawable>(entity);
                if (m_registry.HasComponent<Obstacle>(entity)) m_registry.RemoveComponent<Obstacle>(entity);
                if (m_registry.HasComponent<ObstacleMetadata>(entity)) m_registry.RemoveComponent<ObstacleMetadata>(entity);
                if (m_registry.HasComponent<Player>(entity)) m_registry.RemoveComponent<Player>(entity);
                if (m_registry.HasComponent<Enemy>(entity)) m_registry.RemoveComponent<Enemy>(entity);
                if (m_registry.HasComponent<BoxCollider>(entity)) m_registry.RemoveComponent<BoxCollider>(entity);
                if (m_registry.HasComponent<CircleCollider>(entity)) m_registry.RemoveComponent<CircleCollider>(entity);
                if (m_registry.HasComponent<CollisionLayer>(entity)) m_registry.RemoveComponent<CollisionLayer>(entity);
            } else {
                if (m_registry.HasComponent<Drawable>(entity)) m_registry.RemoveComponent<Drawable>(entity);
                if (m_registry.HasComponent<Obstacle>(entity)) m_registry.RemoveComponent<Obstacle>(entity);
                if (m_registry.HasComponent<ObstacleMetadata>(entity)) m_registry.RemoveComponent<ObstacleMetadata>(entity);
                if (m_registry.HasComponent<Bullet>(entity)) m_registry.RemoveComponent<Bullet>(entity);
                
                if (expectedType == network::EntityType::PLAYER || expectedType == network::EntityType::POWERUP) {
                    if (m_registry.HasComponent<Enemy>(entity)) m_registry.RemoveComponent<Enemy>(entity);
                }
                if (expectedType == network::EntityType::ENEMY || expectedType == network::EntityType::POWERUP) {
                    if (m_registry.HasComponent<Player>(entity)) m_registry.RemoveComponent<Player>(entity);
                }
                if (expectedType == network::EntityType::BOSS) {
                    if (m_registry.HasComponent<Player>(entity)) m_registry.RemoveComponent<Player>(entity);
                }
            }
        }

    }
}
