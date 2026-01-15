/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SettingsState
*/

#pragma once

#include "GameStateMachine.hpp"
#include "ECS/Registry.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/TextRenderingSystem.hpp"
#include "ECS/AudioSystem.hpp"
#include "Renderer/IRenderer.hpp"
#include "Audio/IAudio.hpp"
#include "Core/ColorFilter.hpp"
#include <memory>
#include <vector>
#include <string>

namespace RType {
    namespace Client {

        class SettingsState : public IState {
        public:
            SettingsState(GameStateMachine& machine, GameContext& context);
            ~SettingsState() override = default;

            void Init() override;
            void Cleanup() override;
            void HandleInput() override;
            void Update(float dt) override;
            void Draw() override;

            static bool IsColourBlindModeEnabled();
            static void SetColourBlindMode(bool enabled);
            static void LoadSettingsFromFile();

        private:
            void createUI();
            void updateAnimations(float dt);
            void updateMenuSelection();
            void toggleColourBlindMode();
            void saveSettings();
            void loadSettings();

            enum class SettingsItem {
                COLOUR_BLIND = 0,
                BACK = 1,
                COUNT = 2
            };

            GameStateMachine& m_machine;
            GameContext& m_context;

            RType::ECS::Registry m_registry;
            std::shared_ptr<Renderer::IRenderer> m_renderer;
            std::unique_ptr<RType::ECS::RenderingSystem> m_renderingSystem;
            std::unique_ptr<RType::ECS::TextRenderingSystem> m_textSystem;
            std::unique_ptr<RType::ECS::AudioSystem> m_audioSystem;

            Audio::MusicId m_selectMusic = Audio::INVALID_MUSIC_ID;

            Renderer::FontId m_fontLarge = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontMedium = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontSmall = Renderer::INVALID_FONT_ID;
            Renderer::TextureId m_bgTexture = Renderer::INVALID_TEXTURE_ID;

            std::vector<RType::ECS::Entity> m_entities;
            RType::ECS::Entity m_titleEntity = RType::ECS::NULL_ENTITY;
            std::vector<RType::ECS::Entity> m_menuItems;

            int m_selectedIndex = 0;
            bool m_upKeyPressed = false;
            bool m_downKeyPressed = false;
            bool m_enterKeyPressed = false;
            bool m_escKeyPressed = false;

            float m_animTime = 0.0f;

            bool m_colourBlindMode = false;

            static bool s_colourBlindMode;
            static const std::string SETTINGS_FILE_PATH;
        };

    }
}

