/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameState - Network state update functions
*/

#include "../../include/GameState.hpp"
#include "../../include/LevelTransitionState.hpp"

#include "ECS/Components/TextLabel.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include "ECS/PowerUpFactory.hpp"
#include "Animation/AnimationModule.hpp"
#include <algorithm>

using namespace RType::ECS;
using namespace Animation;

namespace RType {
    namespace Client {

        void InGameState::OnServerStateUpdate(uint32_t tick, const std::vector<network::EntityState>& entities, const std::vector<network::InputAck>& inputAcks) {
#ifdef DEBUG_NETWORK_VERBOSE
            static int stateLog = 0;
            if (stateLog++ % 60 == 0) {
                auto now = std::chrono::steady_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
                std::cout << "[CLIENT RECV STATE] t=" << ms << " tick=" << tick << " entities=" << entities.size() << std::endl;
            }
#endif

            if (m_context.networkClient) {
                m_serverScrollOffset = m_context.networkClient->GetLastScrollOffset();
            }

            for (const auto& ack : inputAcks) {
                if (ack.playerHash == m_context.playerHash) {
                    ReconcileWithServer(ack);
                    break;
                }
            }

            std::unordered_set<uint32_t> receivedIds;
            std::vector<uint32_t> entitiesToRemove;

            {
                auto localObstacles = m_registry.GetEntitiesWithComponent<ECS::Obstacle>();
                for (auto obsEntity : localObstacles) {
                    if (!m_registry.IsEntityAlive(obsEntity)) {
                        continue;
                    }
                    bool contaminated = m_registry.HasComponent<ECS::Bullet>(obsEntity) ||
                                        m_registry.HasComponent<ECS::Shooter>(obsEntity) ||
                                        m_registry.HasComponent<ECS::WeaponSlot>(obsEntity) ||
                                        m_registry.HasComponent<ECS::Velocity>(obsEntity);
                    if (contaminated) {
                        std::cout << "[CLIENT SCRUB] Obstacle entity " << obsEntity
                                  << " had Bullet/Shooter/WeaponSlot/Velocity; destroying to avoid obstacle bullets."
                                  << std::endl;

                        for (auto itMap = m_networkEntityMap.begin(); itMap != m_networkEntityMap.end();) {
                            if (itMap->second == obsEntity) {
                                itMap = m_networkEntityMap.erase(itMap);
                            } else {
                                ++itMap;
                            }
                        }
                        for (auto itId = m_obstacleIdToCollider.begin(); itId != m_obstacleIdToCollider.end();) {
                            if (itId->second == obsEntity) {
                                itId = m_obstacleIdToCollider.erase(itId);
                            } else {
                                ++itId;
                            }
                        }
                        auto itColl = std::find(m_obstacleColliderEntities.begin(), m_obstacleColliderEntities.end(), obsEntity);
                        if (itColl != m_obstacleColliderEntities.end()) {
                            m_obstacleColliderEntities.erase(itColl);
                        }
                        if (m_registry.IsEntityAlive(obsEntity)) {
                            m_registry.DestroyEntity(obsEntity);
                        }
                    }
                }
            }

            for (const auto& entityState : entities) {
                receivedIds.insert(entityState.entityId);

                network::EntityType type = static_cast<network::EntityType>(entityState.entityType);

                if (auto it = m_networkEntityMap.find(entityState.entityId); it == m_networkEntityMap.end()) {
                    if (type == network::EntityType::PLAYER) {
                        bool isForcePod = (entityState.flags & 0x80) != 0;
                        if (isForcePod) {
                            auto newEntity = m_registry.CreateEntity();
                            CleanupInvalidComponents(newEntity, network::EntityType::PLAYER);
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
                        CleanupInvalidComponents(newEntity, network::EntityType::PLAYER);

                        m_registry.AddComponent<Position>(newEntity, Position{entityState.x, entityState.y});
                        m_registry.AddComponent<Velocity>(newEntity, Velocity{entityState.vx, entityState.vy});
                        m_registry.AddComponent<Health>(newEntity, Health{static_cast<int>(entityState.health), 100});

                        m_registry.AddComponent<Controllable>(newEntity, Controllable{200.0f});
                        m_registry.AddComponent<Shooter>(newEntity, Shooter{0.2f, 50.0f, 20.0f});
                        m_registry.AddComponent<BoxCollider>(newEntity, BoxCollider{25.0f, 25.0f});
                        m_registry.AddComponent<CircleCollider>(newEntity, CircleCollider{12.5f});
                        m_registry.AddComponent<CollisionLayer>(newEntity, CollisionLayer(CollisionLayers::PLAYER, CollisionLayers::ENEMY | CollisionLayers::ENEMY_BULLET | CollisionLayers::OBSTACLE));

                        auto& drawable = m_registry.AddComponent<Drawable>(newEntity, Drawable(playerSprite, 10));
                        drawable.scale = {0.5f, 0.5f};
                        drawable.origin = Math::Vector2(128.0f, 128.0f);

                        auto [playerName, playerNum] = FindPlayerNameAndNumber(entityState.ownerHash, m_assignedPlayerNumbers);
                        uint8_t pNum = (playerNum > 0 && playerNum <= MAX_PLAYERS) ? playerNum : 1;
                        m_registry.AddComponent<Player>(newEntity, Player{pNum, entityState.ownerHash, false});
                        if (playerNum > 0 && playerNum <= MAX_PLAYERS) {
                            m_assignedPlayerNumbers.insert(playerNum);
                        }
                        CreatePlayerNameLabel(newEntity, playerName, entityState.x, entityState.y);

                        ApplyPowerUpStateToPlayer(newEntity, entityState);

                        m_networkEntityMap[entityState.entityId] = newEntity;

                        if (entityState.ownerHash == m_context.playerHash) {
                            m_localPlayerEntity = newEntity;
                            if (m_registry.HasComponent<Player>(newEntity)) {
                                auto& playerComp = m_registry.GetComponent<Player>(newEntity);
                                playerComp.isLocalPlayer = true;
                            }
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
                        CleanupInvalidComponents(newEntity, network::EntityType::ENEMY);

                        m_registry.AddComponent<Position>(newEntity, Position{entityState.x, entityState.y});
                        m_registry.AddComponent<Velocity>(newEntity, Velocity{entityState.vx, entityState.vy});
                        m_registry.AddComponent<Health>(newEntity, Health{static_cast<int>(entityState.health), 100});
                        m_registry.AddComponent<Enemy>(newEntity, Enemy{static_cast<ECS::EnemyType>(enemyType)});
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
                            continue;
                        }

                        auto newEntity = m_registry.CreateEntity();
                        CleanupInvalidComponents(newEntity, network::EntityType::BOSS);

                        m_registry.AddComponent<Position>(newEntity, Position{entityState.x, entityState.y});
                        m_registry.AddComponent<Velocity>(newEntity, Velocity{entityState.vx, entityState.vy});
                        m_registry.AddComponent<Health>(newEntity, Health{static_cast<int>(entityState.health), 1000});

                        auto& drawable = m_registry.AddComponent<Drawable>(newEntity, Drawable(bossSprite, 5));
                        drawable.scale = {3.0f, 3.0f};
                        drawable.origin = Math::Vector2(0.0f, 0.0f);

                        m_networkEntityMap[entityState.entityId] = newEntity;

                        m_bossHealthBar.currentHealth = static_cast<int>(entityState.health);
                        m_bossHealthBar.maxHealth = 1000;
                    } else if (type == network::EntityType::BULLET) {
                        auto newEntity = m_registry.CreateEntity();

                        bool hadObstacle = m_registry.HasComponent<ECS::Obstacle>(newEntity);
                        bool hadObstacleMeta = m_registry.HasComponent<ECS::ObstacleMetadata>(newEntity);
                        bool hadDrawable = m_registry.HasComponent<ECS::Drawable>(newEntity);

                        if (hadObstacle || hadObstacleMeta || hadDrawable) {
                            std::cerr << "[CLIENT BULLET] NEW bullet entity " << newEntity
                                      << " has contaminated components: Obstacle=" << hadObstacle
                                      << " ObstacleMeta=" << hadObstacleMeta
                                      << " Drawable=" << hadDrawable;
                            if (hadDrawable) {
                                const auto& drawable = m_registry.GetComponent<ECS::Drawable>(newEntity);
                                std::cerr << " DrawableSpriteId=" << drawable.spriteId;
                            }
                            std::cerr << std::endl;
                        }

                        CleanupInvalidComponents(newEntity, network::EntityType::BULLET);

                        m_registry.AddComponent<Position>(newEntity, Position{entityState.x, entityState.y});
                        m_registry.AddComponent<Velocity>(newEntity, Velocity{entityState.vx, entityState.vy});

                        static uint64_t bulletSpriteLogCount = 0;
                        const char* chosenSpriteKey = "(none)";
                        Renderer::SpriteId chosenSpriteId = Renderer::INVALID_SPRITE_ID;

                        if (entityState.flags == 15) {
                            auto thirdBulletSpriteIt = m_levelAssets.sprites.find("third_bullet");
                            if (thirdBulletSpriteIt != m_levelAssets.sprites.end()) {
                                chosenSpriteKey = "third_bullet";
                                chosenSpriteId = thirdBulletSpriteIt->second;
                                auto& d = m_registry.AddComponent<Drawable>(newEntity, Drawable(thirdBulletSpriteIt->second, 12));
                                d.scale = {2.5f, 2.5f};
                                d.origin = Math::Vector2(16.0f, 16.0f);
                            } else {
                                Core::Logger::Warning("[GameState] Missing third bullet sprite (entity {})", entityState.entityId);
                                m_registry.DestroyEntity(newEntity);
                                continue;
                            }
                        } else if (entityState.flags == 14) {
                            auto blackOrbSpriteIt = m_levelAssets.sprites.find("black_orb");
                            if (blackOrbSpriteIt != m_levelAssets.sprites.end()) {
                                chosenSpriteKey = "black_orb";
                                chosenSpriteId = blackOrbSpriteIt->second;
                                auto& d = m_registry.AddComponent<Drawable>(newEntity, Drawable(blackOrbSpriteIt->second, 12));
                                d.scale = {2.0f, 2.0f};
                                d.origin = Math::Vector2(20.0f, 20.0f);
                            } else {
                                Core::Logger::Warning("[GameState] Missing black orb sprite (entity {})", entityState.entityId);
                                m_registry.DestroyEntity(newEntity);
                                continue;
                            }
                        } else if (entityState.flags == 13) {
                            auto bossBulletSpriteIt = m_levelAssets.sprites.find("boss_bullet");
                            if (bossBulletSpriteIt != m_levelAssets.sprites.end()) {
                                chosenSpriteKey = "boss_bullet";
                                chosenSpriteId = bossBulletSpriteIt->second;
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

                            chosenSpriteKey = "enemy_bullet(config/bullet)";
                            chosenSpriteId = bulletSprite;
                            auto& d = m_registry.AddComponent<Drawable>(newEntity, Drawable(bulletSprite, 12));
                            d.scale = {scaleValue, scaleValue};
                            d.origin = Math::Vector2(128.0f, 128.0f);
                            d.tint = bulletTint;
                        } else {
                            auto bulletSpriteIt = m_levelAssets.sprites.find("bullet");
                            if (bulletSpriteIt != m_levelAssets.sprites.end()) {
                                chosenSpriteKey = "bullet";
                                chosenSpriteId = bulletSpriteIt->second;
                                auto& d = m_registry.AddComponent<Drawable>(newEntity, Drawable(bulletSpriteIt->second, 12));
                                d.scale = {0.1f, 0.1f};
                                d.origin = Math::Vector2(128.0f, 128.0f);
                                d.tint = {0.2f, 0.8f, 1.0f, 1.0f};
                            }
                        }

                        if (bulletSpriteLogCount < 300 || (bulletSpriteLogCount % 20) == 0) {
                            std::cout << "[CLIENT BULLET SPRITE] netId=" << entityState.entityId
                                      << " flags=" << static_cast<int>(entityState.flags)
                                      << " spriteKey=" << chosenSpriteKey
                                      << " spriteId=" << chosenSpriteId
                                      << std::endl;
                        }
                        bulletSpriteLogCount++;
                        m_networkEntityMap[entityState.entityId] = newEntity;
                        m_bulletFlagsMap[entityState.entityId] = entityState.flags;
                    } else if (type == network::EntityType::POWERUP) {
                        uint8_t powerupType = entityState.flags;
                        ECS::PowerUpType puType = static_cast<ECS::PowerUpType>(powerupType);

                        auto newEntity = m_registry.CreateEntity();
                        CleanupInvalidComponents(newEntity, network::EntityType::POWERUP);

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
                    } else if (type == network::EntityType::OBSTACLE) {
                        uint64_t obstacleId = entityState.ownerHash;
                        auto colliderIt = m_obstacleIdToCollider.find(obstacleId);

                        static int obstacleNotFoundLog = 0;
                        if (colliderIt == m_obstacleIdToCollider.end()) {
                            if (obstacleNotFoundLog < 5) {
                                std::cout << "[CLIENT OBSTACLE MISSING] Received NEW obstacle uniqueId=" << obstacleId
                                          << " but not found in m_obstacleIdToCollider map (size="
                                          << m_obstacleIdToCollider.size() << ")" << std::endl;
                                obstacleNotFoundLog++;
                            }
                            continue;
                        }

                        ECS::Entity colliderEntity = colliderIt->second;

                        if (!m_registry.IsEntityAlive(colliderEntity)) {
                            continue;
                        }

                        if (!m_registry.HasComponent<ECS::Obstacle>(colliderEntity) ||
                            !m_registry.HasComponent<ECS::ObstacleMetadata>(colliderEntity)) {
                            std::cout << "[CLIENT ENTITY REUSE] Obstacle ID " << obstacleId
                                      << " maps to entity " << colliderEntity
                                      << " which is NO LONGER an obstacle - skipping mapping" << std::endl;
                            continue;
                        }

                        m_networkEntityMap[entityState.entityId] = colliderEntity;

                        CleanupInvalidComponents(colliderEntity, network::EntityType::OBSTACLE);
                    }
                } else {
                    auto ecsEntity = it->second;

                    if (type == network::EntityType::BULLET) {
                        bool contaminated = false;
                        if (!m_registry.IsEntityAlive(ecsEntity)) {
                            contaminated = true;
                        } else if (m_registry.HasComponent<ECS::Obstacle>(ecsEntity) ||
                                   m_registry.HasComponent<ECS::ObstacleMetadata>(ecsEntity)) {
                            contaminated = true;
                        }
                        if (contaminated) {
                            if (m_registry.IsEntityAlive(ecsEntity)) {
                                m_registry.DestroyEntity(ecsEntity);
                            }
                            m_networkEntityMap.erase(it);
                            m_bulletFlagsMap.erase(entityState.entityId);
                            std::cout << "[CLIENT BULLET RECREATE] netId=" << entityState.entityId
                                      << " had obstacle/invalid mapping; recreating bullet entity." << std::endl;
                            continue;
                        }
                    }

                    if (type == network::EntityType::BULLET) {
                        auto flagsIt = m_bulletFlagsMap.find(entityState.entityId);
                        bool flagsChanged = false;

                        if (flagsIt != m_bulletFlagsMap.end()) {
                            if (flagsIt->second != entityState.flags) {
                                flagsChanged = true;
                            }
                        }

                        if (flagsChanged || !m_registry.IsEntityAlive(ecsEntity)) {
                            if (m_registry.IsEntityAlive(ecsEntity)) {
                                m_registry.DestroyEntity(ecsEntity);
                            }
                            m_networkEntityMap.erase(it);
                            m_bulletFlagsMap.erase(entityState.entityId);
                            continue;
                        }
                    }

                    if (type == network::EntityType::OBSTACLE) {
                        uint64_t obstacleId = entityState.ownerHash;
                        auto colliderIt = m_obstacleIdToCollider.find(obstacleId);

                        static int obstacleNotFoundLog = 0;
                        if (colliderIt == m_obstacleIdToCollider.end()) {
                            if (obstacleNotFoundLog < 5) {
                                std::cout << "[CLIENT OBSTACLE MISSING] Received obstacle uniqueId=" << obstacleId
                                          << " but not found in m_obstacleIdToCollider map (size="
                                          << m_obstacleIdToCollider.size() << ")" << std::endl;
                                obstacleNotFoundLog++;
                            }
                            continue;
                        }

                        ECS::Entity colliderEntity = colliderIt->second;

                        static int entityReuseLog = 0;
                        if (!m_registry.IsEntityAlive(colliderEntity)) {
                            if (entityReuseLog < 10) {
                                std::cout << "[CLIENT ENTITY REUSE] Obstacle network ID " << entityState.entityId
                                          << " maps to DEAD entity " << colliderEntity << std::endl;
                                entityReuseLog++;
                            }
                            m_networkEntityMap.erase(it);
                            continue;
                        }

                        if (!m_registry.HasComponent<ECS::Obstacle>(colliderEntity)) {
                            if (entityReuseLog < 10) {
                                std::cout << "[CLIENT ENTITY REUSE] Obstacle network ID " << entityState.entityId
                                          << " maps to entity " << colliderEntity << " which is NO LONGER AN OBSTACLE"
                                          << " (now has: "
                                          << (m_registry.HasComponent<ECS::Bullet>(colliderEntity) ? "Bullet " : "")
                                          << (m_registry.HasComponent<ECS::Enemy>(colliderEntity) ? "Enemy " : "")
                                          << (m_registry.HasComponent<ECS::Player>(colliderEntity) ? "Player " : "")
                                          << ")" << std::endl;

                                CleanupInvalidComponents(colliderEntity, network::EntityType::OBSTACLE);

                                entityReuseLog++;
                            }
                            m_networkEntityMap.erase(it);
                            continue;
                        }

                        if (!m_registry.HasComponent<Position>(colliderEntity)) {
                            m_networkEntityMap.erase(it);
                            continue;
                        }

                        static int clientObstacleLog = 0;
                        if (clientObstacleLog < 5) {
                            std::cout << "[CLIENT RECV] Obstacle uniqueId=" << obstacleId
                                      << " entity=" << colliderEntity
                                      << " received pos=(" << entityState.x << "," << entityState.y << ")" << std::endl;
                            clientObstacleLog++;
                        }

                        auto& colliderPos = m_registry.GetComponent<Position>(colliderEntity);
                        colliderPos.x = entityState.x;
                        colliderPos.y = entityState.y;

                        static int scrollableRemovalLog = 0;
                        if (m_registry.HasComponent<Scrollable>(colliderEntity)) {
                            if (scrollableRemovalLog < 10) {
                                std::cout << "[CLIENT] Removing Scrollable from obstacle entity " << colliderEntity << std::endl;
                                scrollableRemovalLog++;
                            }
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
                        if (type == network::EntityType::PLAYER && ecsEntity == m_localPlayerEntity) {

                        } else {
                            auto& pos = m_registry.GetComponent<Position>(ecsEntity);
                            bool useInterpolation = (type == network::EntityType::PLAYER ||
                                                     type == network::EntityType::ENEMY ||
                                                     type == network::EntityType::BOSS);
                            if (useInterpolation && m_isNetworkSession) {
                                auto interpIt = m_interpolationStates.find(entityState.entityId);
                                if (interpIt == m_interpolationStates.end()) {
                                    InterpolationState state;
                                    state.prevX = pos.x;
                                    state.prevY = pos.y;
                                    state.targetX = entityState.x;
                                    state.targetY = entityState.y;
                                    state.interpTime = 0.0f;
                                    state.interpDuration = 1.0f / 60.0f;
                                    m_interpolationStates[entityState.entityId] = state;
                                } else {
                                    interpIt->second.prevX = pos.x;
                                    interpIt->second.prevY = pos.y;
                                    interpIt->second.targetX = entityState.x;
                                    interpIt->second.targetY = entityState.y;
                                    interpIt->second.interpTime = 0.0f;
                                }
                            } else {
                                pos.x = entityState.x;
                                pos.y = entityState.y;
                            }
                        }

                        if (type == network::EntityType::PLAYER) {
                            auto& pos = m_registry.GetComponent<Position>(ecsEntity);
                            UpdatePlayerNameLabelPosition(ecsEntity, pos.x, pos.y);
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

                    if (type == network::EntityType::BOSS) {
                        if (m_registry.HasComponent<Drawable>(ecsEntity)) {
                            auto& drawable = m_registry.GetComponent<Drawable>(ecsEntity);
                            if (entityState.flags == 1) {
                                drawable.tint = {1.0f, 0.3f, 0.3f, 1.0f};
                            } else {
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

            static bool loggedObstacleMapping = false;
            if (!loggedObstacleMapping) {
                int obstaclesInNetworkMap = 0;
                for (const auto& pair : m_networkEntityMap) {
                    if (m_registry.IsEntityAlive(pair.second) &&
                        m_registry.HasComponent<ECS::Obstacle>(pair.second)) {
                        obstaclesInNetworkMap++;
                    }
                }
                std::cout << "[CLIENT NETWORK MAP] m_networkEntityMap has " << obstaclesInNetworkMap
                          << " obstacle entities mapped (total map size: " << m_networkEntityMap.size() << ")" << std::endl;
                loggedObstacleMapping = true;
            }

            for (uint32_t entityId : entitiesToRemove) {
                auto it = m_networkEntityMap.find(entityId);
                if (it != m_networkEntityMap.end()) {
                    m_networkEntityMap.erase(it);
                }
                m_bulletFlagsMap.erase(entityId);
            }

            for (auto it = m_networkEntityMap.begin(); it != m_networkEntityMap.end();) {
                if (receivedIds.find(it->first) == receivedIds.end()) {
                    auto ecsEntity = it->second;
                    uint32_t networkId = it->first;

                    if (m_effectFactory && m_registry.IsEntityAlive(ecsEntity) &&
                        m_registry.HasComponent<Enemy>(ecsEntity) &&
                        m_registry.HasComponent<Position>(ecsEntity)) {
                        const auto& pos = m_registry.GetComponent<Position>(ecsEntity);
                        auto explosionEntity = m_effectFactory->CreateExplosionSmall(m_registry, pos.x, pos.y);
                        
                        if (m_explosionSprite != Renderer::INVALID_SPRITE_ID &&
                            m_registry.HasComponent<Drawable>(explosionEntity)) {
                            auto& drawable = m_registry.GetComponent<Drawable>(explosionEntity);
                            if (drawable.spriteId == Renderer::INVALID_SPRITE_ID) {
                                drawable.spriteId = m_explosionSprite;
                            }
                        }
                        
                        if (m_animationSystem && m_registry.HasComponent<ECS::SpriteAnimation>(explosionEntity)) {
                            auto& anim = m_registry.GetComponent<ECS::SpriteAnimation>(explosionEntity);
                            if (anim.currentRegion.size.x <= 0.0f || anim.currentRegion.size.y <= 0.0f) {
                                if (m_animationModule && anim.clipId != Animation::INVALID_CLIP_ID) {
                                    auto firstFrame = m_animationModule->GetFrameAtTime(anim.clipId, 0.0f, anim.looping);
                                    anim.currentRegion = firstFrame.region;
                                    anim.currentFrameIndex = m_animationModule->GetFrameIndexAtTime(anim.clipId, 0.0f, anim.looping);
                                    if (m_registry.HasComponent<ECS::AnimatedSprite>(explosionEntity)) {
                                        m_registry.GetComponent<ECS::AnimatedSprite>(explosionEntity).needsUpdate = true;
                                    }
                                }
                            }
                        }
                    }

                    DestroyPlayerNameLabel(ecsEntity);

                    if (m_registry.IsEntityAlive(ecsEntity) && m_registry.HasComponent<ECS::Obstacle>(ecsEntity)) {
                        for (auto obsIt = m_obstacleIdToCollider.begin(); obsIt != m_obstacleIdToCollider.end(); ) {
                            if (obsIt->second == ecsEntity) {
                                obsIt = m_obstacleIdToCollider.erase(obsIt);
                            } else {
                                ++obsIt;
                            }
                        }

                        auto colliderIt = std::find(m_obstacleColliderEntities.begin(), m_obstacleColliderEntities.end(), ecsEntity);
                        if (colliderIt != m_obstacleColliderEntities.end()) {
                            m_obstacleColliderEntities.erase(colliderIt);
                        }

                        if (m_registry.HasComponent<ECS::ObstacleMetadata>(ecsEntity)) {
                            const auto& metadata = m_registry.GetComponent<ECS::ObstacleMetadata>(ecsEntity);
                            if (metadata.visualEntity != ECS::NULL_ENTITY &&
                                m_registry.IsEntityAlive(metadata.visualEntity)) {
                                auto spriteIt = std::find(m_obstacleSpriteEntities.begin(), m_obstacleSpriteEntities.end(), metadata.visualEntity);
                                if (spriteIt != m_obstacleSpriteEntities.end()) {
                                    m_obstacleSpriteEntities.erase(spriteIt);
                                }
                                m_registry.DestroyEntity(metadata.visualEntity);
                            }
                        }
                    }

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
                    m_bulletFlagsMap.erase(networkId);
                } else {
                    ++it;
                }
            }
        }

        void InGameState::ReconcileWithServer(const network::InputAck& ack) {
            if (m_localPlayerEntity == ECS::NULL_ENTITY ||
                !m_registry.IsEntityAlive(m_localPlayerEntity) ||
                !m_registry.HasComponent<Position>(m_localPlayerEntity)) {
                return;
            }

            if (ack.lastProcessedSeq > m_lastAckedSequence) {
                m_lastAckedSequence = ack.lastProcessedSeq;
            }

            while (!m_inputHistory.empty() && m_inputHistory.front().sequence <= ack.lastProcessedSeq) {
                m_inputHistory.pop_front();
            }

            auto& pos = m_registry.GetComponent<Position>(m_localPlayerEntity);

            float serverX = ack.serverPosX;
            float serverY = ack.serverPosY;

            constexpr float RECONCILE_THRESHOLD = 1.0f;
            float dx = serverX - m_predictedX;
            float dy = serverY - m_predictedY;
            float errorSq = dx * dx + dy * dy;

            if (m_inputHistory.empty()) {
                if (errorSq > RECONCILE_THRESHOLD * RECONCILE_THRESHOLD) {
                    pos.x = serverX;
                    pos.y = serverY;
                    m_predictedX = serverX;
                    m_predictedY = serverY;
                }
                return;
            }

            constexpr float SNAP_THRESHOLD = 100.0f;
            if (errorSq > SNAP_THRESHOLD * SNAP_THRESHOLD) {
                pos.x = serverX;
                pos.y = serverY;
                m_predictedX = serverX;
                m_predictedY = serverY;

                m_inputHistory.clear();
                return;
            }

            if (errorSq > RECONCILE_THRESHOLD * RECONCILE_THRESHOLD) {
                float replayX = serverX;
                float replayY = serverY;

                for (const auto& input : m_inputHistory) {
                    if (input.inputs & network::InputFlags::UP) {
                        replayY -= PREDICTION_SPEED * input.deltaTime;
                    }
                    if (input.inputs & network::InputFlags::DOWN) {
                        replayY += PREDICTION_SPEED * input.deltaTime;
                    }
                    if (input.inputs & network::InputFlags::LEFT) {
                        replayX -= PREDICTION_SPEED * input.deltaTime;
                    }
                    if (input.inputs & network::InputFlags::RIGHT) {
                        replayX += PREDICTION_SPEED * input.deltaTime;
                    }

                    replayX = std::max(0.0f, std::min(replayX, 1280.0f - 66.0f));
                    replayY = std::max(0.0f, std::min(replayY, 720.0f - 32.0f));
                }

                pos.x = replayX;
                pos.y = replayY;
                m_predictedX = replayX;
                m_predictedY = replayY;
            }

            UpdatePlayerNameLabelPosition(m_localPlayerEntity, pos.x, pos.y);
        }

        void InGameState::OnLevelComplete(uint8_t completedLevel, uint8_t nextLevel) {
            Core::Logger::Info("[InGameState] Level {} complete! Next level: {}",
                              static_cast<int>(completedLevel),
                              static_cast<int>(nextLevel));

            m_levelProgress.levelComplete = true;
            m_levelProgress.bossDefeated = true;

            if (m_context.networkClient) {
                Core::Logger::Info("[InGameState] Stopping network client for transition");
                m_context.networkClient->Stop();
            }

            std::string nextLevelPath = "";
            if (nextLevel > 0) {
                nextLevelPath = "assets/levels/level" + std::to_string(nextLevel) + ".json";
            }

            Core::Logger::Info("[InGameState] Transitioning to level transition screen");
            m_machine.ChangeState(std::make_unique<LevelTransitionState>(
                m_machine,
                m_context,
                static_cast<int>(completedLevel),
                m_playerScore,
                nextLevelPath
            ));
        }

    }
}
