#pragma once

#include "LobbyClient.hpp"
#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/TextRenderingSystem.hpp"
#include "Renderer/SFMLRenderer.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace Client {

    class VisualLobbyClient {
    public:
        VisualLobbyClient(const std::string& serverAddr,
            uint16_t port,
            const std::string& playerName);

        void run();

    private:
        void createTitleEntity();
        void createInstructionsEntity();
        void updateLobbyState();
        void updateStatusText(const std::string& status);
        void updateOrCreatePlayerEntity(const network::PlayerInfo& player);
        void removePlayer(uint8_t playerNum);
        void handleInput();

        network::LobbyClient m_client;
        std::string m_playerName;

        RType::ECS::Registry m_registry;
        std::unique_ptr<Renderer::SFMLRenderer> m_renderer;
        std::unique_ptr<RType::ECS::RenderingSystem> m_renderingSystem;
        std::unique_ptr<RType::ECS::TextRenderingSystem> m_textSystem;

        Renderer::FontId m_font = Renderer::INVALID_FONT_ID;
        RType::ECS::Entity m_statusEntity = RType::ECS::NULL_ENTITY;
        std::unordered_map<uint8_t, RType::ECS::Entity> m_playerEntities;

        std::string m_lastStatus;
        bool m_rKeyPressed = false;
        bool m_sKeyPressed = false;
    };

}

