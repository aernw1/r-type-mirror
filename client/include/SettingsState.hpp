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
            static std::uint32_t GetScreenWidth();
            static std::uint32_t GetScreenHeight();
            static bool GetFullscreen();
            static std::uint32_t GetTargetFramerate();

        private:
            void createUI();
            void createScreenUI();
            void updateAnimations(float dt);
            void updateMenuSelection();
            void updateScreenMenuSelection();
            void toggleColourBlindMode();
            void enterScreenMenu();
            void exitScreenMenu();
            void changeResolution(int direction);
            void toggleFullscreen();
            void changeFrameRate(int direction);
            void applyScreenSettings();
            void saveSettings();
            void loadSettings();

            enum class SettingsItem {
                COLOUR_BLIND = 0,
                SCREEN = 1,
                BACK = 2,
                COUNT = 3
            };

            enum class ScreenItem {
                RESOLUTION = 0,
                FULLSCREEN = 1,
                FRAME_RATE = 2,
                BACK = 3,
                COUNT = 4
            };

            struct Resolution {
                std::uint32_t width;
                std::uint32_t height;
                std::string name;
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
            std::vector<RType::ECS::Entity> m_screenMenuItems;

            int m_selectedIndex = 0;
            int m_screenSelectedIndex = 0;
            bool m_upKeyPressed = false;
            bool m_downKeyPressed = false;
            bool m_leftKeyPressed = false;
            bool m_rightKeyPressed = false;
            bool m_enterKeyPressed = false;
            bool m_escKeyPressed = false;

            float m_animTime = 0.0f;

            bool m_colourBlindMode = false;
            bool m_inScreenMenu = false;

            std::uint32_t m_screenWidth = 1280;
            std::uint32_t m_screenHeight = 720;
            bool m_fullscreen = false;
            std::uint32_t m_targetFramerate = 60;
            int m_resolutionIndex = 0;
            int m_framerateIndex = 2;

            static const std::vector<Resolution> RESOLUTIONS;
            static const std::vector<std::uint32_t> FRAMERATES;
            static const std::vector<std::string> FRAMERATE_NAMES;

            static bool s_colourBlindMode;
            static std::uint32_t s_screenWidth;
            static std::uint32_t s_screenHeight;
            static bool s_fullscreen;
            static std::uint32_t s_targetFramerate;
            static const std::string SETTINGS_FILE_PATH;
        };

    }
}

