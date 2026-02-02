/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RoomListState - Room selection screen
*/

#pragma once

#include "GameStateMachine.hpp"
#include "RoomClient.hpp"
#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/TextRenderingSystem.hpp"
#include "Renderer/IRenderer.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace RType {
    namespace Client {

        class RoomListState : public IState {
        public:
            RoomListState(GameStateMachine& machine, GameContext& context);
            ~RoomListState() override = default;

            void Init() override;
            void Cleanup() override;
            void HandleInput() override;
            void Update(float dt) override;
            void Draw() override;

        private:
            void createUI();
            void updateRoomList();
            void createRoomEntities();
            void clearRoomEntities();
            void handleCreateRoom();
            void handleJoinRoom();

            GameStateMachine& m_machine;
            GameContext& m_context;

            std::unique_ptr<network::RoomClient> m_roomClient;

            RType::ECS::Registry m_registry;
            std::shared_ptr<Renderer::IRenderer> m_renderer;
            std::unique_ptr<RType::ECS::RenderingSystem> m_renderingSystem;
            std::unique_ptr<RType::ECS::TextRenderingSystem> m_textSystem;

            Renderer::FontId m_fontLarge = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontMedium = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontSmall = Renderer::INVALID_FONT_ID;
            Renderer::TextureId m_bgTexture = Renderer::INVALID_TEXTURE_ID;

            RType::ECS::Entity m_titleEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_instructionsEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_bgEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_noRoomsEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_errorEntity = RType::ECS::NULL_ENTITY;

            std::vector<RType::ECS::Entity> m_roomEntities;
            std::vector<network::RoomInfo> m_rooms;

            int m_selectedIndex = 0;
            bool m_upKeyPressed = false;
            bool m_downKeyPressed = false;
            bool m_enterKeyPressed = false;
            bool m_cKeyPressed = false;
            bool m_escapeKeyPressed = false;

            bool m_hasError = false;
            std::string m_errorMessage;
            float m_errorTimer = 0.0f;

            float m_refreshTimer = 0.0f;
            static constexpr float REFRESH_INTERVAL = 2.0f;

            bool m_creatingRoom = false;
            std::string m_newRoomName;
        };

    }
}
