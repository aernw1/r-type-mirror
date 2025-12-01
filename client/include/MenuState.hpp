/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MenuState
*/

#pragma once

#include "GameStateMachine.hpp"
#include "ECS/Registry.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/TextRenderingSystem.hpp"
#include "Renderer/IRenderer.hpp"
#include <memory>
#include <vector>

namespace RType {
    namespace Client {

        class MenuState : public IState {
        public:
            MenuState(GameStateMachine& machine, GameContext& context);
            ~MenuState() override = default;

            void Init() override;
            void Cleanup() override;
            void HandleInput() override;
            void Update(float dt) override;
            void Draw() override;

        private:
            void createUI();
            void updateAnimations(float dt);

            GameStateMachine& m_machine;
            GameContext& m_context;

            RType::ECS::Registry m_registry;
            std::shared_ptr<Renderer::IRenderer> m_renderer;
            std::unique_ptr<RType::ECS::RenderingSystem> m_renderingSystem;
            std::unique_ptr<RType::ECS::TextRenderingSystem> m_textSystem;

            Renderer::FontId m_fontLarge = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontMedium = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontSmall = Renderer::INVALID_FONT_ID;
            Renderer::TextureId m_bgTexture = Renderer::INVALID_TEXTURE_ID;

            std::vector<RType::ECS::Entity> m_entities;
            RType::ECS::Entity m_titleEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_playTextEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_subtitleEntity = RType::ECS::NULL_ENTITY;

            bool m_playKeyPressed = false;
            bool m_escapeKeyPressed = false;

            float m_animTime = 0.0f;
            float m_titlePulse = 0.0f;
        };

    }
}
