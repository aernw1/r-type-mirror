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
            initializePlayers();

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
            constexpr float spawnX = 100.0f;
            constexpr float spawnY = 360.0f;

            m_localPlayerEntity = ECS::PlayerFactory::CreatePlayer(m_registry,
                                                                   playerNumber,
                                                                   playerHash,
                                                                   spawnX,
                                                                   spawnY,
                                                                   m_renderer.get());

            if (m_localPlayerEntity == ECS::NULL_ENTITY) {
                Core::Logger::Error("[GameState] Failed to create local player entity");
                return;
            }

            if (m_registry.HasComponent<ECS::Player>(m_localPlayerEntity)) {
                auto& playerComp = m_registry.GetComponent<ECS::Player>(m_localPlayerEntity);
                playerComp.isLocalPlayer = true;
            }

            if (m_registry.HasComponent<ECS::Controllable>(m_localPlayerEntity)) {
                auto& controllable = m_registry.GetComponent<ECS::Controllable>(m_localPlayerEntity);
                controllable.speed = 300.0f;
            }

            m_registry.AddComponent<Shooter>(m_localPlayerEntity, Shooter{0.2f, 50.0f, 25.0f});
            m_registry.AddComponent<ShootCommand>(m_localPlayerEntity, ShootCommand{});

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

            std::string localPlayerName = m_context.playerName;
            NetworkPlayer localNetPlayer(m_context.playerNumber, m_context.playerHash, localPlayerName.c_str(), false);
            m_registry.AddComponent<NetworkPlayer>(m_localPlayerEntity, std::move(localNetPlayer));

            if (m_context.playerNumber > 0 && m_context.playerNumber <= MAX_PLAYERS) {
                m_assignedPlayerNumbers.insert(m_context.playerNumber);
            }

            CreatePlayerNameLabel(m_localPlayerEntity, localPlayerName, 100.0f, 360.0f);
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
                        if (entityState.ownerHash == m_context.playerHash) {
                            auto existing = m_networkEntityMap.find(entityState.entityId);
                            if (existing != m_networkEntityMap.end()) {
                                continue;
                        if (entityState.ownerHash == m_context.playerHash && m_localPlayerEntity != ECS::NULL_ENTITY) {
                            m_networkEntityMap[entityState.entityId] = m_localPlayerEntity;

                            if (m_registry.HasComponent<Position>(m_localPlayerEntity)) {
                                auto& pos = m_registry.GetComponent<Position>(m_localPlayerEntity);
                                pos.x = entityState.x;
                                pos.y = entityState.y;

                                UpdatePlayerNameLabelPosition(m_localPlayerEntity, entityState.x, entityState.y);
                            }

                            if (m_context.playerNumber >= 1 && m_context.playerNumber <= MAX_PLAYERS) {
                                size_t playerIndex = static_cast<size_t>(m_context.playerNumber - 1);
                                m_playersHUD[playerIndex].playerEntity = m_localPlayerEntity;
                                m_playersHUD[playerIndex].active = true;

                                if (entityState.health > 0) {
                                    m_playersHUD[playerIndex].isDead = false;
                                }
                            }
                        }

                        Renderer::SpriteId playerSprite = Renderer::INVALID_SPRITE_ID;
                        size_t spriteIndex = m_networkEntityMap.size() % 3;

                        const char* spriteKeys[] = {"player_green", "player_blue", "player_red"};
                        auto spriteIt = m_levelAssets.sprites.find(spriteKeys[spriteIndex]);
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

                        auto& drawable = m_registry.AddComponent<Drawable>(newEntity, Drawable(playerSprite, 10));
                        drawable.scale = {0.5f, 0.5f};
                        drawable.origin = Math::Vector2(128.0f, 128.0f);

                        auto [playerName, playerNum] = FindPlayerNameAndNumber(entityState.ownerHash, m_assignedPlayerNumbers);
                        if (playerNum > 0 && playerNum <= MAX_PLAYERS) {
                            m_assignedPlayerNumbers.insert(playerNum);
                        }

                        NetworkPlayer netPlayer(playerNum, entityState.ownerHash, playerName.c_str(), false);
                        m_registry.AddComponent<NetworkPlayer>(newEntity, std::move(netPlayer));

                        CreatePlayerNameLabel(newEntity, playerName, entityState.x, entityState.y);

                        m_networkEntityMap[entityState.entityId] = newEntity;

                        if (entityState.ownerHash == m_context.playerHash) {
                            m_localPlayerEntity = newEntity;
                            if (m_context.playerNumber >= 1 && m_context.playerNumber <= MAX_PLAYERS) {
                                size_t localPlayerIndex = static_cast<size_t>(m_context.playerNumber - 1);
                                m_playersHUD[localPlayerIndex].playerEntity = newEntity;
                                m_playersHUD[localPlayerIndex].active = true;
                                if (entityState.health > 0) {
                                    m_playersHUD[localPlayerIndex].isDead = false;
                                }
                            }
                        }

                        if (playerIndex < MAX_PLAYERS) {
                            m_playersHUD[playerIndex].active = true;
                            m_playersHUD[playerIndex].playerEntity = newEntity;
                            std::cout << "[GameState] ✓ Local player ready - client-side prediction enabled" << std::endl;
                        } else if (playerNum > 0 && playerNum <= MAX_PLAYERS) {
                            size_t hudPlayerIndex = static_cast<size_t>(playerNum - 1);
                            m_playersHUD[hudPlayerIndex].active = true;
                            m_playersHUD[hudPlayerIndex].playerEntity = newEntity;
                            if (entityState.health > 0) {
                                m_playersHUD[hudPlayerIndex].isDead = false;
                            }
                            std::cout << "[GameState] Player P" << (int)playerNum << " added to scoreboard" << std::endl;
                        }

                        std::cout << "[GameState] Created PLAYER entity " << entityState.entityId << " with sprite index " << spriteIndex << std::endl;
                    } else if (type == network::EntityType::ENEMY) {
                        uint8_t enemyType = entityState.flags;
                        EnemySpriteConfig config = GetEnemySpriteConfig(enemyType);
                        Renderer::SpriteId enemySprite = config.sprite;
                        Math::Color enemyTint = config.tint;

                        if (enemySprite == Renderer::INVALID_SPRITE_ID) {
                            enemySprite = m_enemyGreenSprite;
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

                        UpdatePlayerNameLabelPosition(ecsEntity, entityState.x, entityState.y);
                    }

                    if (m_registry.HasComponent<Velocity>(ecsEntity)) {
                        auto& vel = m_registry.GetComponent<Velocity>(ecsEntity);
                        vel.dx = entityState.vx;
                        vel.dy = entityState.vy;
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
                        if (playerIndex == MAX_PLAYERS && m_registry.HasComponent<NetworkPlayer>(ecsEntity)) {
                            const auto& netPlayer = m_registry.GetComponent<NetworkPlayer>(ecsEntity);
                            if (netPlayer.playerNumber > 0 && netPlayer.playerNumber <= MAX_PLAYERS) {
                                playerIndex = static_cast<size_t>(netPlayer.playerNumber - 1);
                                isPlayerEntity = true;
                                if (playerIndex < MAX_PLAYERS) {
                                    playerIsDead = m_playersHUD[playerIndex].isDead;
                                    if (m_playersHUD[playerIndex].playerEntity == NULL_ENTITY) {
                                        m_playersHUD[playerIndex].playerEntity = ecsEntity;
                                        m_playersHUD[playerIndex].active = true;
                                    }
                                }
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

                                    if (m_registry.HasComponent<NetworkPlayer>(ecsEntity)) {
                                        const auto& netPlayer = m_registry.GetComponent<NetworkPlayer>(ecsEntity);
                                        if (netPlayer.playerNumber > 0 && netPlayer.playerNumber <= MAX_PLAYERS) {
                                            m_assignedPlayerNumbers.erase(netPlayer.playerNumber);
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

                    if (m_registry.HasComponent<NetworkPlayer>(ecsEntity)) {
                        const auto& netPlayer = m_registry.GetComponent<NetworkPlayer>(ecsEntity);
                        if (netPlayer.playerNumber > 0 && netPlayer.playerNumber <= MAX_PLAYERS) {
                            m_assignedPlayerNumbers.erase(netPlayer.playerNumber);
                        }
                    }

                    for (size_t i = 0; i < MAX_PLAYERS; i++) {
                        if (m_playersHUD[i].playerEntity == ecsEntity) {
                            m_playersHUD[i].isDead = true;
                            m_playersHUD[i].health = 0;
                            m_playersHUD[i].playerEntity = NULL_ENTITY;
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


            if (!m_isNetworkSession &&
                m_localPlayerEntity != ECS::NULL_ENTITY &&
                m_registry.HasComponent<Position>(m_localPlayerEntity)) {
                auto& pos = m_registry.GetComponent<Position>(m_localPlayerEntity);
                pos.x = std::max(0.0f, std::min(pos.x, 1280.0f - 66.0f));
                pos.y = std::max(0.0f, std::min(pos.y, 720.0f - 32.0f));
            auto entities = m_registry.GetEntitiesWithComponent<Position>();
            for (auto entity : entities) {
                if (m_registry.HasComponent<Velocity>(entity)) {
                    auto& pos = m_registry.GetComponent<Position>(entity);
                    auto& vel = m_registry.GetComponent<Velocity>(entity);

                    pos.x += vel.dx * dt;
                    pos.y += vel.dy * dt;

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

            m_scrollingSystem->Update(m_registry, dt);
            m_localScrollOffset += -150.0f * dt;

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
            nameLabel.offsetY = -15.0f;
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
