/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameState - Network state update functions
*/

#include "../../include/GameState.hpp"

#include "ECS/Components/TextLabel.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include "ECS/PowerUpFactory.hpp"

using namespace RType::ECS;

namespace RType {
    namespace Client {

        void InGameState::OnServerStateUpdate(uint32_t tick, const std::vector<network::EntityState>& entities) {
            auto now = std::chrono::steady_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

            static int stateLog = 0;
            if (stateLog++ % 60 == 0) {
                std::cout << "[CLIENT RECV STATE] t=" << ms << " tick=" << tick << " entities=" << entities.size() << std::endl;
            }

            if (m_context.networkClient) {
                m_serverScrollOffset = m_context.networkClient->GetLastScrollOffset();
            }

            std::unordered_set<uint32_t> receivedIds;
            std::vector<uint32_t> entitiesToRemove;

            for (const auto& entityState : entities) {
                receivedIds.insert(entityState.entityId);

                auto it = m_networkEntityMap.find(entityState.entityId);
                network::EntityType type = static_cast<network::EntityType>(entityState.entityType);

                if (it == m_networkEntityMap.end()) {
                    if (type == network::EntityType::PLAYER) {
                        bool isForcePod = (entityState.flags & 0x80) != 0;
                        if (isForcePod) {
                            auto newEntity = m_registry.CreateEntity();
                            m_registry.AddComponent<Position>(newEntity, Position{entityState.x, entityState.y});
                            m_registry.AddComponent<Velocity>(newEntity, Velocity{entityState.vx, entityState.vy});

                            Renderer::SpriteId podSprite = m_powerupForcePodSprite;
                            if (podSprite != Renderer::INVALID_SPRITE_ID) {
                                auto& d = m_registry.AddComponent<Drawable>(newEntity, Drawable(podSprite, 9));
                                d.scale = {0.4f, 0.4f};
                                d.origin = Math::Vector2(128.0f, 128.0f);
                            }

                            m_networkEntityMap[entityState.entityId] = newEntity;
                            std::cout << "[GameState] Created FORCE POD entity " << entityState.entityId << std::endl;
                            continue;
                        }

                        if (entityState.ownerHash == m_context.playerHash) {
                            auto existing = m_networkEntityMap.find(entityState.entityId);
                            if (existing != m_networkEntityMap.end()) {
                                continue;
                            }
                        }

                        Renderer::SpriteId playerSprite = Renderer::INVALID_SPRITE_ID;
                        size_t playerIndex = m_networkEntityMap.size() % 3;

                        const char* spriteKeys[] = {"player_green", "player_blue", "player_red"};
                        auto spriteIt = m_levelAssets.sprites.find(spriteKeys[playerIndex]);
                        if (spriteIt != m_levelAssets.sprites.end()) {
                            playerSprite = spriteIt->second;
                        }

                        if (playerSprite == Renderer::INVALID_SPRITE_ID) {
                            continue;
                        }

                        auto newEntity = m_registry.CreateEntity();
                        m_registry.AddComponent<Position>(newEntity, Position{entityState.x, entityState.y});
                        m_registry.AddComponent<Velocity>(newEntity, Velocity{entityState.vx, entityState.vy});
                        m_registry.AddComponent<Health>(newEntity, Health{static_cast<int>(entityState.health), 100});

                        m_registry.AddComponent<Controllable>(newEntity, Controllable{200.0f});
                        m_registry.AddComponent<Shooter>(newEntity, Shooter{0.2f, 50.0f, 20.0f});
                        m_registry.AddComponent<BoxCollider>(newEntity, BoxCollider{50.0f, 50.0f});
                        m_registry.AddComponent<CircleCollider>(newEntity, CircleCollider{25.0f});
                        m_registry.AddComponent<CollisionLayer>(newEntity, CollisionLayer(CollisionLayers::PLAYER, CollisionLayers::ENEMY | CollisionLayers::ENEMY_BULLET | CollisionLayers::OBSTACLE));

                        auto& drawable = m_registry.AddComponent<Drawable>(newEntity, Drawable(playerSprite, 10));
                        drawable.scale = {0.5f, 0.5f};
                        drawable.origin = Math::Vector2(128.0f, 128.0f);

                        auto [playerName, playerNum] = FindPlayerNameAndNumber(entityState.ownerHash, m_assignedPlayerNumbers);
                        if (playerNum > 0 && playerNum <= MAX_PLAYERS) {
                            m_assignedPlayerNumbers.insert(playerNum);
                        }
                        CreatePlayerNameLabel(newEntity, playerName, entityState.x, entityState.y);

                        ApplyPowerUpStateToPlayer(newEntity, entityState);

                        m_networkEntityMap[entityState.entityId] = newEntity;

                        if (entityState.ownerHash == m_context.playerHash) {
                            m_localPlayerEntity = newEntity;
                            if (!m_registry.HasComponent<ShootCommand>(newEntity)) {
                                m_registry.AddComponent<ShootCommand>(newEntity, ShootCommand{});
                            }
                            if (m_context.playerNumber >= 1 && m_context.playerNumber <= MAX_PLAYERS) {
                                size_t localPlayerIndex = static_cast<size_t>(m_context.playerNumber - 1);
                                m_playersHUD[localPlayerIndex].playerEntity = newEntity;
                                m_playersHUD[localPlayerIndex].active = true;
                                m_playersHUD[localPlayerIndex].score = entityState.score;
                                m_playerScore = entityState.score;
                                if (entityState.health > 0) {
                                    m_playersHUD[localPlayerIndex].isDead = false;
                                }
                            }
                        }
                        if (playerNum > 0 && playerNum <= MAX_PLAYERS) {
                            size_t hudPlayerIndex = static_cast<size_t>(playerNum - 1);
                            m_playersHUD[hudPlayerIndex].active = true;
                            m_playersHUD[hudPlayerIndex].playerEntity = newEntity;
                            m_playersHUD[hudPlayerIndex].score = entityState.score;

                            if (entityState.health > 0) {
                                m_playersHUD[hudPlayerIndex].isDead = false;
                            }
                            std::cout << "[GameState] Player P" << (int)playerNum << " (" << playerName << ") added to scoreboard" << std::endl;
                        }

                        std::cout << "[GameState] Created PLAYER entity " << entityState.entityId << " with color index " << playerIndex << std::endl;
                    } else if (type == network::EntityType::ENEMY) {
                        uint8_t enemyType = entityState.flags;
                        EnemySpriteConfig config = GetEnemySpriteConfig(enemyType);
                        Renderer::SpriteId enemySprite = config.sprite;
                        Math::Color enemyTint = config.tint;

                        if (enemySprite == Renderer::INVALID_SPRITE_ID) {
                            enemySprite = m_enemyGreenSprite;
                        }

                        if (enemySprite == Renderer::INVALID_SPRITE_ID) {
                            Core::Logger::Error("[GameState] Missing enemy sprite for type {}, skipping entity {}", static_cast<int>(enemyType), entityState.entityId);
                            continue;
                        }

                        auto newEntity = m_registry.CreateEntity();
                        m_registry.AddComponent<Position>(newEntity, Position{entityState.x, entityState.y});
                        m_registry.AddComponent<Velocity>(newEntity, Velocity{entityState.vx, entityState.vy});
                        m_registry.AddComponent<Health>(newEntity, Health{static_cast<int>(entityState.health), 100});
                        auto& drawable = m_registry.AddComponent<Drawable>(newEntity, Drawable(enemySprite, 1));
                        drawable.scale = {0.5f, 0.5f};
                        drawable.origin = Math::Vector2(128.0f, 128.0f);
                        drawable.rotation = config.rotation;
                        drawable.tint = enemyTint;

                        m_networkEntityMap[entityState.entityId] = newEntity;
                        std::cout << "[GameState] Created ENEMY entity " << entityState.entityId << " type " << static_cast<int>(enemyType) << std::endl;
                    } else if (type == network::EntityType::BOSS) {
                        Renderer::SpriteId bossSprite = Renderer::INVALID_SPRITE_ID;
                        auto spriteIt = m_levelAssets.sprites.find("boss_dragon");
                        if (spriteIt != m_levelAssets.sprites.end()) {
                            bossSprite = spriteIt->second;
                        }

                        if (bossSprite == Renderer::INVALID_SPRITE_ID) {
                            Core::Logger::Warning("[GameState] Missing boss sprite (entity {})", entityState.entityId);
                            continue;
                        }

                        auto newEntity = m_registry.CreateEntity();
                        m_registry.AddComponent<Position>(newEntity, Position{entityState.x, entityState.y});
                        m_registry.AddComponent<Velocity>(newEntity, Velocity{entityState.vx, entityState.vy});
                        m_registry.AddComponent<Health>(newEntity, Health{static_cast<int>(entityState.health), 1000});

                        auto& drawable = m_registry.AddComponent<Drawable>(newEntity, Drawable(bossSprite, 5));
                        drawable.scale = {3.0f, 3.0f};
                        drawable.origin = Math::Vector2(0.0f, 0.0f);

                        m_networkEntityMap[entityState.entityId] = newEntity;
                        std::cout << "[GameState] Created BOSS entity " << entityState.entityId << std::endl;

                        m_bossHealthBar.currentHealth = static_cast<int>(entityState.health);
                        m_bossHealthBar.maxHealth = 1000;
                    } else if (type == network::EntityType::BULLET) {
                        auto newEntity = m_registry.CreateEntity();
                        m_registry.AddComponent<Position>(newEntity, Position{entityState.x, entityState.y});
                        m_registry.AddComponent<Velocity>(newEntity, Velocity{entityState.vx, entityState.vy});

                        if (entityState.flags == 13) {
                            auto bossBulletSpriteIt = m_levelAssets.sprites.find("boss_bullet");
                            if (bossBulletSpriteIt != m_levelAssets.sprites.end()) {
                                auto& d = m_registry.AddComponent<Drawable>(newEntity, Drawable(bossBulletSpriteIt->second, 12));
                                d.scale = {2.0f, 2.0f};
                                d.origin = Math::Vector2(8.0f, 4.0f);
                            } else {
                                Core::Logger::Warning("[GameState] Missing boss bullet sprite (entity {})", entityState.entityId);
                                m_registry.DestroyEntity(newEntity);
                                continue;
                            }
                        } else if (entityState.flags >= 10) {
                            uint8_t enemyType = entityState.flags - 10;
                            EnemyBulletSpriteConfig config = GetEnemyBulletSpriteConfig(enemyType);
                            Renderer::SpriteId bulletSprite = config.sprite;
                            Math::Color bulletTint = config.tint;
                            float scaleValue = config.scale;

                            if (bulletSprite == Renderer::INVALID_SPRITE_ID) {
                                auto bulletSpriteIt = m_levelAssets.sprites.find("bullet");
                                if (bulletSpriteIt != m_levelAssets.sprites.end()) {
                                    bulletSprite = bulletSpriteIt->second;
                                }
                            }

                            if (bulletSprite == Renderer::INVALID_SPRITE_ID) {
                                Core::Logger::Warning("[GameState] Missing enemy bullet sprite for type {} (entity {})", static_cast<int>(enemyType), entityState.entityId);
                                m_registry.DestroyEntity(newEntity);
                                continue;
                            }

                            auto& d = m_registry.AddComponent<Drawable>(newEntity, Drawable(bulletSprite, 12));
                            d.scale = {scaleValue, scaleValue};
                            d.origin = Math::Vector2(128.0f, 128.0f);
                            d.tint = bulletTint;
                        } else {
                            auto bulletSpriteIt = m_levelAssets.sprites.find("bullet");
                            if (bulletSpriteIt != m_levelAssets.sprites.end()) {
                                auto& d = m_registry.AddComponent<Drawable>(newEntity, Drawable(bulletSpriteIt->second, 12));
                                d.scale = {0.1f, 0.1f};
                                d.origin = Math::Vector2(128.0f, 128.0f);
                                d.tint = {0.2f, 0.8f, 1.0f, 1.0f};
                            }
                        }
                        m_networkEntityMap[entityState.entityId] = newEntity;
                    } else if (type == network::EntityType::OBSTACLE) {
                        uint64_t obstacleId = entityState.ownerHash;
                        auto colliderIt = m_obstacleIdToCollider.find(obstacleId);
                        if (colliderIt == m_obstacleIdToCollider.end()) {
                            continue;
                        }

                        ECS::Entity colliderEntity = colliderIt->second;
                        if (!m_registry.IsEntityAlive(colliderEntity) ||
                            !m_registry.HasComponent<Position>(colliderEntity)) {
                            continue;
                        }

                        auto& colliderPos = m_registry.GetComponent<Position>(colliderEntity);
                        colliderPos.x = entityState.x;
                        colliderPos.y = entityState.y;

                        if (m_registry.HasComponent<Scrollable>(colliderEntity)) {
                            m_registry.RemoveComponent<Scrollable>(colliderEntity);
                        }

                        if (m_registry.HasComponent<ECS::ObstacleMetadata>(colliderEntity)) {
                            const auto& metadata = m_registry.GetComponent<ECS::ObstacleMetadata>(colliderEntity);
                            if (metadata.visualEntity != ECS::NULL_ENTITY &&
                                m_registry.IsEntityAlive(metadata.visualEntity) &&
                                m_registry.HasComponent<Position>(metadata.visualEntity)) {
                                auto& visualPos = m_registry.GetComponent<Position>(metadata.visualEntity);
                                visualPos.x = entityState.x - metadata.offsetX;
                                visualPos.y = entityState.y - metadata.offsetY;

                                if (m_registry.HasComponent<Scrollable>(metadata.visualEntity)) {
                                    m_registry.RemoveComponent<Scrollable>(metadata.visualEntity);
                                }
                            }
                        }

                        m_networkEntityMap[entityState.entityId] = colliderEntity;
                    } else if (type == network::EntityType::POWERUP) {
                        uint8_t powerupType = entityState.flags;
                        ECS::PowerUpType puType = static_cast<ECS::PowerUpType>(powerupType);

                        auto newEntity = m_registry.CreateEntity();
                        m_registry.AddComponent<Position>(newEntity, Position{entityState.x, entityState.y});
                        m_registry.AddComponent<Velocity>(newEntity, Velocity{entityState.vx, entityState.vy});

                        Math::Color powerupColor = ECS::PowerUpFactory::GetPowerUpColor(puType);

                        Renderer::SpriteId powerupSprite = Renderer::INVALID_SPRITE_ID;
                        switch (puType) {
                            case ECS::PowerUpType::FIRE_RATE_BOOST:
                                powerupSprite = m_powerupSpreadSprite;
                                break;
                            case ECS::PowerUpType::SPREAD_SHOT:
                                powerupSprite = m_powerupSpreadSprite;
                                break;
                            case ECS::PowerUpType::LASER_BEAM:
                                powerupSprite = m_powerupLaserSprite;
                                break;
                            case ECS::PowerUpType::FORCE_POD:
                                powerupSprite = m_powerupForcePodSprite;
                                break;
                            case ECS::PowerUpType::SPEED_BOOST:
                                powerupSprite = m_powerupSpeedSprite;
                                break;
                            case ECS::PowerUpType::SHIELD:
                                powerupSprite = m_powerupShieldSprite;
                                break;
                            default:
                                powerupSprite = m_powerupSpreadSprite;
                                break;
                        }

                        if (powerupSprite != Renderer::INVALID_SPRITE_ID) {
                            auto& d = m_registry.AddComponent<Drawable>(newEntity, Drawable(powerupSprite, 5));
                            d.scale = {0.8f, 0.8f};
                            d.tint = powerupColor;
                        }

                        m_registry.AddComponent<BoxCollider>(newEntity, BoxCollider{32.0f, 32.0f});

                        m_networkEntityMap[entityState.entityId] = newEntity;
                        std::cout << "[GameState] Created POWERUP entity " << entityState.entityId << " type " << static_cast<int>(powerupType) << std::endl;
                    }
                } else {
                    auto ecsEntity = it->second;

                    if (type == network::EntityType::OBSTACLE) {
                        uint64_t obstacleId = entityState.ownerHash;
                        auto colliderIt = m_obstacleIdToCollider.find(obstacleId);
                        if (colliderIt == m_obstacleIdToCollider.end()) {
                            continue;
                        }

                        ECS::Entity colliderEntity = colliderIt->second;
                        if (!m_registry.IsEntityAlive(colliderEntity) ||
                            !m_registry.HasComponent<Position>(colliderEntity)) {
                            continue;
                        }

                        auto& colliderPos = m_registry.GetComponent<Position>(colliderEntity);
                        colliderPos.x = entityState.x;
                        colliderPos.y = entityState.y;

                        if (m_registry.HasComponent<Scrollable>(colliderEntity)) {
                            m_registry.RemoveComponent<Scrollable>(colliderEntity);
                        }

                        if (m_registry.HasComponent<ECS::ObstacleMetadata>(colliderEntity)) {
                            const auto& metadata = m_registry.GetComponent<ECS::ObstacleMetadata>(colliderEntity);
                            if (metadata.visualEntity != ECS::NULL_ENTITY &&
                                m_registry.IsEntityAlive(metadata.visualEntity) &&
                                m_registry.HasComponent<Position>(metadata.visualEntity)) {
                                auto& visualPos = m_registry.GetComponent<Position>(metadata.visualEntity);
                                visualPos.x = entityState.x - metadata.offsetX;
                                visualPos.y = entityState.y - metadata.offsetY;

                                if (m_registry.HasComponent<Scrollable>(metadata.visualEntity)) {
                                    m_registry.RemoveComponent<Scrollable>(metadata.visualEntity);
                                }
                            }
                        }

                        continue;
                    }

                    if (m_registry.HasComponent<Position>(ecsEntity)) {
                        auto& pos = m_registry.GetComponent<Position>(ecsEntity);
                        pos.x = entityState.x;
                        pos.y = entityState.y;

                        if (type == network::EntityType::PLAYER) {
                            UpdatePlayerNameLabelPosition(ecsEntity, entityState.x, entityState.y);
                        }
                    }

                    if (m_registry.HasComponent<Velocity>(ecsEntity)) {
                        auto& vel = m_registry.GetComponent<Velocity>(ecsEntity);
                        vel.dx = entityState.vx;
                        vel.dy = entityState.vy;
                    }

                    if (type == network::EntityType::PLAYER) {
                        for (size_t i = 0; i < MAX_PLAYERS; i++) {
                            if (m_playersHUD[i].playerEntity == ecsEntity) {
                                m_playersHUD[i].score = entityState.score;
                                if (ecsEntity == m_localPlayerEntity) {
                                    m_playerScore = entityState.score;
                                }
                                break;
                            }
                        }
                    }

                    if (type == network::EntityType::POWERUP) {
                        if (entityState.health == 0 && m_registry.IsEntityAlive(ecsEntity)) {
                            m_registry.DestroyEntity(ecsEntity);
                            entitiesToRemove.push_back(entityState.entityId);
                        }
                        continue;
                    }

                    if (m_registry.HasComponent<Health>(ecsEntity)) {
                        auto& health = m_registry.GetComponent<Health>(ecsEntity);
                        int newHealth = static_cast<int>(entityState.health);
                        if (newHealth < 0)
                            newHealth = 0;
                        if (newHealth > 100)
                            newHealth = 100;

                        bool playerIsDead = false;
                        size_t playerIndex = MAX_PLAYERS;
                        bool isPlayerEntity = false;

                        for (size_t i = 0; i < MAX_PLAYERS; i++) {
                            if (m_playersHUD[i].playerEntity == ecsEntity) {
                                playerIsDead = m_playersHUD[i].isDead;
                                playerIndex = i;
                                isPlayerEntity = true;
                                break;
                            }
                        }
                        if (ecsEntity == m_localPlayerEntity && playerIndex == MAX_PLAYERS) {
                            size_t localPlayerIndex = static_cast<size_t>(m_context.playerNumber - 1);
                            if (localPlayerIndex < MAX_PLAYERS) {
                                playerIsDead = m_playersHUD[localPlayerIndex].isDead;
                                playerIndex = localPlayerIndex;
                                isPlayerEntity = true;
                            }
                        }

                        if (playerIsDead) {
                            health.current = 0;
                            if (playerIndex < MAX_PLAYERS) {
                                m_playersHUD[playerIndex].health = 0;
                            }
                        } else {
                            health.current = newHealth;
                            if (health.max != 100) {
                                health.max = 100;
                            }

                            if (isPlayerEntity && playerIndex < MAX_PLAYERS) {
                                if (newHealth <= 0) {
                                    m_playersHUD[playerIndex].isDead = true;
                                    m_playersHUD[playerIndex].health = 0;
                                    health.current = 0;

                                    DestroyPlayerNameLabel(ecsEntity);

                                    for (const auto& p : m_context.allPlayers) {
                                        if (m_playersHUD[playerIndex].playerEntity == ecsEntity) {
                                            if (p.number > 0 && p.number <= MAX_PLAYERS) {
                                                m_assignedPlayerNumbers.erase(p.number);
                                            }
                                            break;
                                        }
                                    }

                                    if (m_registry.IsEntityAlive(ecsEntity)) {
                                        m_registry.DestroyEntity(ecsEntity);
                                    }
                                    m_playersHUD[playerIndex].playerEntity = NULL_ENTITY;
                                    if (ecsEntity == m_localPlayerEntity) {
                                        m_localPlayerEntity = NULL_ENTITY;
                                    }
                                    entitiesToRemove.push_back(entityState.entityId);
                                } else {
                                    m_playersHUD[playerIndex].health = newHealth;
                                }
                            }
                        }
                    }

                    // Handle boss damage flash and health bar update
                    if (type == network::EntityType::BOSS) {
                        if (m_registry.HasComponent<Drawable>(ecsEntity)) {
                            auto& drawable = m_registry.GetComponent<Drawable>(ecsEntity);
                            if (entityState.flags == 1) {
                                // Red tint flash
                                drawable.tint = {1.0f, 0.3f, 0.3f, 1.0f};
                            } else {
                                // Normal tint
                                drawable.tint = {1.0f, 1.0f, 1.0f, 1.0f};
                            }
                        }

                        if (!m_bossHealthBar.active && entityState.x < 1920.0f) {
                            initializeBossHealthBar();
                            m_bossHealthBar.maxHealth = 100;
                            m_bossHealthBar.bossNetworkId = entityState.entityId;
                        }

                        m_bossHealthBar.currentHealth = static_cast<int>(entityState.health);

                        if (m_bossHealthBar.active && entityState.health == 0) {
                            destroyBossHealthBar();
                        } else {
                            updateBossHealthBar();
                        }
                    }

                    if (type == network::EntityType::PLAYER) {
                        ApplyPowerUpStateToPlayer(ecsEntity, entityState);
                    }
                }
            }

            for (uint32_t entityId : entitiesToRemove) {
                auto it = m_networkEntityMap.find(entityId);
                if (it != m_networkEntityMap.end()) {
                    m_networkEntityMap.erase(it);
                }
            }

            for (auto it = m_networkEntityMap.begin(); it != m_networkEntityMap.end();) {
                if (receivedIds.find(it->first) == receivedIds.end()) {
                    auto ecsEntity = it->second;
                    uint32_t networkId = it->first;

                    DestroyPlayerNameLabel(ecsEntity);

                    for (size_t i = 0; i < MAX_PLAYERS; i++) {
                        if (m_playersHUD[i].playerEntity == ecsEntity) {
                            m_playersHUD[i].isDead = true;
                            m_playersHUD[i].health = 0;
                            m_playersHUD[i].playerEntity = NULL_ENTITY;
                            for (const auto& p : m_context.allPlayers) {
                                if (p.number > 0 && p.number <= MAX_PLAYERS && static_cast<size_t>(p.number - 1) == i) {
                                    m_assignedPlayerNumbers.erase(p.number);
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    if (ecsEntity == m_localPlayerEntity) {
                        size_t localPlayerIndex = static_cast<size_t>(m_context.playerNumber - 1);
                        if (localPlayerIndex < MAX_PLAYERS) {
                            m_playersHUD[localPlayerIndex].isDead = true;
                            m_playersHUD[localPlayerIndex].health = 0;
                            m_playersHUD[localPlayerIndex].playerEntity = NULL_ENTITY;
                        }
                        m_localPlayerEntity = NULL_ENTITY;
                    }

                    if (m_bossHealthBar.active && networkId == m_bossHealthBar.bossNetworkId) {
                        destroyBossHealthBar();
                    }

                    if (m_registry.IsEntityAlive(ecsEntity)) {
                        m_registry.DestroyEntity(ecsEntity);
                    }
                    it = m_networkEntityMap.erase(it);
                } else {
                    ++it;
                }
            }
        }

    }
}
