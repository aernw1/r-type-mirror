/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameState
*/

#include "../include/GameState.hpp"

#include "ECS/Components/TextLabel.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include "ECS/PlayerFactory.hpp"
#include <iomanip>
#include <sstream>
#include <cmath>

using namespace RType::ECS;

namespace RType {
    namespace Client {

        InGameState::InGameState(GameStateMachine& machine, GameContext& context, uint32_t seed)
            : m_machine(machine),
              m_context(context),
              m_gameSeed(seed) {
            m_renderer = context.renderer;
        }

        void InGameState::Init() {
            Core::Logger::Info("[GameState] Initializing game");

            if (m_context.networkClient) {
                m_context.networkClient->SetStateCallback([this](uint32_t tick, const std::vector<network::EntityState>& entities) { this->OnServerStateUpdate(tick, entities); });
            } else {
                Core::Logger::Warning("[GameState] No network client available");
            }

            m_isNetworkSession = (m_context.networkClient != nullptr);

            for (const auto& player : m_context.allPlayers) {
                if (player.hash != 0) {
                    m_playerNameMap[player.hash] = player.name;
                }
                m_playerNameMap[static_cast<uint64_t>(player.number)] = player.name;
            }

            loadLevel(m_currentLevelPath);
            createSystems();
            initializeFromLevel();
            if (!m_isNetworkSession) {
                initializePlayers();
            }
            initializeUI();

            Core::Logger::Info("[GameState] Initialization complete");
        }

