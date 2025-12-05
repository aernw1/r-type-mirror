/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyState
*/

#include "LobbyState.hpp"
#include <chrono>
#include <thread>
#include "ECS/Components/TextLabel.hpp"
#include <iostream>
#include <stdexcept>

using namespace RType::ECS;

namespace RType {
    namespace Client {

        LobbyState::LobbyState(GameStateMachine& machine, GameContext& context) : m_machine(machine), m_context(context), m_client(context.serverIp, context.serverPort), m_playerName(context.playerName) {

            m_renderer = context.renderer;
            m_renderingSystem = std::make_unique<RenderingSystem>(m_renderer.get());
            m_textSystem = std::make_unique<TextRenderingSystem>(m_renderer.get());
        }

        void LobbyState::Init() {
            std::cout << "[LobbyState] Connecting to " << m_context.serverIp << ":" << m_context.serverPort << " as " << m_playerName << std::endl;

            // Load fonts
            m_fontLarge = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 32);
            if (m_fontLarge == Renderer::INVALID_FONT_ID) {
                m_fontLarge = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 32);
            }

            m_fontMedium = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 20);
            if (m_fontMedium == Renderer::INVALID_FONT_ID) {
                m_fontMedium = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 20);
            }

            m_fontSmall = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 14);
            if (m_fontSmall == Renderer::INVALID_FONT_ID) {
                m_fontSmall = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 14);
            }

            m_playerBlueTexture = m_renderer->LoadTexture("assets/spaceships/player_blue.png");
            if (m_playerBlueTexture == Renderer::INVALID_TEXTURE_ID) {
                m_playerBlueTexture = m_renderer->LoadTexture("../assets/spaceships/player_blue.png");
            }

            m_playerGreenTexture = m_renderer->LoadTexture("assets/spaceships/player_green.png");
            if (m_playerGreenTexture == Renderer::INVALID_TEXTURE_ID) {
                m_playerGreenTexture = m_renderer->LoadTexture("../assets/spaceships/player_green.png");
            }

            m_nave2BlueTexture = m_renderer->LoadTexture("assets/spaceships/nave2_blue.png");
            if (m_nave2BlueTexture == Renderer::INVALID_TEXTURE_ID) {
                m_nave2BlueTexture = m_renderer->LoadTexture("../assets/spaceships/nave2_blue.png");
            }

            m_nave2Texture = m_renderer->LoadTexture("assets/spaceships/nave2.png");
            if (m_nave2Texture == Renderer::INVALID_TEXTURE_ID) {
                m_nave2Texture = m_renderer->LoadTexture("../assets/spaceships/nave2.png");
            }

            m_bgTexture = m_renderer->LoadTexture("assets/backgrounds/1.jpg");
            if (m_bgTexture == Renderer::INVALID_TEXTURE_ID) {
                m_bgTexture = m_renderer->LoadTexture("../assets/backgrounds/1.jpg");
            }

            m_client.onPlayerLeft([this](uint8_t playerNum) {
                removePlayer(playerNum);
            });

            m_client.onConnectionError([this](const std::string& error) {
                m_errorMessage = error;
                m_hasError = true;
            });

            m_client.onCountdown([this](uint8_t seconds) {
                m_countdownSeconds = seconds;
            });

            createUI();
            m_client.connect(m_playerName);
        }

        void LobbyState::Cleanup() {
            std::cout << "[LobbyState] Cleaning up..." << std::endl;

            m_client.disconnect();

            for (auto& [playerNum, entity] : m_playerEntities) {
                if (m_registry.IsEntityAlive(entity)) {
                    m_registry.DestroyEntity(entity);
                }
            }
            m_playerEntities.clear();

            for (auto& [playerNum, sprite] : m_playerSprites) {
                if (m_registry.IsEntityAlive(sprite)) {
                    m_registry.DestroyEntity(sprite);
                }
            }
            m_playerSprites.clear();
        }

        void LobbyState::createUI() {
            if (m_bgTexture != Renderer::INVALID_TEXTURE_ID) {
                m_bgEntity = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(m_bgEntity, Position{0.0f, 0.0f});

                Renderer::SpriteId spriteId = m_renderer->CreateSprite(m_bgTexture, {});
                Drawable drawable(spriteId, -10);

                Renderer::Vector2 texSize = m_renderer->GetTextureSize(m_bgTexture);
                if (texSize.x > 0 && texSize.y > 0) {
                    drawable.scale = {1280.0f / texSize.x, 720.0f / texSize.y};
                }

                m_registry.AddComponent<Drawable>(m_bgEntity, std::move(drawable));
            }

            if (m_fontLarge == Renderer::INVALID_FONT_ID)
                return;
            m_titleEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_titleEntity, Position{640.0f, 50.0f});
            TextLabel titleLabel("MULTIPLAYER LOBBY", m_fontLarge, 36);
            titleLabel.color = {0.0f, 1.0f, 1.0f, 1.0f};
            titleLabel.centered = true;
            m_registry.AddComponent<TextLabel>(m_titleEntity, std::move(titleLabel));

            m_instructionsEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_instructionsEntity, Position{640.0f, 650.0f});
            TextLabel instrLabel("[R] Toggle Ready  |  [ESC] Back to Menu", m_fontSmall, 14);
            instrLabel.color = {0.5f, 0.86f, 1.0f, 0.9f};
            instrLabel.centered = true;
            m_registry.AddComponent<TextLabel>(m_instructionsEntity, std::move(instrLabel));
        }

        void LobbyState::updateLobbyState() {
            if (!m_client.isConnected()) {
                return;
            }

            const auto& players = m_client.getPlayers();
            for (const auto& player : players) {
                updateOrCreatePlayerEntity(player);
            }
        }

        Renderer::TextureId LobbyState::getPlayerTexture(uint8_t playerNum) {
            switch (playerNum) {
            case 0:
                return m_playerBlueTexture;
            case 1:
                return m_playerGreenTexture;
            case 2:
                return m_nave2BlueTexture;
            case 3:
                return m_nave2Texture;
            default:
                return m_playerBlueTexture;
            }
        }

        void LobbyState::updateOrCreatePlayerEntity(const network::PlayerInfo& player) {
            Entity entity;
            auto it = m_playerEntities.find(player.number);

            if (it == m_playerEntities.end()) {
                entity = m_registry.CreateEntity();
                m_playerEntities[player.number] = entity;

                float baseX = 240.0f;
                float baseY = 180.0f;
                float spacingX = 420.0f;
                float spacingY = 220.0f;

                uint8_t index = player.number - 1;
                float cardX = baseX + (index % 2) * spacingX;
                float cardY = baseY + (index / 2) * spacingY;

                Entity spriteEntity = m_registry.CreateEntity();
                m_playerSprites[player.number] = spriteEntity;

                Renderer::TextureId texture = getPlayerTexture(index);
                if (texture != Renderer::INVALID_TEXTURE_ID) {
                    Renderer::SpriteId spriteId = m_renderer->CreateSprite(texture, {});
                    Drawable drawable(spriteId, 0);
                    drawable.scale = {1.0f, 1.0f};
                    m_registry.AddComponent<Position>(spriteEntity, Position{cardX, cardY});
                    m_registry.AddComponent<Drawable>(spriteEntity, std::move(drawable));
                }

                m_registry.AddComponent<Position>(entity, Position{cardX + 10.0f, cardY + 50.0f});
                NetworkPlayer netPlayer{player.number, player.hash, player.name, player.ready};
                m_registry.AddComponent<NetworkPlayer>(entity, std::move(netPlayer));

                if (m_fontMedium != Renderer::INVALID_FONT_ID) {
                    TextLabel label("", m_fontMedium, 16);
                    m_registry.AddComponent<TextLabel>(entity, std::move(label));
                }

                updatePlayerCardVisuals(player.number, player);
            } else {
                entity = it->second;
                auto& netPlayer = m_registry.GetComponent<NetworkPlayer>(entity);
                if (netPlayer.ready != player.ready) {
                    netPlayer.ready = player.ready;
                    updatePlayerCardVisuals(player.number, player);
                }
            }
        }

        void LobbyState::updatePlayerCardVisuals(uint8_t playerNum, const network::PlayerInfo& player) {
            auto it = m_playerEntities.find(playerNum);
            if (it == m_playerEntities.end())
                return;

            Entity entity = it->second;
            const auto& netPlayer = m_registry.GetComponent<NetworkPlayer>(entity);

            std::string displayText = "P" + std::to_string(netPlayer.playerNumber) + " - " + std::string(player.name);
            if (netPlayer.ready) {
                displayText += "\n[READY]";
            } else {
                displayText += "\n[WAITING]";
            }

            if (m_fontMedium != Renderer::INVALID_FONT_ID) {
                auto& label = m_registry.GetComponent<TextLabel>(entity);
                label.text = displayText;

                if (netPlayer.ready) {
                    label.color = {0.4f, 1.0f, 0.5f, 1.0f};
                } else {
                    label.color = {1.0f, 0.85f, 0.3f, 1.0f};
                }
            }
        }

        void LobbyState::removePlayer(uint8_t playerNum) {
            auto it = m_playerEntities.find(playerNum);
            if (it != m_playerEntities.end()) {
                if (m_registry.IsEntityAlive(it->second)) {
                    m_registry.DestroyEntity(it->second);
                }
                m_playerEntities.erase(it);
            }

            auto spriteIt = m_playerSprites.find(playerNum);
            if (spriteIt != m_playerSprites.end()) {
                if (m_registry.IsEntityAlive(spriteIt->second)) {
                    m_registry.DestroyEntity(spriteIt->second);
                }
                m_playerSprites.erase(spriteIt);
            }
        }

        void LobbyState::HandleInput() {
            if (m_renderer->IsKeyPressed(Renderer::Key::R) && !m_rKeyPressed) {
                m_rKeyPressed = true;
                if (m_client.isConnected()) {
                    m_client.ready();
                    std::cout << "[LobbyState] Toggled ready!" << std::endl;
                }
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::R)) {
                m_rKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escapeKeyPressed) {
                m_escapeKeyPressed = true;
                std::cout << "[LobbyState] Returning to menu..." << std::endl;
                m_machine.PopState();
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                m_escapeKeyPressed = false;
            }
        }

        void LobbyState::Update(float dt) {
            if (m_hasError) {
                if (m_errorEntity == NULL_ENTITY && m_fontLarge != Renderer::INVALID_FONT_ID) {
                    m_errorEntity = m_registry.CreateEntity();
                    m_registry.AddComponent<Position>(m_errorEntity, Position{640.0f, 300.0f});
                    TextLabel errorLabel(m_errorMessage, m_fontMedium, 24);
                    errorLabel.color = {1.0f, 0.08f, 0.58f, 1.0f};
                    errorLabel.centered = true;
                    m_registry.AddComponent<TextLabel>(m_errorEntity, std::move(errorLabel));

                    Entity backMsg = m_registry.CreateEntity();
                    m_registry.AddComponent<Position>(backMsg, Position{640.0f, 380.0f});
                    TextLabel backLabel("Press ESC to return to menu", m_fontSmall, 14);
                    backLabel.color = {0.5f, 0.86f, 1.0f, 0.9f};
                    backLabel.centered = true;
                    m_registry.AddComponent<TextLabel>(backMsg, std::move(backLabel));
                }
                m_errorTimer += dt;
                if (m_errorTimer >= 3.0f) {
                    std::cout << "[LobbyState] Auto-returning to menu after error..." << std::endl;
                    m_machine.PopState();
                }
                return;
            }

            m_client.update();
            updateLobbyState();

            if (m_countdownSeconds > 0) {
                if (m_countdownEntity == NULL_ENTITY && m_fontLarge != Renderer::INVALID_FONT_ID) {
                    m_countdownEntity = m_registry.CreateEntity();
                    m_registry.AddComponent<Position>(m_countdownEntity, Position{640.0f, 500.0f});
                    TextLabel countdownLabel("", m_fontLarge, 48);
                    countdownLabel.color = {1.0f, 0.08f, 0.58f, 1.0f};
                    countdownLabel.centered = true;
                    m_registry.AddComponent<TextLabel>(m_countdownEntity, std::move(countdownLabel));
                }

                if (m_countdownEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_countdownEntity)) {
                    auto& label = m_registry.GetComponent<TextLabel>(m_countdownEntity);
                    label.text = "Starting in " + std::to_string((int)m_countdownSeconds) + "...";
                }
            } else {
                if (m_countdownEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_countdownEntity)) {
                    m_registry.DestroyEntity(m_countdownEntity);
                    m_countdownEntity = NULL_ENTITY;
                }
            }

            if (m_client.isGameStarted()) {
                m_countdownSeconds = 0;

                uint32_t seed = m_client.getGameSeed();
                std::string serverIp = m_context.serverIp;
                uint16_t udpPort = m_context.serverPort;

                std::cout << "[LobbyState] Game started! Seed: " << seed << std::endl;
                std::cout << "[LobbyState] Waiting for server to start UDP..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));

                std::cout << "[LobbyState] Transitioning to GameState..." << std::endl;

                network::PlayerInfo localPlayer = m_client.getMyInfo();

                auto gameClient = std::make_shared<network::GameClient>(serverIp, udpPort, localPlayer);
                if (gameClient->ConnectToServer()) {
                    m_context.networkClient = gameClient;
                    m_machine.ChangeState(std::make_unique<InGameState>(m_machine, m_context, seed));
                }
                // Transition to Game
                return;
            }
        }

        void LobbyState::Draw() {
            m_renderer->Clear({0.05f, 0.05f, 0.12f, 1.0f});

            m_renderingSystem->Update(m_registry, 0.0f);
            m_textSystem->Update(m_registry, 0.0f);
        }

    }
}
