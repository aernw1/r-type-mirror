/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyState - Initialization and cleanup functions
*/

#include "../../include/LobbyState.hpp"
#include "ECS/Components/TextLabel.hpp"
#include "ECS/Component.hpp"
#include "ECS/AudioSystem.hpp"
#include <iostream>

using namespace RType::ECS;

namespace RType {
    namespace Client {

        LobbyState::LobbyState(GameStateMachine& machine, GameContext& context)
            : m_machine(machine), m_context(context), m_playerName(context.playerName) {
            m_renderer = context.renderer;
            m_renderingSystem = std::make_unique<RenderingSystem>(m_renderer.get());
            m_textSystem = std::make_unique<TextRenderingSystem>(m_renderer.get());
        }

        LobbyState::LobbyState(GameStateMachine& machine, GameContext& context, network::NetworkTcpSocket&& socket)
            : m_machine(machine), m_context(context), m_playerName(context.playerName) {
            m_renderer = context.renderer;
            m_renderingSystem = std::make_unique<RenderingSystem>(m_renderer.get());
            m_textSystem = std::make_unique<TextRenderingSystem>(m_renderer.get());
            m_client = std::make_unique<network::LobbyClient>(std::move(socket));
        }

        void LobbyState::Init() {
            std::cout << "[LobbyState] Connecting to " << m_context.serverIp << ":" << m_context.serverPort << " as " << m_playerName << std::endl;

            if (m_context.audio) {
                m_audioSystem = std::make_unique<RType::ECS::AudioSystem>(m_context.audio.get());
                m_lobbyMusic = m_context.audio->LoadMusic("assets/sounds/lobby.flac");
                if (m_lobbyMusic == Audio::INVALID_MUSIC_ID) {
                    m_lobbyMusic = m_context.audio->LoadMusic("../assets/sounds/lobby.flac");
                }

                if (m_lobbyMusic != Audio::INVALID_MUSIC_ID) {
                    auto cmd = m_registry.CreateEntity();
                    auto& me = m_registry.AddComponent<MusicEffect>(cmd, MusicEffect(m_lobbyMusic));
                    me.play = true;
                    me.stop = false;
                    me.loop = true;
                    me.volume = 0.35f;
                    me.pitch = 1.0f;
                    m_lobbyMusicPlaying = true;
                }
            }

            if (!m_client) {
                if (!m_context.networkModule) {
                    std::cout << "[LobbyState] ERROR: networkModule is null (cannot create LobbyClient)" << std::endl;
                    m_errorMessage = "Internal error: network module missing";
                    m_hasError = true;
                    return;
                }
                m_client = std::make_unique<network::LobbyClient>(
                    m_context.networkModule.get(), m_context.serverIp, m_context.serverPort);
            }

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

            m_client->onPlayerLeft([this](uint8_t playerNum) {
                removePlayer(playerNum);
            });

            m_client->onConnectionError([this](const std::string& error) {
                m_errorMessage = error;
                m_hasError = true;
            });

            m_client->onCountdown([this](uint8_t seconds) {
                m_countdownSeconds = seconds;
            });

            createUI();
            m_client->connect(m_playerName);
        }

        void LobbyState::Cleanup() {
            std::cout << "[LobbyState] Cleaning up..." << std::endl;

            if (m_context.audio && m_lobbyMusic != Audio::INVALID_MUSIC_ID) {
                m_context.audio->StopMusic(m_lobbyMusic);
                m_context.audio->UnloadMusic(m_lobbyMusic);
                m_lobbyMusic = Audio::INVALID_MUSIC_ID;
                m_lobbyMusicPlaying = false;
            }

            if (m_client) {
                m_client->disconnect();
                m_client.reset();
            }

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
            TextLabel instrLabel("[R] Toggle Ready  |  [ESC] Change Room", m_fontSmall, 14);
            instrLabel.color = {0.5f, 0.86f, 1.0f, 0.9f};
            instrLabel.centered = true;
            m_registry.AddComponent<TextLabel>(m_instructionsEntity, std::move(instrLabel));
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

    }
}
