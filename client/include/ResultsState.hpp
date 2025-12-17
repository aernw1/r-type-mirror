/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ResultsState
*/

#pragma once

#include "GameStateMachine.hpp"
#include "ECS/Registry.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/TextRenderingSystem.hpp"
#include "Renderer/IRenderer.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace RType {
    namespace Client {

        class ResultsState : public IState {
        public:
            ResultsState(GameStateMachine& machine,
                         GameContext& context,
                         std::vector<std::pair<std::string, uint32_t>> scores);
            ~ResultsState() override = default;

            void Init() override;
            void Cleanup() override;
            void HandleInput() override;
            void Update(float dt) override;
            void Draw() override;

        private:
            void createUI();

        private:
            GameStateMachine& m_machine;
            GameContext& m_context;
            std::vector<std::pair<std::string, uint32_t>> m_scores;

            RType::ECS::Registry m_registry;
            std::shared_ptr<Renderer::IRenderer> m_renderer;
            std::unique_ptr<RType::ECS::RenderingSystem> m_renderingSystem;
            std::unique_ptr<RType::ECS::TextRenderingSystem> m_textSystem;

            Renderer::FontId m_fontLarge = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontMedium = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontSmall = Renderer::INVALID_FONT_ID;

            Renderer::TextureId m_bgTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId m_bgSprite = Renderer::INVALID_SPRITE_ID;
            Renderer::Vector2 m_bgTextureSize{0.0f, 0.0f};

            RType::ECS::Entity m_bgEntity = RType::ECS::NULL_ENTITY;

            bool m_drawScorePanel = false;
            Renderer::Rectangle m_scorePanelRect{};
            float m_scorePanelHeaderHeight = 44.0f;
            float m_scorePanelRowStartY = 0.0f;
            float m_scorePanelRowStepY = 36.0f;
            size_t m_scorePanelRowCount = 0;
            float m_colRankX = 0.0f;
            float m_colNameX = 0.0f;
            float m_colScoreX = 0.0f;

            bool m_enterPressed = false;
            bool m_escapePressed = false;
        };

    }
}