        void InGameState::loadLevel(const std::string& levelPath) {
            m_levelData = ECS::LevelLoader::LoadFromFile(levelPath);
            m_levelAssets = ECS::LevelLoader::LoadAssets(m_levelData, m_renderer.get());
            Core::Logger::Info("[GameState] Loaded level '{}' with {} textures, {} obstacle definitions",
                               m_levelData.name, m_levelAssets.textures.size(), m_levelData.obstacles.size());

            // Load enemy bullet textures (not in level assets, needed for enemy bullets)
            m_enemyBulletGreenTexture = m_renderer->LoadTexture("assets/projectiles/bullet-green.png");
            if (m_enemyBulletGreenTexture == Renderer::INVALID_TEXTURE_ID) {
                m_enemyBulletGreenTexture = m_renderer->LoadTexture("../assets/projectiles/bullet-green.png");
            }
            if (m_enemyBulletGreenTexture != Renderer::INVALID_TEXTURE_ID) {
                m_enemyBulletGreenSprite = m_renderer->CreateSprite(m_enemyBulletGreenTexture, {});
                Core::Logger::Info("[GameState] Enemy bullet green sprite loaded");
            }

            m_enemyBulletYellowTexture = m_renderer->LoadTexture("assets/projectiles/bullet-yellow.png");
            if (m_enemyBulletYellowTexture == Renderer::INVALID_TEXTURE_ID) {
                m_enemyBulletYellowTexture = m_renderer->LoadTexture("../assets/projectiles/bullet-yellow.png");
            }
            if (m_enemyBulletYellowTexture != Renderer::INVALID_TEXTURE_ID) {
                m_enemyBulletYellowSprite = m_renderer->CreateSprite(m_enemyBulletYellowTexture, {});
                Core::Logger::Info("[GameState] Enemy bullet yellow sprite loaded");
            }

            m_enemyBulletPurpleTexture = m_renderer->LoadTexture("assets/projectiles/bullet-purple.png");
            if (m_enemyBulletPurpleTexture == Renderer::INVALID_TEXTURE_ID) {
                m_enemyBulletPurpleTexture = m_renderer->LoadTexture("../assets/projectiles/bullet-purple.png");
            }
            if (m_enemyBulletPurpleTexture != Renderer::INVALID_TEXTURE_ID) {
                m_enemyBulletPurpleSprite = m_renderer->CreateSprite(m_enemyBulletPurpleTexture, {});
                Core::Logger::Info("[GameState] Enemy bullet purple sprite loaded");
            }

            // Load enemy sprites from level assets or fallback
            auto enemyGreenIt = m_levelAssets.sprites.find("enemy-green");
            if (enemyGreenIt != m_levelAssets.sprites.end()) {
                m_enemyGreenSprite = enemyGreenIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("enemy-green");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_enemyGreenSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto enemyRedIt = m_levelAssets.sprites.find("enemy-red");
            if (enemyRedIt != m_levelAssets.sprites.end()) {
                m_enemyRedSprite = enemyRedIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("enemy-red");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_enemyRedSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto enemyBlueIt = m_levelAssets.sprites.find("enemy-blue");
            if (enemyBlueIt != m_levelAssets.sprites.end()) {
                m_enemyBlueSprite = enemyBlueIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("enemy-blue");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_enemyBlueSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto ensureEnemySprite = [this](Renderer::SpriteId& target,
                                            Renderer::SpriteId fallbackPlayerSprite,
                                            const std::string& path) {
                if (target != Renderer::INVALID_SPRITE_ID) {
                    return;
                }
                Renderer::TextureId textureId = m_renderer->LoadTexture(path);
                if (textureId == Renderer::INVALID_TEXTURE_ID) {
                    textureId = m_renderer->LoadTexture("../" + path);
                }
                if (textureId != Renderer::INVALID_TEXTURE_ID) {
                    target = m_renderer->CreateSprite(textureId, {});
                } else if (fallbackPlayerSprite != Renderer::INVALID_SPRITE_ID) {
                    target = fallbackPlayerSprite;
                }
            };

            ensureEnemySprite(m_enemyGreenSprite, m_playerGreenSprite, "assets/spaceships/enemy-green.png");
            ensureEnemySprite(m_enemyRedSprite, m_playerRedSprite, "assets/spaceships/enemy-red.png");
            ensureEnemySprite(m_enemyBlueSprite, m_playerBlueSprite, "assets/spaceships/enemy-blue.png");

            // Load power-up sprites from level assets
            auto powerupSpreadIt = m_levelAssets.sprites.find("powerup-spread");
            if (powerupSpreadIt != m_levelAssets.sprites.end()) {
                m_powerupSpreadSprite = powerupSpreadIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("powerup-spread");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_powerupSpreadSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto powerupLaserIt = m_levelAssets.sprites.find("powerup-laser");
            if (powerupLaserIt != m_levelAssets.sprites.end()) {
                m_powerupLaserSprite = powerupLaserIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("powerup-laser");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_powerupLaserSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto powerupForcePodIt = m_levelAssets.sprites.find("powerup-force-pod");
            if (powerupForcePodIt != m_levelAssets.sprites.end()) {
                m_powerupForcePodSprite = powerupForcePodIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("powerup-force-pod");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_powerupForcePodSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto powerupSpeedIt = m_levelAssets.sprites.find("powerup-speed");
            if (powerupSpeedIt != m_levelAssets.sprites.end()) {
                m_powerupSpeedSprite = powerupSpeedIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("powerup-speed");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_powerupSpeedSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto powerupShieldIt = m_levelAssets.sprites.find("powerup-shield");
            if (powerupShieldIt != m_levelAssets.sprites.end()) {
                m_powerupShieldSprite = powerupShieldIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("powerup-shield");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_powerupShieldSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }
        }

        void InGameState::initializeFromLevel() {
            m_levelEntities = ECS::LevelLoader::CreateEntities(
                m_registry,
                m_levelData,
                m_levelAssets,
                m_renderer.get());

            m_backgroundEntities = m_levelEntities.backgrounds;
            m_obstacleSpriteEntities = m_levelEntities.obstacleVisuals;
            m_obstacleColliderEntities = m_levelEntities.obstacleColliders;
            m_obstacleIdToCollider.clear();
            for (auto collider : m_obstacleColliderEntities) {
                if (!m_registry.IsEntityAlive(collider) ||
                    !m_registry.HasComponent<ECS::ObstacleMetadata>(collider)) {
                    continue;
                }
                const auto& metadata = m_registry.GetComponent<ECS::ObstacleMetadata>(collider);
                m_obstacleIdToCollider[metadata.uniqueId] = collider;
            }
        }

        void InGameState::createSystems() {
            auto bulletSpriteIt = m_levelAssets.sprites.find("bullet");
            Renderer::SpriteId bulletSprite = (bulletSpriteIt != m_levelAssets.sprites.end())
                                                  ? bulletSpriteIt->second
                                                  : Renderer::INVALID_SPRITE_ID;

            if (bulletSprite == Renderer::INVALID_SPRITE_ID) {
                Core::Logger::Error("[GameState] Bullet sprite not found in level assets");
            }

            m_scrollingSystem = std::make_unique<RType::ECS::ScrollingSystem>();
            m_renderingSystem = std::make_unique<RType::ECS::RenderingSystem>(m_renderer.get());
            m_textSystem = std::make_unique<RType::ECS::TextRenderingSystem>(m_renderer.get());

            m_powerUpSpawnSystem = std::make_unique<RType::ECS::PowerUpSpawnSystem>(
                m_renderer.get(),
                m_levelData.config.screenWidth,
                m_levelData.config.screenHeight);
            m_powerUpSpawnSystem->SetSpawnInterval(m_levelData.config.powerUpSpawnInterval);
            m_powerUpCollisionSystem = std::make_unique<RType::ECS::PowerUpCollisionSystem>(m_renderer.get());
            m_forcePodSystem = std::make_unique<RType::ECS::ForcePodSystem>();
            m_shieldSystem = std::make_unique<RType::ECS::ShieldSystem>();
            // Client still runs these systems for local VFX even when the server owns power-up logic.

            if (!m_isNetworkSession) {
                m_shootingSystem = std::make_unique<RType::ECS::ShootingSystem>(bulletSprite);
                m_movementSystem = std::make_unique<RType::ECS::MovementSystem>();
                m_inputSystem = std::make_unique<RType::ECS::InputSystem>(m_renderer.get());
                m_collisionDetectionSystem = std::make_unique<RType::ECS::CollisionDetectionSystem>();
                m_bulletResponseSystem = std::make_unique<RType::ECS::BulletCollisionResponseSystem>();
                m_playerResponseSystem = std::make_unique<RType::ECS::PlayerCollisionResponseSystem>();
                m_obstacleResponseSystem = std::make_unique<RType::ECS::ObstacleCollisionResponseSystem>();
                m_healthSystem = std::make_unique<RType::ECS::HealthSystem>();
            } else {
                m_shootingSystem.reset();
                m_movementSystem.reset();
                m_inputSystem.reset();
                m_collisionDetectionSystem.reset();
                m_bulletResponseSystem.reset();
                m_playerResponseSystem.reset();
                m_obstacleResponseSystem.reset();
                m_healthSystem.reset();
            }
        }

        void InGameState::initializePlayers() {
            if (m_isNetworkSession) {
                return;
            }
            std::cout << "[GameState] Initializing local player (ECS)..." << std::endl;

            const uint8_t playerNumber = m_context.playerNumber == 0 ? 1 : m_context.playerNumber;
            const uint64_t playerHash = m_context.playerHash;

            // Get player spawn position from level data
            const auto& spawns = ECS::LevelLoader::GetPlayerSpawns(m_levelData);
            uint8_t playerIndex = playerNumber - 1;

            Position spawnPos{100.0f, 360.0f}; // Fallback default
            if (!spawns.empty()) {
                if (playerIndex < spawns.size()) {
                    spawnPos = Position{spawns[playerIndex].x, spawns[playerIndex].y};
                    Core::Logger::Info("[GameState] Player {} spawning at ({}, {}) from level data",
                                       playerNumber, spawnPos.x, spawnPos.y);
                } else {
                    // Use first spawn as fallback
                    spawnPos = Position{spawns[0].x, spawns[0].y};
                    Core::Logger::Warning("[GameState] Player {} index out of range, using first spawn",
                                          playerNumber);
                }
            } else {
                Core::Logger::Warning("[GameState] No player spawns in level data, using default position");
            }

            const auto& playerConfig = m_levelData.config.playerDefaults;

            m_localPlayerEntity = ECS::PlayerFactory::CreatePlayer(m_registry,
                                                                   playerNumber,
                                                                   playerHash,
                                                                   spawnPos.x,
                                                                   spawnPos.y,
                                                                   m_renderer.get());

            if (m_localPlayerEntity == ECS::NULL_ENTITY) {
                Core::Logger::Error("[GameState] Failed to create local player entity");
                return;
            }

            if (m_registry.HasComponent<ECS::Player>(m_localPlayerEntity)) {
                auto& playerComp = m_registry.GetComponent<ECS::Player>(m_localPlayerEntity);
                playerComp.isLocalPlayer = true;
            } else {
                m_registry.AddComponent<Player>(m_localPlayerEntity, Player{playerNumber, playerHash, true});
            }

            if (m_registry.HasComponent<ECS::Position>(m_localPlayerEntity)) {
                auto& position = m_registry.GetComponent<ECS::Position>(m_localPlayerEntity);
                position = spawnPos;
            } else {
                m_registry.AddComponent<Position>(m_localPlayerEntity, Position{spawnPos.x, spawnPos.y});
            }

            if (!m_registry.HasComponent<Velocity>(m_localPlayerEntity)) {
                m_registry.AddComponent<Velocity>(m_localPlayerEntity, Velocity{0.0f, 0.0f});
            }

            if (m_registry.HasComponent<ECS::Controllable>(m_localPlayerEntity)) {
                auto& controllable = m_registry.GetComponent<ECS::Controllable>(m_localPlayerEntity);
                controllable.speed = playerConfig.movementSpeed;
            } else {
                m_registry.AddComponent<Controllable>(m_localPlayerEntity, Controllable{playerConfig.movementSpeed});
            }

            Shooter shooterConfig{
                playerConfig.fireRate,
                playerConfig.bulletOffsetX,
                playerConfig.bulletOffsetY};
            if (m_registry.HasComponent<Shooter>(m_localPlayerEntity)) {
                auto& shooter = m_registry.GetComponent<Shooter>(m_localPlayerEntity);
                shooter = shooterConfig;
            } else {
                m_registry.AddComponent<Shooter>(m_localPlayerEntity, Shooter{shooterConfig.fireRate, shooterConfig.offsetX, shooterConfig.offsetY});
            }

            if (m_registry.HasComponent<ShootCommand>(m_localPlayerEntity)) {
                auto& shootCommand = m_registry.GetComponent<ShootCommand>(m_localPlayerEntity);
                shootCommand = ShootCommand{};
            } else {
                m_registry.AddComponent<ShootCommand>(m_localPlayerEntity, ShootCommand{});
            }

            if (m_registry.HasComponent<Health>(m_localPlayerEntity)) {
                auto& health = m_registry.GetComponent<Health>(m_localPlayerEntity);
                health.max = playerConfig.maxHealth;
                if (health.current > playerConfig.maxHealth) {
                    health.current = playerConfig.maxHealth;
                }
            } else {
                m_registry.AddComponent<Health>(m_localPlayerEntity, Health{playerConfig.maxHealth});
            }

            auto spriteIt = m_levelAssets.sprites.find("player_blue");
            if (spriteIt != m_levelAssets.sprites.end()) {
                if (m_registry.HasComponent<Drawable>(m_localPlayerEntity)) {
                    auto& drawable = m_registry.GetComponent<Drawable>(m_localPlayerEntity);
                    drawable.spriteId = spriteIt->second;
                    drawable.layer = 10;
                    drawable.scale = {0.5f, 0.5f};
                } else {
                    auto& drawable = m_registry.AddComponent<Drawable>(m_localPlayerEntity, Drawable(spriteIt->second, 10));
                    drawable.scale = {0.5f, 0.5f};
                }
            }
        }

        void InGameState::initializeUI() {
            m_hudFont = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 16);
            if (m_hudFont == Renderer::INVALID_FONT_ID) {
                m_hudFont = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 16);
            }

            m_hudFontSmall = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 12);
            if (m_hudFontSmall == Renderer::INVALID_FONT_ID) {
                m_hudFontSmall = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 12);
            }

            if (m_hudFont == Renderer::INVALID_FONT_ID) {
                std::cerr << "[GameState] Error: Failed to load HUD font!" << std::endl;
                return;
            }

            m_hudPlayerEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_hudPlayerEntity, Position{20.0f, 680.0f});
            std::string playerText = "P" + std::to_string(m_context.playerNumber);
            TextLabel playerLabel(playerText, m_hudFont, 16);
            playerLabel.color = {0.0f, 1.0f, 1.0f, 1.0f};
            m_registry.AddComponent<TextLabel>(m_hudPlayerEntity, std::move(playerLabel));

            m_hudLivesEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_hudLivesEntity, Position{350.0f, 680.0f});
            TextLabel livesLabel("LIVES x3", m_hudFontSmall != Renderer::INVALID_FONT_ID ? m_hudFontSmall : m_hudFont, 12);
            livesLabel.color = {1.0f, 0.8f, 0.0f, 1.0f};
            m_registry.AddComponent<TextLabel>(m_hudLivesEntity, std::move(livesLabel));

            m_hudScoreboardTitle = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_hudScoreboardTitle, Position{1050.0f, 620.0f});
            TextLabel titleLabel("SCORES", m_hudFontSmall != Renderer::INVALID_FONT_ID ? m_hudFontSmall : m_hudFont, 12);
            titleLabel.color = {0.0f, 1.0f, 1.0f, 1.0f};
            m_registry.AddComponent<TextLabel>(m_hudScoreboardTitle, std::move(titleLabel));

            for (size_t i = 0; i < MAX_PLAYERS; i++) {
                m_playersHUD[i].active = false;
                m_playersHUD[i].score = 0;
                m_playersHUD[i].lives = 3;
                m_playersHUD[i].health = 100;
                m_playersHUD[i].maxHealth = 100;
                m_playersHUD[i].playerEntity = NULL_ENTITY;

                m_playersHUD[i].scoreEntity = m_registry.CreateEntity();
                float yPos = 645.0f + (i * 20.0f);
                m_registry.AddComponent<Position>(m_playersHUD[i].scoreEntity, Position{1050.0f, yPos});

                std::string scoreText = "P" + std::to_string(i + 1) + " --------";
                TextLabel label(scoreText, m_hudFontSmall != Renderer::INVALID_FONT_ID ? m_hudFontSmall : m_hudFont, 12);
                label.color = {0.4f, 0.4f, 0.4f, 0.6f};
                m_registry.AddComponent<TextLabel>(m_playersHUD[i].scoreEntity, std::move(label));
            }

            if (m_context.playerNumber >= 1 && m_context.playerNumber <= MAX_PLAYERS) {
                m_playersHUD[m_context.playerNumber - 1].active = true;
            }

            std::cout << "[GameState] HUD initialized for P" << (int)m_context.playerNumber << std::endl;
        }

