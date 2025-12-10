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

            loadLevel(m_currentLevelPath);
            createSystems();
            initializeFromLevel();
            initializePlayers();
            initializeUI();

            Core::Logger::Info("[GameState] Initialization complete");
        }

        void InGameState::loadLevel(const std::string& levelPath) {
            m_levelData = ECS::LevelLoader::LoadFromFile(levelPath);
            m_levelAssets = ECS::LevelLoader::LoadAssets(m_levelData, m_renderer.get());
            Core::Logger::Info("[GameState] Loaded level '{}' with {} textures",
                               m_levelData.name, m_levelAssets.textures.size());
        }

        void InGameState::initializeFromLevel() {
            m_levelEntities = ECS::LevelLoader::CreateEntities(
                m_registry,
                m_levelData,
                m_levelAssets,
                m_renderer.get());

            m_backgroundEntities = m_levelEntities.backgrounds;
            m_obstacleEntities = m_levelEntities.obstacles;
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
            m_shootingSystem = std::make_unique<RType::ECS::ShootingSystem>(bulletSprite);
            m_movementSystem = std::make_unique<RType::ECS::MovementSystem>();
            m_inputSystem = std::make_unique<RType::ECS::InputSystem>(m_renderer.get());
            m_collisionSystem = std::make_unique<RType::ECS::CollisionSystem>();
            m_healthSystem = std::make_unique<RType::ECS::HealthSystem>();
        }

        void InGameState::initializePlayers() {
            std::cout << "[GameState] Initializing local player (ECS)..." << std::endl;

            m_localPlayerEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_localPlayerEntity, Position{100.0f, 360.0f});
            m_registry.AddComponent<Velocity>(m_localPlayerEntity, Velocity{0.0f, 0.0f});
            m_registry.AddComponent<Shooter>(m_localPlayerEntity, Shooter{0.2f, 50.0f, 25.0f});
            m_registry.AddComponent<ShootCommand>(m_localPlayerEntity, ShootCommand{});
            m_registry.AddComponent<Health>(m_localPlayerEntity, Health{100});

            auto spriteIt = m_levelAssets.sprites.find("player_blue");
            auto textureIt = m_levelAssets.textures.find("player_blue");

            if (spriteIt != m_levelAssets.sprites.end() && textureIt != m_levelAssets.textures.end()) {
                auto& d = m_registry.AddComponent<Drawable>(m_localPlayerEntity, Drawable(spriteIt->second, 10));
                d.scale = {0.5f, 0.5f};

                auto size = m_renderer->GetTextureSize(textureIt->second);
                if (size.x > 0) {
                    m_registry.AddComponent<BoxCollider>(m_localPlayerEntity, BoxCollider{size.x * 0.5f, size.y * 0.5f});
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
                    // Update health from player entity if available
                    if (m_playersHUD[i].playerEntity != NULL_ENTITY &&
                        m_registry.IsEntityAlive(m_playersHUD[i].playerEntity) &&
                        m_registry.HasComponent<Health>(m_playersHUD[i].playerEntity)) {
                        const auto& health = m_registry.GetComponent<Health>(m_playersHUD[i].playerEntity);
                        m_playersHUD[i].health = health.current;
                        m_playersHUD[i].maxHealth = health.max;
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

            if (m_localPlayerEntity != ECS::NULL_ENTITY && m_registry.HasComponent<ShootCommand>(m_localPlayerEntity)) {
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
                m_context.networkClient->SendInput(m_currentInputs);
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

            auto colliders = m_registry.GetEntitiesWithComponent<BoxCollider>();
            for (auto entity : colliders) {
                if (m_registry.HasComponent<Position>(entity)) {
                    auto& pos = m_registry.GetComponent<Position>(entity);
                    auto& box = m_registry.GetComponent<BoxCollider>(entity);

                    Renderer::Rectangle rect{{pos.x, pos.y}, {box.width, box.height}};
                    Renderer::Color color{1.0f, 0.0f, 0.0f, 0.3f};

                    m_renderer->DrawRectangle(rect, color);
                }
            }
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
            if (m_playersHUD[playerIndex].maxHealth > 0) {
                healthPercent = static_cast<float>(m_playersHUD[playerIndex].health) / static_cast<float>(m_playersHUD[playerIndex].maxHealth);
                healthPercent = std::max(0.0f, std::min(1.0f, healthPercent));
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

            for (auto& obstacle : m_obstacleEntities) {
                if (m_registry.IsEntityAlive(obstacle)) {
                    m_registry.DestroyEntity(obstacle);
                }
            }
            m_obstacleEntities.clear();

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

            for (const auto& entityState : entities) {
                receivedIds.insert(entityState.entityId);

                auto it = m_networkEntityMap.find(entityState.entityId);
                network::EntityType type = static_cast<network::EntityType>(entityState.entityType);

                if (it == m_networkEntityMap.end()) {
                    if (type == network::EntityType::PLAYER) {
                        if (entityState.ownerHash == m_context.playerHash && m_localPlayerEntity != ECS::NULL_ENTITY) {
                            m_networkEntityMap[entityState.entityId] = m_localPlayerEntity;
                            std::cout << "[GameState] Linked Local Player to NetID " << entityState.entityId << std::endl;
                            continue;
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

                        auto& drawable = m_registry.AddComponent<Drawable>(newEntity, Drawable(playerSprite, 10));
                        drawable.scale = {0.5f, 0.5f};

                        m_networkEntityMap[entityState.entityId] = newEntity;

                        if (entityState.ownerHash == m_context.playerHash) {
                            m_localPlayerEntity = newEntity;
                            std::cout << "[GameState] ✓ Local player ready - client-side prediction enabled" << std::endl;
                        }

                        if (playerIndex < MAX_PLAYERS) {
                            m_playersHUD[playerIndex].active = true;
                            m_playersHUD[playerIndex].playerEntity = newEntity;
                            std::cout << "[GameState] Player P" << (playerIndex + 1) << " added to scoreboard" << std::endl;
                        }

                        std::cout << "[GameState] Created PLAYER entity " << entityState.entityId << " with color index " << playerIndex << std::endl;
                    } else if (type == network::EntityType::BULLET) {
                        auto newEntity = m_registry.CreateEntity();
                        m_registry.AddComponent<Position>(newEntity, Position{entityState.x, entityState.y});
                        m_registry.AddComponent<Velocity>(newEntity, Velocity{entityState.vx, entityState.vy});

                        auto bulletSpriteIt = m_levelAssets.sprites.find("bullet");
                        if (bulletSpriteIt != m_levelAssets.sprites.end()) {
                            auto& d = m_registry.AddComponent<Drawable>(newEntity, Drawable(bulletSpriteIt->second, 12));
                            d.scale = {0.1f, 0.1f};
                        }
                        m_networkEntityMap[entityState.entityId] = newEntity;
                    }
                } else {
                    auto ecsEntity = it->second;

                    if (m_registry.HasComponent<Position>(ecsEntity)) {
                        auto& pos = m_registry.GetComponent<Position>(ecsEntity);
                        pos.x = entityState.x;
                        pos.y = entityState.y;
                    }

                    if (m_registry.HasComponent<Velocity>(ecsEntity)) {
                        auto& vel = m_registry.GetComponent<Velocity>(ecsEntity);
                        vel.dx = entityState.vx;
                        vel.dy = entityState.vy;
                    }

                    if (m_registry.HasComponent<Health>(ecsEntity)) {
                        auto& health = m_registry.GetComponent<Health>(ecsEntity);
                        health.current = static_cast<int>(entityState.health);
                    }
                }
            }

            for (auto it = m_networkEntityMap.begin(); it != m_networkEntityMap.end();) {
                if (receivedIds.find(it->first) == receivedIds.end()) {
                    if (m_registry.IsEntityAlive(it->second)) {
                        m_registry.DestroyEntity(it->second);
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
                }
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
    }
}
