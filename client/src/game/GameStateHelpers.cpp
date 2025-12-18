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
            } else {
                if (m_registry.HasComponent<WeaponSlot>(playerEntity)) {
                    m_registry.RemoveComponent<WeaponSlot>(playerEntity);
                }
            }

            if (!m_registry.HasComponent<WeaponSlot>(playerEntity) && m_registry.HasComponent<Shooter>(playerEntity)) {
                auto& shooter = m_registry.GetComponent<Shooter>(playerEntity);
                shooter.fireRate = fireRate;
            }

            if (activePowerUps.hasShield) {
                if (!m_registry.HasComponent<Shield>(playerEntity)) {
                    constexpr float SHIELD_DURATION_SECONDS = 5.0f;
                    m_registry.AddComponent<Shield>(playerEntity, Shield(SHIELD_DURATION_SECONDS));
                }
            } else {
                if (m_registry.HasComponent<Shield>(playerEntity)) {
                    m_registry.RemoveComponent<Shield>(playerEntity);
                }
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

    }
}