        void InGameState::updateHUD() {
            if (m_hudScoreEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_hudScoreEntity)) {
                auto& scoreLabel = m_registry.GetComponent<TextLabel>(m_hudScoreEntity);
                std::ostringstream ss;
                ss << std::setw(8) << std::setfill('0') << m_playerScore;
                scoreLabel.text = ss.str();
            }

            if (m_hudLivesEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_hudLivesEntity)) {
                auto& livesLabel = m_registry.GetComponent<TextLabel>(m_hudLivesEntity);
                livesLabel.text = "LIVES x" + std::to_string(m_playerLives);

                if (m_playerLives <= 1) {
                    livesLabel.color = {1.0f, 0.2f, 0.2f, 1.0f};
                } else if (m_playerLives <= 2) {
                    livesLabel.color = {1.0f, 0.6f, 0.0f, 1.0f};
                } else {
                    livesLabel.color = {1.0f, 0.8f, 0.0f, 1.0f};
                }
            }

            if (m_context.playerNumber >= 1 && m_context.playerNumber <= MAX_PLAYERS) {
                m_playersHUD[m_context.playerNumber - 1].score = m_playerScore;
                m_playersHUD[m_context.playerNumber - 1].lives = m_playerLives;
            }

            const Math::Color playerColors[MAX_PLAYERS] = {
                {0.2f, 1.0f, 0.2f, 1.0f}, // P1 - green
                {0.2f, 0.6f, 1.0f, 1.0f}, // P2 - blue
                {1.0f, 0.3f, 0.3f, 1.0f}, // P3 - Red
                {1.0f, 1.0f, 0.2f, 1.0f}  // P4 - yellow
            };

