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
#include "ECS/AudioSystem.hpp"
#include "Renderer/IRenderer.hpp"
#include "Audio/IAudio.hpp"
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
            void updateMenuSelection();

            enum class MenuItem {
                PLAY = 0,
                EDITOR = 1,
                SETTINGS = 2,
                QUIT = 3,
                COUNT = 4
            };

            GameStateMachine& m_machine;
            GameContext& m_context;

            RType::ECS::Registry m_registry;
            std::shared_ptr<Renderer::IRenderer> m_renderer;
            std::unique_ptr<RType::ECS::RenderingSystem> m_renderingSystem;
            std::unique_ptr<RType::ECS::TextRenderingSystem> m_textSystem;
            std::unique_ptr<RType::ECS::AudioSystem> m_audioSystem;

            Audio::MusicId m_menuMusic = Audio::INVALID_MUSIC_ID;
            bool m_menuMusicPlaying = false;

            Renderer::FontId m_fontLarge = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontMedium = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontSmall = Renderer::INVALID_FONT_ID;
            Renderer::TextureId m_bgTexture = Renderer::INVALID_TEXTURE_ID;

            std::vector<RType::ECS::Entity> m_entities;
            RType::ECS::Entity m_titleEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_subtitleEntity = RType::ECS::NULL_ENTITY;
            std::vector<RType::ECS::Entity> m_menuItems;

            int m_selectedIndex = 0;
            bool m_upKeyPressed = false;
            bool m_downKeyPressed = false;
            bool m_enterKeyPressed = false;

            float m_animTime = 0.0f;
            float m_titlePulse = 0.0f;
        };

    }
}
