/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyState
*/

#pragma once

#include "GameStateMachine.hpp"
#include "LobbyClient.hpp"
#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/TextRenderingSystem.hpp"
#include "ECS/AudioSystem.hpp"
#include "Renderer/IRenderer.hpp"
#include "GameState.hpp"
#include "Audio/IAudio.hpp"

#include <cstdint>
#include <memory>
#include <cstring>
#include <string>
#include <unordered_map>

namespace RType {
    namespace Client {

        class LobbyState : public IState {
        public:
            LobbyState(GameStateMachine& machine, GameContext& context);
            LobbyState(GameStateMachine& machine, GameContext& context, network::NetworkTcpSocket&& socket);
            ~LobbyState() override = default;

            void Init() override;
            void Cleanup() override;
            void HandleInput() override;
            void Update(float dt) override;
            void Draw() override;
        private:
            void createUI();
            void updateLobbyState();
            void updateOrCreatePlayerEntity(const network::PlayerInfo& player);
            void updatePlayerCardVisuals(uint8_t playerNum, const network::PlayerInfo& player);
            void removePlayer(uint8_t playerNum);
            Renderer::TextureId getPlayerTexture(uint8_t playerNum);

            GameStateMachine& m_machine;
            GameContext& m_context;

            std::unique_ptr<network::LobbyClient> m_client;
            std::string m_playerName;

            RType::ECS::Registry m_registry;
            std::shared_ptr<Renderer::IRenderer> m_renderer;
            std::unique_ptr<RType::ECS::RenderingSystem> m_renderingSystem;
            std::unique_ptr<RType::ECS::TextRenderingSystem> m_textSystem;
            std::unique_ptr<RType::ECS::AudioSystem> m_audioSystem;

            Audio::MusicId m_lobbyMusic = Audio::INVALID_MUSIC_ID;
            bool m_lobbyMusicPlaying = false;

            Renderer::FontId m_fontLarge = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontMedium = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontSmall = Renderer::INVALID_FONT_ID;

            Renderer::TextureId m_playerBlueTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_playerGreenTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_nave2BlueTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_nave2Texture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_bgTexture = Renderer::INVALID_TEXTURE_ID;

            RType::ECS::Entity m_titleEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_instructionsEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_bgEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_errorEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_countdownEntity = RType::ECS::NULL_ENTITY;

            std::unordered_map<uint8_t, RType::ECS::Entity> m_playerEntities;
            std::unordered_map<uint8_t, RType::ECS::Entity> m_playerSprites;

            std::string m_lastStatus;
            std::string m_errorMessage;
            float m_errorTimer = 0.0f;
            uint8_t m_countdownSeconds = 0;
            bool m_rKeyPressed = false;
            bool m_escapeKeyPressed = false;
            bool m_hasError = false;
        };

    }
}