            for (size_t i = 0; i < MAX_PLAYERS; i++) {
                if (m_playersHUD[i].scoreEntity == NULL_ENTITY ||
                    !m_registry.IsEntityAlive(m_playersHUD[i].scoreEntity)) {
                    continue;
                }

                auto& label = m_registry.GetComponent<TextLabel>(m_playersHUD[i].scoreEntity);

                if (m_playersHUD[i].active) {
                    if (m_playersHUD[i].isDead) {
                        m_playersHUD[i].health = 0;
                    } else {
                        bool entityExists = false;
                        if (m_playersHUD[i].playerEntity != NULL_ENTITY &&
                            m_registry.IsEntityAlive(m_playersHUD[i].playerEntity) &&
                            m_registry.HasComponent<Health>(m_playersHUD[i].playerEntity)) {
                            const auto& health = m_registry.GetComponent<Health>(m_playersHUD[i].playerEntity);
                            if (health.current > 0) {
                                m_playersHUD[i].health = health.current;
                                m_playersHUD[i].maxHealth = health.max;
                            } else {
                                m_playersHUD[i].isDead = true;
                                m_playersHUD[i].health = 0;
                            }
                            entityExists = true;
                        } else if (i == static_cast<size_t>(m_context.playerNumber - 1) &&
                                   m_localPlayerEntity != ECS::NULL_ENTITY &&
                                   m_registry.IsEntityAlive(m_localPlayerEntity) &&
                                   m_registry.HasComponent<Health>(m_localPlayerEntity)) {
                            const auto& health = m_registry.GetComponent<Health>(m_localPlayerEntity);
                            if (health.current > 0) {
                                m_playersHUD[i].health = health.current;
                                m_playersHUD[i].maxHealth = health.max;
                            } else {
                                m_playersHUD[i].isDead = true;
                                m_playersHUD[i].health = 0;
                            }
                            entityExists = true;

                            if (m_playersHUD[i].playerEntity == NULL_ENTITY) {
                                m_playersHUD[i].playerEntity = m_localPlayerEntity;
                            }
                        }

                        if (!entityExists && m_playersHUD[i].health > 0) {
                            m_playersHUD[i].isDead = true;
                            m_playersHUD[i].health = 0;
                        } else if (!entityExists) {
                            m_playersHUD[i].isDead = true;
                            m_playersHUD[i].health = 0;
                        }
                    }

                    // Format: "P1 00000000"
                    std::ostringstream ss;
                    ss << "P" << (i + 1) << " " << std::setw(8) << std::setfill('0') << m_playersHUD[i].score;
                    label.text = ss.str();
                    label.color = playerColors[i];

                    if (i == static_cast<size_t>(m_context.playerNumber - 1)) {
                        label.color.a = 1.0f;
                    } else {
                        label.color.a = 0.85f;
                    }
                } else {
                    label.text = "P" + std::to_string(i + 1) + " --------";
                    label.color = {0.4f, 0.4f, 0.4f, 0.5f};
                }
            }
        }

        void InGameState::HandleInput() {
            m_currentInputs = 0;

            if (m_renderer->IsKeyPressed(Renderer::Key::Up)) {
                m_currentInputs |= network::InputFlags::UP;
            }
            if (m_renderer->IsKeyPressed(Renderer::Key::Down)) {
                m_currentInputs |= network::InputFlags::DOWN;
            }
            if (m_renderer->IsKeyPressed(Renderer::Key::Left)) {
                m_currentInputs |= network::InputFlags::LEFT;
            }
            if (m_renderer->IsKeyPressed(Renderer::Key::Right)) {
                m_currentInputs |= network::InputFlags::RIGHT;
            }
            if (m_renderer->IsKeyPressed(Renderer::Key::Space)) {
                if (!m_isCharging) {
                    m_isCharging = true;
                    m_chargeTime = 0.0f;
                }
            } else if (m_isCharging) {
                m_currentInputs |= network::InputFlags::SHOOT;
                m_isCharging = false;
                m_chargeTime = 0.0f;
            }

            if (!m_isNetworkSession &&
                m_localPlayerEntity != ECS::NULL_ENTITY &&
                m_registry.HasComponent<ShootCommand>(m_localPlayerEntity)) {
                auto& shootCmd = m_registry.GetComponent<ShootCommand>(m_localPlayerEntity);
                shootCmd.wantsToShoot = (m_currentInputs & network::InputFlags::SHOOT);
            }
            if (m_context.networkClient && m_currentInputs != m_previousInputs) {
                auto now = std::chrono::steady_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastInputTime).count();

                if (ms >= 30) {
                    m_context.networkClient->SendInput(static_cast<uint8_t>(m_currentInputs));
                    m_lastInputTime = now;

                    static int inputLog = 0;
                    if (inputLog++ % 10 == 0) {
                        auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
                        std::cout << "[CLIENT SEND INPUT] t=" << epoch << " inputs=" << m_currentInputs << std::endl;
                    }
                }
                m_previousInputs = m_currentInputs;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escapeKeyPressed) {
                m_escapeKeyPressed = true;
                std::cout << "[GameState] Returning to lobby..." << std::endl;
                m_machine.PopState();
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                m_escapeKeyPressed = false;
            }
        }

        void InGameState::Draw() {
            m_renderingSystem->Update(m_registry, 0.0f);
            m_textSystem->Update(m_registry, 0.0f);

            renderChargeBar();
            renderHealthBars();
        }

        void InGameState::renderChargeBar() {
            const float barX = 500.0f;
            const float barY = 675.0f;
            const float barWidth = 200.0f;
            const float barHeight = 20.0f;

            Renderer::Rectangle bgRect;
            bgRect.position = Renderer::Vector2(barX - 2, barY - 2);
            bgRect.size = Renderer::Vector2(barWidth + 4, barHeight + 4);
            m_renderer->DrawRectangle(bgRect, Renderer::Color(0.2f, 0.2f, 0.2f, 0.8f));

            Renderer::Rectangle innerBgRect;
            innerBgRect.position = Renderer::Vector2(barX, barY);
            innerBgRect.size = Renderer::Vector2(barWidth, barHeight);
            m_renderer->DrawRectangle(innerBgRect, Renderer::Color(0.1f, 0.1f, 0.1f, 0.9f));

            float chargePercent = m_chargeTime / MAX_CHARGE_TIME;
            float filledWidth = barWidth * chargePercent;

            Renderer::Color barColor;
            if (chargePercent < 0.5f) {
                float blend = chargePercent * 2.0f;
                barColor = Renderer::Color(blend, 1.0f, 1.0f - blend, 1.0f);
            } else {
                float blend = (chargePercent - 0.5f) * 2.0f;
                barColor = Renderer::Color(1.0f, 1.0f - blend * 0.5f, 0.0f, 1.0f);
            }

            if (filledWidth > 0) {
                Renderer::Rectangle fillRect;
                fillRect.position = Renderer::Vector2(barX, barY);
                fillRect.size = Renderer::Vector2(filledWidth, barHeight);
                m_renderer->DrawRectangle(fillRect, barColor);
            }

            if (m_isCharging && m_hudFontSmall != Renderer::INVALID_FONT_ID) {
                Renderer::TextParams textParams;
                textParams.position = Renderer::Vector2(barX + barWidth + 10, barY + 2);
                textParams.color = Renderer::Color(0.0f, 1.0f, 1.0f, 1.0f);
                textParams.scale = 1.0f;

                if (m_chargeTime >= MAX_CHARGE_TIME) {
                    float pulse = 0.7f + 0.3f * std::sin(m_chargeTime * 10.0f);
                    textParams.color = Renderer::Color(1.0f, pulse, 0.0f, 1.0f);
                    m_renderer->DrawText(m_hudFontSmall, "MAX!", textParams);
                } else {
                    m_renderer->DrawText(m_hudFontSmall, "BEAM", textParams);
                }
            }
        }

        void InGameState::renderHealthBars() {
            if (m_context.playerNumber < 1 || m_context.playerNumber > MAX_PLAYERS) {
                return;
            }

            size_t playerIndex = static_cast<size_t>(m_context.playerNumber - 1);
            if (!m_playersHUD[playerIndex].active) {
                return;
            }

            const float barWidth = 150.0f;
            const float barHeight = 12.0f;
            const float barX = 180.0f;
            const float barY = 680.0f;

            Renderer::Rectangle bgRect;
            bgRect.position = Renderer::Vector2(barX - 2, barY - 2);
            bgRect.size = Renderer::Vector2(barWidth + 4, barHeight + 4);
            m_renderer->DrawRectangle(bgRect, Renderer::Color(0.1f, 0.1f, 0.1f, 0.9f));

            Renderer::Rectangle innerBgRect;
            innerBgRect.position = Renderer::Vector2(barX, barY);
            innerBgRect.size = Renderer::Vector2(barWidth, barHeight);
            m_renderer->DrawRectangle(innerBgRect, Renderer::Color(0.3f, 0.1f, 0.1f, 0.8f));

            Renderer::TextParams textParams;
            textParams.position = Renderer::Vector2(barX, barY - barHeight - 2);
            textParams.color = Renderer::Color(0.0f, 1.0f, 1.0f, 1.0f);
            textParams.scale = 1.0f;
            m_renderer->DrawText(m_hudFontSmall, "HEALTH", textParams);

            float healthPercent = 0.0f;
            if (m_playersHUD[playerIndex].isDead) {
                healthPercent = 0.0f;
            } else if (m_playersHUD[playerIndex].maxHealth > 0) {
                healthPercent = static_cast<float>(m_playersHUD[playerIndex].health) / static_cast<float>(m_playersHUD[playerIndex].maxHealth);
                healthPercent = std::max(0.0f, std::min(1.0f, healthPercent));
            }

            if (m_playersHUD[playerIndex].isDead || m_playersHUD[playerIndex].health <= 0) {
                healthPercent = 0.0f;
            }

            float filledWidth = barWidth * healthPercent;

            Renderer::Color healthColor;
            if (healthPercent <= 0.3f) {
                healthColor = Renderer::Color(1.0f, 0.2f, 0.2f, 1.0f);
            } else if (healthPercent <= 0.6f) {
                float blend = (healthPercent - 0.3f) / 0.3f;
                healthColor = Renderer::Color(
                    1.0f,
                    0.2f + blend * 0.8f,
                    0.2f,
                    1.0f);
            } else {
                healthColor = Renderer::Color(0.2f, 1.0f, 0.2f, 1.0f);
            }

            if (filledWidth > 0) {
                Renderer::Rectangle fillRect;
                fillRect.position = Renderer::Vector2(barX, barY);
                fillRect.size = Renderer::Vector2(filledWidth, barHeight);
                m_renderer->DrawRectangle(fillRect, healthColor);
            }
        }

        void InGameState::Cleanup() {
            std::cout << "[GameState] Cleaning up game state..." << std::endl;

            for (auto& bg : m_backgroundEntities) {
                if (m_registry.IsEntityAlive(bg)) {
                    m_registry.DestroyEntity(bg);
                }
            }
            m_backgroundEntities.clear();

            for (auto& obstacle : m_obstacleSpriteEntities) {
                if (m_registry.IsEntityAlive(obstacle)) {
                    m_registry.DestroyEntity(obstacle);
                }
            }
            m_obstacleSpriteEntities.clear();

            for (auto& obstacle : m_obstacleColliderEntities) {
                if (m_registry.IsEntityAlive(obstacle)) {
                    m_registry.DestroyEntity(obstacle);
                }
            }
            m_obstacleColliderEntities.clear();
            m_obstacleIdToCollider.clear();

            if (m_hudPlayerEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_hudPlayerEntity)) {
                m_registry.DestroyEntity(m_hudPlayerEntity);
            }
            if (m_hudScoreEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_hudScoreEntity)) {
                m_registry.DestroyEntity(m_hudScoreEntity);
            }
            if (m_hudLivesEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_hudLivesEntity)) {
                m_registry.DestroyEntity(m_hudLivesEntity);
            }
            if (m_hudScoreboardTitle != NULL_ENTITY && m_registry.IsEntityAlive(m_hudScoreboardTitle)) {
                m_registry.DestroyEntity(m_hudScoreboardTitle);
            }
            for (auto& playerHUD : m_playersHUD) {
                if (playerHUD.scoreEntity != NULL_ENTITY && m_registry.IsEntityAlive(playerHUD.scoreEntity)) {
                    m_registry.DestroyEntity(playerHUD.scoreEntity);
                }
            }

            for (auto& [playerEntity, labelEntity] : m_playerNameLabels) {
                if (m_registry.IsEntityAlive(labelEntity)) {
                    m_registry.DestroyEntity(labelEntity);
                }
            }
            m_playerNameLabels.clear();
        }

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
                        
                        // Add necessary components for player functionality
                        m_registry.AddComponent<Controllable>(newEntity, Controllable{200.0f});
                        m_registry.AddComponent<Shooter>(newEntity, Shooter{0.2f, 50.0f, 20.0f});
                        m_registry.AddComponent<BoxCollider>(newEntity, BoxCollider{50.0f, 50.0f});
                        m_registry.AddComponent<CircleCollider>(newEntity, CircleCollider{25.0f});
                        m_registry.AddComponent<CollisionLayer>(newEntity,
                            CollisionLayer(CollisionLayers::PLAYER,
                                          CollisionLayers::ENEMY | CollisionLayers::ENEMY_BULLET | CollisionLayers::OBSTACLE));

                        auto& drawable = m_registry.AddComponent<Drawable>(newEntity, Drawable(playerSprite, 10));
                        drawable.scale = {0.5f, 0.5f};
                        drawable.origin = Math::Vector2(128.0f, 128.0f);

                        auto [playerName, playerNum] = FindPlayerNameAndNumber(entityState.ownerHash, m_assignedPlayerNumbers);
                        if (playerNum > 0 && playerNum <= MAX_PLAYERS) {
                            m_assignedPlayerNumbers.insert(playerNum);
                        }
                        CreatePlayerNameLabel(newEntity, playerName, entityState.x, entityState.y);

                        // Apply power-up state from server
                        ApplyPowerUpStateToPlayer(newEntity, entityState);

                        m_networkEntityMap[entityState.entityId] = newEntity;

                        if (entityState.ownerHash == m_context.playerHash) {
                            m_localPlayerEntity = newEntity;
                            // Ensure local player has ShootCommand for shooting
                            if (!m_registry.HasComponent<ShootCommand>(newEntity)) {
                                m_registry.AddComponent<ShootCommand>(newEntity, ShootCommand{});
                            }
                            if (m_context.playerNumber >= 1 && m_context.playerNumber <= MAX_PLAYERS) {
                                size_t localPlayerIndex = static_cast<size_t>(m_context.playerNumber - 1);
                                m_playersHUD[localPlayerIndex].playerEntity = newEntity;
                                m_playersHUD[localPlayerIndex].active = true;
                                if (entityState.health > 0) {
                                    m_playersHUD[localPlayerIndex].isDead = false;
                                }
                            }
                        } else if (playerNum > 0 && playerNum <= MAX_PLAYERS) {
                            size_t hudPlayerIndex = static_cast<size_t>(playerNum - 1);
                            m_playersHUD[hudPlayerIndex].active = true;
                            m_playersHUD[hudPlayerIndex].playerEntity = newEntity;
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
                            Core::Logger::Error("[GameState] Missing enemy sprite for type {}, skipping entity {}",
                                                static_cast<int>(enemyType),
                                                entityState.entityId);
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
                    } else if (type == network::EntityType::BULLET) {
                        auto newEntity = m_registry.CreateEntity();
                        m_registry.AddComponent<Position>(newEntity, Position{entityState.x, entityState.y});
                        m_registry.AddComponent<Velocity>(newEntity, Velocity{entityState.vx, entityState.vy});

                        if (entityState.flags >= 10) {
                            // Enemy bullet
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
                                Core::Logger::Warning("[GameState] Missing enemy bullet sprite for type {} (entity {})",
                                                      static_cast<int>(enemyType),
                                                      entityState.entityId);
                                m_registry.DestroyEntity(newEntity);
                                continue;
                            }

                            auto& d = m_registry.AddComponent<Drawable>(newEntity, Drawable(bulletSprite, 12));
                            d.scale = {scaleValue, scaleValue};
                            d.origin = Math::Vector2(128.0f, 128.0f);
                            d.tint = bulletTint;
                        } else {
                            // Player bullet
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

                        // Select appropriate power-up sprite based on type
                        Renderer::SpriteId powerupSprite = Renderer::INVALID_SPRITE_ID;
                        switch (puType) {
                            case ECS::PowerUpType::FIRE_RATE_BOOST:
                                // Use spread sprite for fire rate boost as fallback
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
                                // Fallback to first available power-up sprite
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
                        std::cout << "[GameState] Created POWERUP entity " << entityState.entityId
                                  << " type " << static_cast<int>(powerupType) << std::endl;
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

                    if (m_registry.IsEntityAlive(ecsEntity)) {
                        m_registry.DestroyEntity(ecsEntity);
                    }
                    it = m_networkEntityMap.erase(it);
                } else {
                    ++it;
                }
            }
        }

        void InGameState::Update(float dt) {
            if (m_context.networkClient) {
                m_context.networkClient->ReceivePackets();
            }

            m_scoreAccumulator += dt * 100.0f;
            if (m_scoreAccumulator >= 1.0f) {
                int points = static_cast<int>(m_scoreAccumulator);
                m_playerScore += points;
                m_scoreAccumulator -= points;
            }

            if (m_isCharging) {
                m_chargeTime += dt;
                if (m_chargeTime > MAX_CHARGE_TIME) {
                    m_chargeTime = MAX_CHARGE_TIME;
                }
            }
            if (m_inputSystem) {
                m_inputSystem->Update(m_registry, dt);
            }
            if (m_movementSystem) {
                m_movementSystem->Update(m_registry, dt);
            }

            auto entities = m_registry.GetEntitiesWithComponent<Position>();
            for (auto entity : entities) {
                if (m_registry.HasComponent<Velocity>(entity)) {
                    auto& pos = m_registry.GetComponent<Position>(entity);
                    if (entity == m_localPlayerEntity) {
                        pos.x = std::max(0.0f, std::min(pos.x, 1280.0f - 66.0f));
                        pos.y = std::max(0.0f, std::min(pos.y, 720.0f - 32.0f));
                    }
                    UpdatePlayerNameLabelPosition(entity, pos.x, pos.y);
                }
            }

            if (m_collisionDetectionSystem) {
                m_collisionDetectionSystem->Update(m_registry, dt);
            }
            if (m_bulletResponseSystem) {
                m_bulletResponseSystem->Update(m_registry, dt);
            }
            if (m_playerResponseSystem) {
                m_playerResponseSystem->Update(m_registry, dt);
            }
            if (m_obstacleResponseSystem) {
                m_obstacleResponseSystem->Update(m_registry, dt);
            }
            if (m_healthSystem) {
                m_healthSystem->Update(m_registry, dt);
            }

            if (m_scrollingSystem) {
                m_scrollingSystem->Update(m_registry, dt);
            }
            m_localScrollOffset += -150.0f * dt;

            // Power-up visual systems (client-side rendering)
            if (m_shieldSystem) {
                m_shieldSystem->Update(m_registry, dt);
            }
            if (m_forcePodSystem) {
                m_forcePodSystem->Update(m_registry, dt);
            }
            
            // Shooting system - enabled for local player to create bullets with power-up effects
            // Server also creates bullets, but client needs this for local player's weapon effects
            if (m_shootingSystem) {
                if (m_context.networkClient) {
                    // Only update shooting for local player in networked games
                    // Server handles other players' bullets
                    m_shootingSystem->Update(m_registry, dt);
                } else {
                    m_shootingSystem->Update(m_registry, dt);
                }
            }

            // Background infinite loop
            for (auto& bg : m_backgroundEntities) {
                if (!m_registry.HasComponent<Position>(bg))
                    continue;
                auto& pos = m_registry.GetComponent<Position>(bg);
                if (pos.x <= -1280.0f) {
                    pos.x = pos.x + 3 * 1280.0f;
                }
            }

            updateHUD();
        }

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
                    // Mirror server: 5-second shield, primarily for visual effects client-side
                    constexpr float SHIELD_DURATION_SECONDS = 5.0f;
                    m_registry.AddComponent<Shield>(playerEntity, Shield(SHIELD_DURATION_SECONDS));
                }
            } else {
                if (m_registry.HasComponent<Shield>(playerEntity)) {
                    m_registry.RemoveComponent<Shield>(playerEntity);
                }
            }

            // Note: Force Pod is handled as a separate entity on the server, so we don't create it here
            // The server will sync the force pod entity separately if it exists
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

            Entity nameLabelEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(nameLabelEntity, Position{x, y});
            TextLabel nameLabel(playerName, nameFont, 12);
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
