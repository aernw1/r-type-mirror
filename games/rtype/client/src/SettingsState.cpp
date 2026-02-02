/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SettingsState
*/

#include "SettingsState.hpp"
#include "ECS/Components/TextLabel.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include "Core/InputMapping.hpp"
#include <iostream>
#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace RType::ECS;
using json = nlohmann::json;

namespace {
    const std::unordered_map<Renderer::Key, std::string> KEY_TO_STRING = {
        {Renderer::Key::A, "A"}, {Renderer::Key::B, "B"}, {Renderer::Key::C, "C"},
        {Renderer::Key::D, "D"}, {Renderer::Key::E, "E"}, {Renderer::Key::F, "F"},
        {Renderer::Key::G, "G"}, {Renderer::Key::H, "H"}, {Renderer::Key::I, "I"},
        {Renderer::Key::J, "J"}, {Renderer::Key::K, "K"}, {Renderer::Key::L, "L"},
        {Renderer::Key::M, "M"}, {Renderer::Key::N, "N"}, {Renderer::Key::O, "O"},
        {Renderer::Key::P, "P"}, {Renderer::Key::Q, "Q"}, {Renderer::Key::R, "R"},
        {Renderer::Key::S, "S"}, {Renderer::Key::T, "T"}, {Renderer::Key::U, "U"},
        {Renderer::Key::V, "V"}, {Renderer::Key::W, "W"}, {Renderer::Key::X, "X"},
        {Renderer::Key::Y, "Y"}, {Renderer::Key::Z, "Z"},
        {Renderer::Key::Num0, "0"}, {Renderer::Key::Num1, "1"}, {Renderer::Key::Num2, "2"},
        {Renderer::Key::Num3, "3"}, {Renderer::Key::Num4, "4"}, {Renderer::Key::Num5, "5"},
        {Renderer::Key::Num6, "6"}, {Renderer::Key::Num7, "7"}, {Renderer::Key::Num8, "8"},
        {Renderer::Key::Num9, "9"},
        {Renderer::Key::Space, "SPACE"}, {Renderer::Key::Enter, "ENTER"},
        {Renderer::Key::Escape, "ESC"}, {Renderer::Key::Tab, "TAB"},
        {Renderer::Key::Left, "LEFT"}, {Renderer::Key::Right, "RIGHT"},
        {Renderer::Key::Up, "UP"}, {Renderer::Key::Down, "DOWN"},
        {Renderer::Key::LShift, "LSHIFT"}, {Renderer::Key::RShift, "RSHIFT"},
        {Renderer::Key::LControl, "LCTRL"}, {Renderer::Key::RControl, "RCTRL"},
        {Renderer::Key::LAlt, "LALT"}, {Renderer::Key::RAlt, "RALT"}
    };

    const std::unordered_map<std::string, Renderer::Key> STRING_TO_KEY = {
        {"A", Renderer::Key::A}, {"B", Renderer::Key::B}, {"C", Renderer::Key::C},
        {"D", Renderer::Key::D}, {"E", Renderer::Key::E}, {"F", Renderer::Key::F},
        {"G", Renderer::Key::G}, {"H", Renderer::Key::H}, {"I", Renderer::Key::I},
        {"J", Renderer::Key::J}, {"K", Renderer::Key::K}, {"L", Renderer::Key::L},
        {"M", Renderer::Key::M}, {"N", Renderer::Key::N}, {"O", Renderer::Key::O},
        {"P", Renderer::Key::P}, {"Q", Renderer::Key::Q}, {"R", Renderer::Key::R},
        {"S", Renderer::Key::S}, {"T", Renderer::Key::T}, {"U", Renderer::Key::U},
        {"V", Renderer::Key::V}, {"W", Renderer::Key::W}, {"X", Renderer::Key::X},
        {"Y", Renderer::Key::Y}, {"Z", Renderer::Key::Z},
        {"0", Renderer::Key::Num0}, {"1", Renderer::Key::Num1}, {"2", Renderer::Key::Num2},
        {"3", Renderer::Key::Num3}, {"4", Renderer::Key::Num4}, {"5", Renderer::Key::Num5},
        {"6", Renderer::Key::Num6}, {"7", Renderer::Key::Num7}, {"8", Renderer::Key::Num8},
        {"9", Renderer::Key::Num9},
        {"SPACE", Renderer::Key::Space}, {"ENTER", Renderer::Key::Enter},
        {"ESC", Renderer::Key::Escape}, {"TAB", Renderer::Key::Tab},
        {"LEFT", Renderer::Key::Left}, {"RIGHT", Renderer::Key::Right},
        {"UP", Renderer::Key::Up}, {"DOWN", Renderer::Key::Down},
        {"LSHIFT", Renderer::Key::LShift}, {"RSHIFT", Renderer::Key::RShift},
        {"LCTRL", Renderer::Key::LControl}, {"RCTRL", Renderer::Key::RControl},
        {"LALT", Renderer::Key::LAlt}, {"RALT", Renderer::Key::RAlt}
    };

    const std::vector<Renderer::Key> REBINDABLE_KEYS = {
        Renderer::Key::A, Renderer::Key::B, Renderer::Key::C, Renderer::Key::D, Renderer::Key::E,
        Renderer::Key::F, Renderer::Key::G, Renderer::Key::H, Renderer::Key::I, Renderer::Key::J,
        Renderer::Key::K, Renderer::Key::L, Renderer::Key::M, Renderer::Key::N, Renderer::Key::O,
        Renderer::Key::P, Renderer::Key::Q, Renderer::Key::R, Renderer::Key::S, Renderer::Key::T,
        Renderer::Key::U, Renderer::Key::V, Renderer::Key::W, Renderer::Key::X, Renderer::Key::Y,
        Renderer::Key::Z,
        Renderer::Key::Num0, Renderer::Key::Num1, Renderer::Key::Num2, Renderer::Key::Num3,
        Renderer::Key::Num4, Renderer::Key::Num5, Renderer::Key::Num6, Renderer::Key::Num7,
        Renderer::Key::Num8, Renderer::Key::Num9,
        Renderer::Key::Space, Renderer::Key::Enter, Renderer::Key::Escape, Renderer::Key::Tab,
        Renderer::Key::Left, Renderer::Key::Right, Renderer::Key::Up, Renderer::Key::Down,
        Renderer::Key::LShift, Renderer::Key::RShift, Renderer::Key::LControl, Renderer::Key::RControl,
        Renderer::Key::LAlt, Renderer::Key::RAlt
    };

    const std::vector<std::string> ACTION_KEYS = {"MOVE_UP", "MOVE_DOWN", "MOVE_LEFT", "MOVE_RIGHT", "SHOOT"};
}

namespace RType {
    namespace Client {

        const std::string SettingsState::SETTINGS_FILE_PATH = "settings.json";
        bool SettingsState::s_colourBlindMode = false;
        std::uint32_t SettingsState::s_screenWidth = 1280;
        std::uint32_t SettingsState::s_screenHeight = 720;
        bool SettingsState::s_fullscreen = false;
        std::uint32_t SettingsState::s_targetFramerate = 60;
        float SettingsState::s_masterVolume = 1.0f;
        bool SettingsState::s_muted = false;

        const std::vector<SettingsState::Resolution> SettingsState::RESOLUTIONS = {
            {1280, 720, "1280x720"},
            {1920, 1080, "1920x1080"},
            {2560, 1440, "2560x1440"},
            {3840, 2160, "3840x2160"}
        };

        const std::vector<std::uint32_t> SettingsState::FRAMERATES = {30, 60, 120, 0};
        const std::vector<std::string> SettingsState::FRAMERATE_NAMES = {"30", "60", "120", "Unlimited"};

        SettingsState::SettingsState(GameStateMachine& machine, GameContext& context)
            : m_machine(machine), m_context(context) {
            m_renderer = context.renderer;
            m_renderingSystem = std::make_unique<RenderingSystem>(m_renderer.get());
            m_textSystem = std::make_unique<TextRenderingSystem>(m_renderer.get());
        }

        void SettingsState::Init() {
            std::cout << "[SettingsState] Initializing..." << std::endl;

            if (m_context.audio) {
                m_audioSystem = std::make_unique<RType::ECS::AudioSystem>(m_context.audio.get());
                m_selectMusic = m_context.audio->LoadMusic("assets/sounds/select.flac");
                if (m_selectMusic == Audio::INVALID_MUSIC_ID) {
                    m_selectMusic = m_context.audio->LoadMusic("../assets/sounds/select.flac");
                }
            }

            m_fontLarge = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 48);
            if (m_fontLarge == Renderer::INVALID_FONT_ID) {
                m_fontLarge = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 48);
            }

            m_fontMedium = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 24);
            if (m_fontMedium == Renderer::INVALID_FONT_ID) {
                m_fontMedium = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 24);
            }

            m_fontSmall = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 16);
            if (m_fontSmall == Renderer::INVALID_FONT_ID) {
                m_fontSmall = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 16);
            }

            m_bgTexture = m_renderer->LoadTexture("assets/backgrounds/1.jpg");
            if (m_bgTexture == Renderer::INVALID_TEXTURE_ID) {
                m_bgTexture = m_renderer->LoadTexture("../assets/backgrounds/1.jpg");
            }

            loadSettings();
            createUI();
        }

        void SettingsState::Cleanup() {
            std::cout << "[SettingsState] Cleaning up..." << std::endl;

            if (m_context.audio && m_selectMusic != Audio::INVALID_MUSIC_ID) {
                m_context.audio->StopMusic(m_selectMusic);
                m_context.audio->UnloadMusic(m_selectMusic);
                m_selectMusic = Audio::INVALID_MUSIC_ID;
            }

            for (Entity entity : m_entities) {
                if (m_registry.IsEntityAlive(entity)) {
                    m_registry.DestroyEntity(entity);
                }
            }
            m_entities.clear();
        }

        void SettingsState::createUI() {
            m_menuItems.clear();

            if (m_bgTexture != Renderer::INVALID_TEXTURE_ID) {
                Entity bg = m_registry.CreateEntity();
                m_entities.push_back(bg);
                m_registry.AddComponent(bg, Position{0.0f, 0.0f});

                Renderer::SpriteId spriteId = m_renderer->CreateSprite(m_bgTexture, {});
                Drawable drawable(spriteId, -10);

                Renderer::Vector2 texSize = m_renderer->GetTextureSize(m_bgTexture);
                if (texSize.x > 0 && texSize.y > 0) {
                    drawable.scale = {1280.0f / texSize.x, 720.0f / texSize.y};
                }

                m_registry.AddComponent(bg, std::move(drawable));
            }

            if (m_fontLarge == Renderer::INVALID_FONT_ID)
                return;

            m_titleEntity = m_registry.CreateEntity();
            m_entities.push_back(m_titleEntity);
            m_registry.AddComponent(m_titleEntity, Position{640.0f, 150.0f});
            TextLabel titleLabel("SETTINGS", m_fontLarge, 72);
            titleLabel.color = {0.0f, 1.0f, 1.0f, 1.0f};
            titleLabel.centered = true;
            m_registry.AddComponent(m_titleEntity, std::move(titleLabel));

            float startY = 250.0f;
            float itemSpacing = 70.0f;

            Entity colourBlindItem = m_registry.CreateEntity();
            m_entities.push_back(colourBlindItem);
            m_menuItems.push_back(colourBlindItem);
            m_registry.AddComponent(colourBlindItem, Position{640.0f, startY});
            std::string colourBlindText = "COLOUR-BLIND MODE: " + std::string(m_colourBlindMode ? "ON" : "OFF");
            TextLabel colourBlindLabel(colourBlindText, m_fontMedium, 24);
            colourBlindLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            colourBlindLabel.centered = true;
            m_registry.AddComponent(colourBlindItem, std::move(colourBlindLabel));

            Entity screenItem = m_registry.CreateEntity();
            m_entities.push_back(screenItem);
            m_menuItems.push_back(screenItem);
            m_registry.AddComponent(screenItem, Position{640.0f, startY + itemSpacing});
            TextLabel screenLabel("SCREEN", m_fontMedium, 24);
            screenLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            screenLabel.centered = true;
            m_registry.AddComponent(screenItem, std::move(screenLabel));

            Entity audioItem = m_registry.CreateEntity();
            m_entities.push_back(audioItem);
            m_menuItems.push_back(audioItem);
            m_registry.AddComponent(audioItem, Position{640.0f, startY + itemSpacing * 2});
            TextLabel audioLabel("AUDIO", m_fontMedium, 24);
            audioLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            audioLabel.centered = true;
            m_registry.AddComponent(audioItem, std::move(audioLabel));

            Entity commandsItem = m_registry.CreateEntity();
            m_entities.push_back(commandsItem);
            m_menuItems.push_back(commandsItem);
            m_registry.AddComponent(commandsItem, Position{640.0f, startY + itemSpacing * 3});
            TextLabel commandsLabel("COMMANDS", m_fontMedium, 24);
            commandsLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            commandsLabel.centered = true;
            m_registry.AddComponent(commandsItem, std::move(commandsLabel));

            Entity backItem = m_registry.CreateEntity();
            m_entities.push_back(backItem);
            m_menuItems.push_back(backItem);
            m_registry.AddComponent(backItem, Position{640.0f, startY + itemSpacing * 4});
            TextLabel backLabel("BACK", m_fontMedium, 24);
            backLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            backLabel.centered = true;
            m_registry.AddComponent(backItem, std::move(backLabel));

            Entity controlsText = m_registry.CreateEntity();
            m_entities.push_back(controlsText);
            m_registry.AddComponent(controlsText, Position{640.0f, 600.0f});
            TextLabel controlsLabel("USE ARROWS TO NAVIGATE  |  ENTER TO SELECT  |  ESC TO GO BACK", m_fontSmall, 12);
            controlsLabel.color = {0.5f, 0.86f, 1.0f, 0.85f};
            controlsLabel.centered = true;
            m_registry.AddComponent(controlsText, std::move(controlsLabel));
        }

        void SettingsState::createScreenUI() {
            for (Entity entity : m_screenMenuItems) {
                if (m_registry.IsEntityAlive(entity)) {
                    m_registry.DestroyEntity(entity);
                }
            }
            m_screenMenuItems.clear();

            if (m_fontLarge == Renderer::INVALID_FONT_ID)
                return;

            Entity titleEntity = m_registry.CreateEntity();
            m_entities.push_back(titleEntity);
            m_screenMenuItems.push_back(titleEntity);
            m_registry.AddComponent(titleEntity, Position{640.0f, 150.0f});
            TextLabel titleLabel("SCREEN SETTINGS", m_fontLarge, 72);
            titleLabel.color = {0.0f, 1.0f, 1.0f, 1.0f};
            titleLabel.centered = true;
            m_registry.AddComponent(titleEntity, std::move(titleLabel));

            float startY = 280.0f;
            float itemSpacing = 70.0f;

            Entity resolutionItem = m_registry.CreateEntity();
            m_entities.push_back(resolutionItem);
            m_screenMenuItems.push_back(resolutionItem);
            m_registry.AddComponent(resolutionItem, Position{640.0f, startY});
            std::string resolutionText = "RESOLUTION: " + RESOLUTIONS[m_resolutionIndex].name;
            TextLabel resolutionLabel(resolutionText, m_fontMedium, 24);
            resolutionLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            resolutionLabel.centered = true;
            m_registry.AddComponent(resolutionItem, std::move(resolutionLabel));

            Entity fullscreenItem = m_registry.CreateEntity();
            m_entities.push_back(fullscreenItem);
            m_screenMenuItems.push_back(fullscreenItem);
            m_registry.AddComponent(fullscreenItem, Position{640.0f, startY + itemSpacing});
            std::string fullscreenText = "FULLSCREEN: " + std::string(m_fullscreen ? "ON" : "OFF");
            TextLabel fullscreenLabel(fullscreenText, m_fontMedium, 24);
            fullscreenLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            fullscreenLabel.centered = true;
            m_registry.AddComponent(fullscreenItem, std::move(fullscreenLabel));

            Entity framerateItem = m_registry.CreateEntity();
            m_entities.push_back(framerateItem);
            m_screenMenuItems.push_back(framerateItem);
            m_registry.AddComponent(framerateItem, Position{640.0f, startY + itemSpacing * 2});
            std::string framerateText = "FRAME RATE: " + FRAMERATE_NAMES[m_framerateIndex];
            TextLabel framerateLabel(framerateText, m_fontMedium, 24);
            framerateLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            framerateLabel.centered = true;
            m_registry.AddComponent(framerateItem, std::move(framerateLabel));

            Entity backItem = m_registry.CreateEntity();
            m_entities.push_back(backItem);
            m_screenMenuItems.push_back(backItem);
            m_registry.AddComponent(backItem, Position{640.0f, startY + itemSpacing * 3});
            TextLabel backLabel("BACK", m_fontMedium, 24);
            backLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            backLabel.centered = true;
            m_registry.AddComponent(backItem, std::move(backLabel));

            Entity controlsText = m_registry.CreateEntity();
            m_entities.push_back(controlsText);
            m_screenMenuItems.push_back(controlsText);
            m_registry.AddComponent(controlsText, Position{640.0f, 600.0f});
            TextLabel controlsLabel("UP/DOWN: NAVIGATE  |  LEFT/RIGHT: CHANGE VALUE  |  ESC: BACK", m_fontSmall, 12);
            controlsLabel.color = {0.5f, 0.86f, 1.0f, 0.85f};
            controlsLabel.centered = true;
            m_registry.AddComponent(controlsText, std::move(controlsLabel));
        }

        void SettingsState::createAudioUI() {
            for (Entity entity : m_audioMenuItems) {
                if (m_registry.IsEntityAlive(entity)) {
                    m_registry.DestroyEntity(entity);
                }
            }
            m_audioMenuItems.clear();

            if (m_fontLarge == Renderer::INVALID_FONT_ID)
                return;

            Entity titleEntity = m_registry.CreateEntity();
            m_entities.push_back(titleEntity);
            m_audioMenuItems.push_back(titleEntity);
            m_registry.AddComponent(titleEntity, Position{640.0f, 150.0f});
            TextLabel titleLabel("AUDIO SETTINGS", m_fontLarge, 72);
            titleLabel.color = {0.0f, 1.0f, 1.0f, 1.0f};
            titleLabel.centered = true;
            m_registry.AddComponent(titleEntity, std::move(titleLabel));

            float startY = 280.0f;
            float itemSpacing = 70.0f;

            Entity volumeItem = m_registry.CreateEntity();
            m_entities.push_back(volumeItem);
            m_audioMenuItems.push_back(volumeItem);
            m_registry.AddComponent(volumeItem, Position{640.0f, startY});
            int volumePercent = static_cast<int>(std::round(m_masterVolume * 100.0f));
            volumePercent = (volumePercent / 5) * 5;
            std::string volumeText = "MASTER VOLUME: " + std::to_string(volumePercent) + "%";
            TextLabel volumeLabel(volumeText, m_fontMedium, 24);
            volumeLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            volumeLabel.centered = true;
            m_registry.AddComponent(volumeItem, std::move(volumeLabel));

            Entity muteItem = m_registry.CreateEntity();
            m_entities.push_back(muteItem);
            m_audioMenuItems.push_back(muteItem);
            m_registry.AddComponent(muteItem, Position{640.0f, startY + itemSpacing});
            std::string muteText = "MUTE: " + std::string(m_muted ? "ON" : "OFF");
            TextLabel muteLabel(muteText, m_fontMedium, 24);
            muteLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            muteLabel.centered = true;
            m_registry.AddComponent(muteItem, std::move(muteLabel));

            Entity backItem = m_registry.CreateEntity();
            m_entities.push_back(backItem);
            m_audioMenuItems.push_back(backItem);
            m_registry.AddComponent(backItem, Position{640.0f, startY + itemSpacing * 2});
            TextLabel backLabel("BACK", m_fontMedium, 24);
            backLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            backLabel.centered = true;
            m_registry.AddComponent(backItem, std::move(backLabel));

            Entity controlsText = m_registry.CreateEntity();
            m_entities.push_back(controlsText);
            m_audioMenuItems.push_back(controlsText);
            m_registry.AddComponent(controlsText, Position{640.0f, 600.0f});
            TextLabel controlsLabel("UP/DOWN: NAVIGATE  |  LEFT/RIGHT: CHANGE VALUE  |  ESC: BACK", m_fontSmall, 12);
            controlsLabel.color = {0.5f, 0.86f, 1.0f, 0.85f};
            controlsLabel.centered = true;
            m_registry.AddComponent(controlsText, std::move(controlsLabel));
        }

        void SettingsState::createCommandsUI() {
            for (Entity entity : m_commandsMenuItems) {
                if (m_registry.IsEntityAlive(entity)) {
                    m_registry.DestroyEntity(entity);
                }
            }
            m_commandsMenuItems.clear();

            if (m_fontLarge == Renderer::INVALID_FONT_ID)
                return;

            Entity titleEntity = m_registry.CreateEntity();
            m_entities.push_back(titleEntity);
            m_commandsMenuItems.push_back(titleEntity);
            m_registry.AddComponent(titleEntity, Position{640.0f, 150.0f});
            TextLabel titleLabel("COMMANDS", m_fontLarge, 72);
            titleLabel.color = {0.0f, 1.0f, 1.0f, 1.0f};
            titleLabel.centered = true;
            m_registry.AddComponent(titleEntity, std::move(titleLabel));

            float startY = 260.0f;
            float itemSpacing = 55.0f;

            std::vector<std::string> actionNames = {"MOVE UP", "MOVE DOWN", "MOVE LEFT", "MOVE RIGHT", "SHOOT"};

            for (size_t i = 0; i < actionNames.size(); i++) {
                Entity actionItem = m_registry.CreateEntity();
                m_entities.push_back(actionItem);
                m_commandsMenuItems.push_back(actionItem);
                m_registry.AddComponent(actionItem, Position{640.0f, startY + itemSpacing * i});

                Renderer::Key currentKey = Core::InputMapping::GetKey(ACTION_KEYS[i]);
                std::string keyName = keyToString(currentKey);
                std::string actionText = actionNames[i] + ": " + keyName;
                if (m_waitingForRebind && m_rebindingActionIndex == static_cast<int>(i)) {
                    actionText = actionNames[i] + ": PRESS A KEY...";
                }

                TextLabel actionLabel(actionText, m_fontMedium, 24);
                actionLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
                actionLabel.centered = true;
                m_registry.AddComponent(actionItem, std::move(actionLabel));
            }

            Entity backItem = m_registry.CreateEntity();
            m_entities.push_back(backItem);
            m_commandsMenuItems.push_back(backItem);
            m_registry.AddComponent(backItem, Position{640.0f, startY + itemSpacing * 5});
            TextLabel backLabel("BACK", m_fontMedium, 24);
            backLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            backLabel.centered = true;
            m_registry.AddComponent(backItem, std::move(backLabel));

            Entity controlsText = m_registry.CreateEntity();
            m_entities.push_back(controlsText);
            m_commandsMenuItems.push_back(controlsText);
            m_registry.AddComponent(controlsText, Position{640.0f, 600.0f});
            std::string controlsTextStr = m_waitingForRebind ?
                "PRESS ANY KEY TO REBIND  |  ESC TO CANCEL" :
                "UP/DOWN: NAVIGATE  |  ENTER: REBIND  |  ESC: BACK";
            TextLabel controlsLabel(controlsTextStr, m_fontSmall, 12);
            controlsLabel.color = {0.5f, 0.86f, 1.0f, 0.85f};
            controlsLabel.centered = true;
            m_registry.AddComponent(controlsText, std::move(controlsLabel));
        }

        void SettingsState::updateAnimations(float dt) {
            m_animTime += dt;
        }

        void SettingsState::updateMenuSelection() {
            if (m_inScreenMenu) {
                updateScreenMenuSelection();
                return;
            }
            if (m_inAudioMenu) {
                updateAudioMenuSelection();
                return;
            }
            if (m_inCommandsMenu) {
                updateCommandsMenuSelection();
                return;
            }

            for (size_t i = 0; i < m_menuItems.size(); i++) {
                if (m_registry.IsEntityAlive(m_menuItems[i])) {
                    auto& label = m_registry.GetComponent<TextLabel>(m_menuItems[i]);

                    if (static_cast<int>(i) == m_selectedIndex) {
                        float pulse = std::sin(m_animTime * 4.0f) * 0.3f + 0.7f;
                        label.color = {1.0f, 0.08f + pulse * 0.5f, 0.58f, 1.0f};
                    } else {
                        label.color = {0.7f, 0.7f, 0.7f, 1.0f};
                    }
                }
            }

            if (m_menuItems.size() > 0 && m_registry.IsEntityAlive(m_menuItems[0])) {
                auto& label = m_registry.GetComponent<TextLabel>(m_menuItems[0]);
                std::string colourBlindText = "COLOUR-BLIND MODE: " + std::string(m_colourBlindMode ? "ON" : "OFF");
                label.text = colourBlindText;
            }
        }

        void SettingsState::updateScreenMenuSelection() {
            for (size_t i = 0; i < m_screenMenuItems.size(); i++) {
                if (m_registry.IsEntityAlive(m_screenMenuItems[i])) {
                    auto& label = m_registry.GetComponent<TextLabel>(m_screenMenuItems[i]);

                    if (i == 0) {
                        continue;
                    }

                    int itemIndex = static_cast<int>(i) - 1;
                    if (itemIndex == m_screenSelectedIndex) {
                        float pulse = std::sin(m_animTime * 4.0f) * 0.3f + 0.7f;
                        label.color = {1.0f, 0.08f + pulse * 0.5f, 0.58f, 1.0f};
                    } else {
                        label.color = {0.7f, 0.7f, 0.7f, 1.0f};
                    }
                }
            }

            if (m_screenMenuItems.size() > 1 && m_registry.IsEntityAlive(m_screenMenuItems[1])) {
                auto& label = m_registry.GetComponent<TextLabel>(m_screenMenuItems[1]);
                std::string resolutionText = "RESOLUTION: " + RESOLUTIONS[m_resolutionIndex].name;
                label.text = resolutionText;
            }

            if (m_screenMenuItems.size() > 2 && m_registry.IsEntityAlive(m_screenMenuItems[2])) {
                auto& label = m_registry.GetComponent<TextLabel>(m_screenMenuItems[2]);
                std::string fullscreenText = "FULLSCREEN: " + std::string(m_fullscreen ? "ON" : "OFF");
                label.text = fullscreenText;
            }

            if (m_screenMenuItems.size() > 3 && m_registry.IsEntityAlive(m_screenMenuItems[3])) {
                auto& label = m_registry.GetComponent<TextLabel>(m_screenMenuItems[3]);
                std::string framerateText = "FRAME RATE: " + FRAMERATE_NAMES[m_framerateIndex];
                label.text = framerateText;
            }
        }

        void SettingsState::updateAudioMenuSelection() {
            for (size_t i = 0; i < m_audioMenuItems.size(); i++) {
                if (m_registry.IsEntityAlive(m_audioMenuItems[i])) {
                    auto& label = m_registry.GetComponent<TextLabel>(m_audioMenuItems[i]);

                    if (i == 0) {
                        continue;
                    }

                    int itemIndex = static_cast<int>(i) - 1;
                    if (itemIndex == m_audioSelectedIndex) {
                        float pulse = std::sin(m_animTime * 4.0f) * 0.3f + 0.7f;
                        label.color = {1.0f, 0.08f + pulse * 0.5f, 0.58f, 1.0f};
                    } else {
                        label.color = {0.7f, 0.7f, 0.7f, 1.0f};
                    }
                }
            }

            if (m_audioMenuItems.size() > 1 && m_registry.IsEntityAlive(m_audioMenuItems[1])) {
                auto& label = m_registry.GetComponent<TextLabel>(m_audioMenuItems[1]);
                int volumePercent = static_cast<int>(std::round(m_masterVolume * 100.0f));
                volumePercent = (volumePercent / 5) * 5;
                std::string volumeText = "MASTER VOLUME: " + std::to_string(volumePercent) + "%";
                label.text = volumeText;
            }

            if (m_audioMenuItems.size() > 2 && m_registry.IsEntityAlive(m_audioMenuItems[2])) {
                auto& label = m_registry.GetComponent<TextLabel>(m_audioMenuItems[2]);
                std::string muteText = "MUTE: " + std::string(m_muted ? "ON" : "OFF");
                label.text = muteText;
            }
        }

        void SettingsState::updateCommandsMenuSelection() {
            std::vector<std::string> actionNames = {"MOVE UP", "MOVE DOWN", "MOVE LEFT", "MOVE RIGHT", "SHOOT"};

            for (size_t i = 0; i < m_commandsMenuItems.size(); i++) {
                if (m_registry.IsEntityAlive(m_commandsMenuItems[i])) {
                    auto& label = m_registry.GetComponent<TextLabel>(m_commandsMenuItems[i]);

                    if (i == 0) {
                        continue;
                    }

                    int itemIndex = static_cast<int>(i) - 1;
                    if (itemIndex == m_commandsSelectedIndex) {
                        float pulse = std::sin(m_animTime * 4.0f) * 0.3f + 0.7f;
                        label.color = {1.0f, 0.08f + pulse * 0.5f, 0.58f, 1.0f};
                    } else {
                        label.color = {0.7f, 0.7f, 0.7f, 1.0f};
                    }

                    if (itemIndex >= 0 && itemIndex < static_cast<int>(actionNames.size())) {
                        Renderer::Key currentKey = Core::InputMapping::GetKey(ACTION_KEYS[itemIndex]);
                        std::string keyName = keyToString(currentKey);
                        std::string actionText = actionNames[itemIndex] + ": " + keyName;
                        if (m_waitingForRebind && m_rebindingActionIndex == itemIndex) {
                            actionText = actionNames[itemIndex] + ": PRESS A KEY...";
                        }
                        label.text = actionText;
                    } else if (itemIndex == static_cast<int>(actionNames.size())) {
                        label.text = "BACK";
                    }
                }
            }
        }

        void SettingsState::HandleInput() {
            auto playSelectSound = [this]() {
                if (m_context.audio && m_selectMusic != Audio::INVALID_MUSIC_ID) {
                    m_context.audio->StopMusic(m_selectMusic);
                    Audio::PlaybackOptions opts;
                    opts.volume = 1.0f;
                    opts.loop = false;
                    m_context.audio->PlayMusic(m_selectMusic, opts);
                }
            };

            if (m_inScreenMenu) {
                if (m_renderer->IsKeyPressed(Renderer::Key::Up) && !m_upKeyPressed) {
                    m_upKeyPressed = true;
                    playSelectSound();
                    m_screenSelectedIndex--;
                    if (m_screenSelectedIndex < 0) {
                        m_screenSelectedIndex = static_cast<int>(ScreenItem::COUNT) - 1;
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Up)) {
                    m_upKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Down) && !m_downKeyPressed) {
                    m_downKeyPressed = true;
                    playSelectSound();
                    m_screenSelectedIndex++;
                    if (m_screenSelectedIndex >= static_cast<int>(ScreenItem::COUNT)) {
                        m_screenSelectedIndex = 0;
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Down)) {
                    m_downKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Left) && !m_leftKeyPressed) {
                    m_leftKeyPressed = true;
                    playSelectSound();
                    switch (static_cast<ScreenItem>(m_screenSelectedIndex)) {
                        case ScreenItem::RESOLUTION:
                            changeResolution(-1);
                            break;
                        case ScreenItem::FULLSCREEN:
                            toggleFullscreen();
                            break;
                        case ScreenItem::FRAME_RATE:
                            changeFrameRate(-1);
                            break;
                        default:
                            break;
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Left)) {
                    m_leftKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Right) && !m_rightKeyPressed) {
                    m_rightKeyPressed = true;
                    playSelectSound();
                    switch (static_cast<ScreenItem>(m_screenSelectedIndex)) {
                        case ScreenItem::RESOLUTION:
                            changeResolution(1);
                            break;
                        case ScreenItem::FULLSCREEN:
                            toggleFullscreen();
                            break;
                        case ScreenItem::FRAME_RATE:
                            changeFrameRate(1);
                            break;
                        default:
                            break;
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Right)) {
                    m_rightKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Enter) && !m_enterKeyPressed) {
                    m_enterKeyPressed = true;
                    playSelectSound();
                    if (static_cast<ScreenItem>(m_screenSelectedIndex) == ScreenItem::BACK) {
                        exitSubMenu();
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Enter)) {
                    m_enterKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escKeyPressed) {
                    m_escKeyPressed = true;
                    playSelectSound();
                    exitSubMenu();
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                    m_escKeyPressed = false;
                }
                return;
            }

            if (m_inAudioMenu) {
                if (m_renderer->IsKeyPressed(Renderer::Key::Up) && !m_upKeyPressed) {
                    m_upKeyPressed = true;
                    playSelectSound();
                    m_audioSelectedIndex--;
                    if (m_audioSelectedIndex < 0) {
                        m_audioSelectedIndex = static_cast<int>(AudioItem::COUNT) - 1;
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Up)) {
                    m_upKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Down) && !m_downKeyPressed) {
                    m_downKeyPressed = true;
                    playSelectSound();
                    m_audioSelectedIndex++;
                    if (m_audioSelectedIndex >= static_cast<int>(AudioItem::COUNT)) {
                        m_audioSelectedIndex = 0;
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Down)) {
                    m_downKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Left) && !m_leftKeyPressed) {
                    m_leftKeyPressed = true;
                    playSelectSound();
                    switch (static_cast<AudioItem>(m_audioSelectedIndex)) {
                        case AudioItem::MASTER_VOLUME:
                            changeMasterVolume(-1);
                            break;
                        case AudioItem::MUTE:
                            toggleMute();
                            break;
                        default:
                            break;
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Left)) {
                    m_leftKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Right) && !m_rightKeyPressed) {
                    m_rightKeyPressed = true;
                    playSelectSound();
                    switch (static_cast<AudioItem>(m_audioSelectedIndex)) {
                        case AudioItem::MASTER_VOLUME:
                            changeMasterVolume(1);
                            break;
                        case AudioItem::MUTE:
                            toggleMute();
                            break;
                        default:
                            break;
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Right)) {
                    m_rightKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Enter) && !m_enterKeyPressed) {
                    m_enterKeyPressed = true;
                    playSelectSound();
                    if (static_cast<AudioItem>(m_audioSelectedIndex) == AudioItem::BACK) {
                        exitSubMenu();
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Enter)) {
                    m_enterKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escKeyPressed) {
                    m_escKeyPressed = true;
                    playSelectSound();
                    exitSubMenu();
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                    m_escKeyPressed = false;
                }
                return;
            }

            if (m_inCommandsMenu) {
                if (m_waitingForRebind) {
                    processRebind();
                    return;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Up) && !m_upKeyPressed) {
                    m_upKeyPressed = true;
                    playSelectSound();
                    m_commandsSelectedIndex--;
                    if (m_commandsSelectedIndex < 0) {
                        m_commandsSelectedIndex = static_cast<int>(CommandsItem::COUNT) - 1;
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Up)) {
                    m_upKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Down) && !m_downKeyPressed) {
                    m_downKeyPressed = true;
                    playSelectSound();
                    m_commandsSelectedIndex++;
                    if (m_commandsSelectedIndex >= static_cast<int>(CommandsItem::COUNT)) {
                        m_commandsSelectedIndex = 0;
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Down)) {
                    m_downKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Enter) && !m_enterKeyPressed) {
                    m_enterKeyPressed = true;
                    playSelectSound();
                    if (static_cast<CommandsItem>(m_commandsSelectedIndex) == CommandsItem::BACK) {
                        exitSubMenu();
                    } else if (m_commandsSelectedIndex >= 0 && m_commandsSelectedIndex < 5) {
                        startRebind(m_commandsSelectedIndex);
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Enter)) {
                    m_enterKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escKeyPressed) {
                    m_escKeyPressed = true;
                    playSelectSound();
                    exitSubMenu();
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                    m_escKeyPressed = false;
                }
                return;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Up) && !m_upKeyPressed) {
                m_upKeyPressed = true;
                playSelectSound();
                m_selectedIndex--;
                if (m_selectedIndex < 0) {
                    m_selectedIndex = static_cast<int>(SettingsItem::COUNT) - 1;
                }
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Up)) {
                m_upKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Down) && !m_downKeyPressed) {
                m_downKeyPressed = true;
                playSelectSound();
                m_selectedIndex++;
                if (m_selectedIndex >= static_cast<int>(SettingsItem::COUNT)) {
                    m_selectedIndex = 0;
                }
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Down)) {
                m_downKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Enter) && !m_enterKeyPressed) {
                m_enterKeyPressed = true;
                playSelectSound();

                switch (static_cast<SettingsItem>(m_selectedIndex)) {
                    case SettingsItem::COLOUR_BLIND:
                        toggleColourBlindMode();
                        break;

                    case SettingsItem::SCREEN:
                        enterSubMenu(true);
                        break;

                    case SettingsItem::AUDIO:
                        enterSubMenu(false);
                        break;

                    case SettingsItem::COMMANDS:
                        enterCommandsMenu();
                        break;

                    case SettingsItem::BACK:
                        m_machine.PopState();
                        break;

                    default:
                        break;
                }

            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Enter)) {
                m_enterKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escKeyPressed) {
                m_escKeyPressed = true;
                playSelectSound();
                m_machine.PopState();
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                m_escKeyPressed = false;
            }
        }

        void SettingsState::Update(float dt) {
            updateAnimations(dt);
            updateMenuSelection();

            if (m_audioSystem) {
                m_audioSystem->Update(m_registry, dt);
            }
        }

        void SettingsState::Draw() {
            m_renderer->Clear({0.05f, 0.05f, 0.1f, 1.0f});

            m_renderingSystem->Update(m_registry, 0.0f);
            m_textSystem->Update(m_registry, 0.0f);
        }

        void SettingsState::toggleColourBlindMode() {
            m_colourBlindMode = !m_colourBlindMode;
            s_colourBlindMode = m_colourBlindMode;
            SetColourBlindMode(m_colourBlindMode);
            saveSettings();
            std::cout << "[SettingsState] Colour-blind mode: " << (m_colourBlindMode ? "ON" : "OFF") << std::endl;
        }

        void SettingsState::saveSettings() {
            try {
                json j;
                j["colourBlindMode"] = m_colourBlindMode;
                j["screenWidth"] = m_screenWidth;
                j["screenHeight"] = m_screenHeight;
                j["fullscreen"] = m_fullscreen;
                j["targetFramerate"] = m_targetFramerate;
                j["masterVolume"] = m_masterVolume;
                j["muted"] = m_muted;

                json inputMappings;
                std::vector<std::string> actionKeys = {"MOVE_UP", "MOVE_DOWN", "MOVE_LEFT", "MOVE_RIGHT", "SHOOT"};
                for (const auto& action : actionKeys) {
                    Renderer::Key key = Core::InputMapping::GetKey(action);
                    if (key != Renderer::Key::Unknown) {
                        inputMappings[action] = keyToString(key);
                    }
                }
                j["inputMappings"] = inputMappings;

                std::string filePath = SETTINGS_FILE_PATH;
                std::ofstream file(filePath);
                if (!file.is_open()) {
                    filePath = "../" + SETTINGS_FILE_PATH;
                    file.open(filePath);
                }

                if (file.is_open()) {
                    file << j.dump(4);
                    file.close();
                    Core::Logger::Info("[SettingsState] Settings saved to {}", filePath);
                } else {
                    Core::Logger::Warning("[SettingsState] Failed to save settings to {}", SETTINGS_FILE_PATH);
                }
            } catch (const std::exception& e) {
                Core::Logger::Error("[SettingsState] Error saving settings: {}", e.what());
            }
        }

        void SettingsState::loadSettings() {
            m_colourBlindMode = s_colourBlindMode;
            m_screenWidth = s_screenWidth;
            m_screenHeight = s_screenHeight;
            m_fullscreen = s_fullscreen;
            m_targetFramerate = s_targetFramerate;
            m_masterVolume = s_masterVolume;
            m_muted = s_muted;
            m_volumeBeforeMute = m_masterVolume;

            if (m_context.audio) {
                if (m_muted) {
                    m_context.audio->SetMasterVolume(0.0f);
                } else {
                    m_context.audio->SetMasterVolume(m_masterVolume);
                }
            }

            m_resolutionIndex = 0;
            for (size_t i = 0; i < RESOLUTIONS.size(); i++) {
                if (RESOLUTIONS[i].width == m_screenWidth && RESOLUTIONS[i].height == m_screenHeight) {
                    m_resolutionIndex = static_cast<int>(i);
                    break;
                }
            }

            m_framerateIndex = 2;
            for (size_t i = 0; i < FRAMERATES.size(); i++) {
                if (FRAMERATES[i] == m_targetFramerate) {
                    m_framerateIndex = static_cast<int>(i);
                    break;
                }
            }
        }

        void SettingsState::LoadSettingsFromFile() {
            try {
                std::string filePath = SETTINGS_FILE_PATH;
                std::ifstream file(filePath);
                if (!file.is_open()) {
                    filePath = "../" + SETTINGS_FILE_PATH;
                    file.open(filePath);
                }

                if (!file.is_open()) {
                    Core::Logger::Info("[SettingsState] Settings file not found, using defaults");
                    s_colourBlindMode = false;
                    SetColourBlindMode(false);
                    s_screenWidth = 1280;
                    s_screenHeight = 720;
                    s_fullscreen = false;
                    s_targetFramerate = 60;
                    s_masterVolume = 1.0f;
                    s_muted = false;
                    return;
                }

                json j;
                file >> j;
                file.close();

                if (j.contains("colourBlindMode")) {
                    s_colourBlindMode = j["colourBlindMode"].get<bool>();
                    SetColourBlindMode(s_colourBlindMode);
                } else {
                    s_colourBlindMode = false;
                    SetColourBlindMode(false);
                }

                if (j.contains("screenWidth")) {
                    s_screenWidth = j["screenWidth"].get<std::uint32_t>();
                } else {
                    s_screenWidth = 1280;
                }
                if (j.contains("screenHeight")) {
                    s_screenHeight = j["screenHeight"].get<std::uint32_t>();
                } else {
                    s_screenHeight = 720;
                }
                if (j.contains("fullscreen")) {
                    s_fullscreen = j["fullscreen"].get<bool>();
                } else {
                    s_fullscreen = false;
                }
                if (j.contains("targetFramerate")) {
                    s_targetFramerate = j["targetFramerate"].get<std::uint32_t>();
                } else {
                    s_targetFramerate = 60;
                }

                if (j.contains("masterVolume")) {
                    s_masterVolume = j["masterVolume"].get<float>();
                } else {
                    s_masterVolume = 1.0f;
                }

                if (j.contains("muted")) {
                    s_muted = j["muted"].get<bool>();
                } else {
                    s_muted = false;
                }

                if (j.contains("inputMappings") && j["inputMappings"].is_object()) {
                    for (auto& [action, keyStr] : j["inputMappings"].items()) {
                        Renderer::Key key = stringToKey(keyStr.get<std::string>());
                        if (key != Renderer::Key::Unknown) {
                            Core::InputMapping::SetKey(action, key);
                        }
                    }
                }

                Core::Logger::Info("[SettingsState] Settings loaded: colourBlindMode={}, resolution={}x{}, fullscreen={}, framerate={}, masterVolume={}, muted={}",
                                   s_colourBlindMode, s_screenWidth, s_screenHeight, s_fullscreen, s_targetFramerate, s_masterVolume, s_muted);
            } catch (const std::exception& e) {
                Core::Logger::Error("[SettingsState] Error loading settings: {}", e.what());
                s_colourBlindMode = false;
                SetColourBlindMode(false);
                s_screenWidth = 1280;
                s_screenHeight = 720;
                s_fullscreen = false;
                s_targetFramerate = 60;
                s_masterVolume = 1.0f;
                s_muted = false;
            }
        }

        bool SettingsState::IsColourBlindModeEnabled() {
            return s_colourBlindMode;
        }

        void SettingsState::SetColourBlindMode(bool enabled) {
            s_colourBlindMode = enabled;
            Core::ColorFilter::SetColourBlindMode(enabled);
        }

        void SettingsState::clearMenuUI() {
            for (auto it = m_entities.begin(); it != m_entities.end();) {
                Entity entity = *it;
                if (m_registry.IsEntityAlive(entity)) {
                    bool isBackground = false;
                    if (m_registry.HasComponent<Drawable>(entity)) {
                        const auto& drawable = m_registry.GetComponent<Drawable>(entity);
                        if (drawable.layer == -10) {
                            isBackground = true;
                        }
                    }
                    if (!isBackground) {
                        m_registry.DestroyEntity(entity);
                        it = m_entities.erase(it);
                    } else {
                        ++it;
                    }
                } else {
                    it = m_entities.erase(it);
                }
            }
        }

        void SettingsState::enterSubMenu(bool screen) {
            for (Entity entity : m_menuItems) {
                if (m_registry.IsEntityAlive(entity)) {
                    m_registry.DestroyEntity(entity);
                }
            }
            m_menuItems.clear();

            if (m_titleEntity != RType::ECS::NULL_ENTITY && m_registry.IsEntityAlive(m_titleEntity)) {
                m_registry.DestroyEntity(m_titleEntity);
                m_titleEntity = RType::ECS::NULL_ENTITY;
            }

            clearMenuUI();

            if (screen) {
                m_inScreenMenu = true;
                m_screenSelectedIndex = 0;
                createScreenUI();
            } else {
                m_inAudioMenu = true;
                m_audioSelectedIndex = 0;
                createAudioUI();
            }
        }

        void SettingsState::exitSubMenu() {
            std::vector<Entity>* menuItems = nullptr;
            if (m_inScreenMenu) {
                menuItems = &m_screenMenuItems;
            } else if (m_inAudioMenu) {
                menuItems = &m_audioMenuItems;
            } else if (m_inCommandsMenu) {
                menuItems = &m_commandsMenuItems;
            }

            if (menuItems) {
                for (Entity entity : *menuItems) {
                    if (m_registry.IsEntityAlive(entity)) {
                        m_registry.DestroyEntity(entity);
                    }
                }
                menuItems->clear();
            }

            clearMenuUI();

            int savedSelectedIndex = m_selectedIndex;
            if (m_inScreenMenu) {
                savedSelectedIndex = static_cast<int>(SettingsItem::SCREEN);
            } else if (m_inAudioMenu) {
                savedSelectedIndex = static_cast<int>(SettingsItem::AUDIO);
            } else if (m_inCommandsMenu) {
                savedSelectedIndex = static_cast<int>(SettingsItem::COMMANDS);
            }

            m_inScreenMenu = false;
            m_inAudioMenu = false;
            m_inCommandsMenu = false;
            m_waitingForRebind = false;
            m_rebindingActionIndex = -1;
            m_screenSelectedIndex = 0;
            m_audioSelectedIndex = 0;
            m_commandsSelectedIndex = 0;
            m_selectedIndex = savedSelectedIndex;
            createUI();
            updateMenuSelection();
        }

        void SettingsState::enterCommandsMenu() {
            clearMenuUI();
            m_inCommandsMenu = true;
            m_commandsSelectedIndex = 0;
            m_waitingForRebind = false;
            m_rebindingActionIndex = -1;
            createCommandsUI();
        }

        void SettingsState::startRebind(int actionIndex) {
            m_waitingForRebind = true;
            m_rebindingActionIndex = actionIndex;
            m_enterKeyPressed = true;
            createCommandsUI();
        }

        void SettingsState::processRebind() {
            if (m_renderer->IsKeyPressed(Renderer::Key::Enter)) {
                m_enterKeyPressed = true;
                return;
            }

            if (m_enterKeyPressed && !m_renderer->IsKeyPressed(Renderer::Key::Enter)) {
                m_enterKeyPressed = false;
                return;
            }

            for (Renderer::Key key : REBINDABLE_KEYS) {
                if (m_renderer->IsKeyPressed(key)) {
                    if (key == Renderer::Key::Escape) {
                        m_waitingForRebind = false;
                        m_rebindingActionIndex = -1;
                        m_enterKeyPressed = false;
                        createCommandsUI();
                        return;
                    }

                    if (m_rebindingActionIndex >= 0 && m_rebindingActionIndex < static_cast<int>(ACTION_KEYS.size())) {
                        Core::InputMapping::SetKey(ACTION_KEYS[m_rebindingActionIndex], key);
                        saveSettings();
                        m_waitingForRebind = false;
                        m_rebindingActionIndex = -1;
                        m_enterKeyPressed = false;
                        createCommandsUI();
                    }
                    return;
                }
            }
        }

        std::string SettingsState::keyToString(Renderer::Key key) {
            auto it = KEY_TO_STRING.find(key);
            return (it != KEY_TO_STRING.end()) ? it->second : "UNKNOWN";
        }

        Renderer::Key SettingsState::stringToKey(const std::string& str) {
            auto it = STRING_TO_KEY.find(str);
            return (it != STRING_TO_KEY.end()) ? it->second : Renderer::Key::Unknown;
        }


        void SettingsState::changeResolution(int direction) {
            m_resolutionIndex += direction;
            if (m_resolutionIndex < 0) {
                m_resolutionIndex = static_cast<int>(RESOLUTIONS.size()) - 1;
            } else if (m_resolutionIndex >= static_cast<int>(RESOLUTIONS.size())) {
                m_resolutionIndex = 0;
            }
            m_screenWidth = RESOLUTIONS[m_resolutionIndex].width;
            m_screenHeight = RESOLUTIONS[m_resolutionIndex].height;
            applyScreenSettings();
        }

        void SettingsState::toggleFullscreen() {
            m_fullscreen = !m_fullscreen;
            applyScreenSettings();
        }

        void SettingsState::changeFrameRate(int direction) {
            m_framerateIndex += direction;
            if (m_framerateIndex < 0) {
                m_framerateIndex = static_cast<int>(FRAMERATES.size()) - 1;
            } else if (m_framerateIndex >= static_cast<int>(FRAMERATES.size())) {
                m_framerateIndex = 0;
            }
            m_targetFramerate = FRAMERATES[m_framerateIndex];
            applyScreenSettings();
        }

        void SettingsState::changeMasterVolume(int direction) {
            if (m_muted) {
                return;
            }

            float step = 0.05f;
            m_masterVolume += direction * step;
            m_masterVolume = std::clamp(m_masterVolume, 0.0f, 1.0f);

            if (m_context.audio) {
                m_context.audio->SetMasterVolume(m_masterVolume);
            }

            s_masterVolume = m_masterVolume;
            saveSettings();
        }

        void SettingsState::toggleMute() {
            m_muted = !m_muted;

            if (m_context.audio) {
                if (m_muted) {
                    m_volumeBeforeMute = m_masterVolume;
                    m_context.audio->SetMasterVolume(0.0f);
                } else {
                    m_masterVolume = m_volumeBeforeMute;
                    m_context.audio->SetMasterVolume(m_masterVolume);
                }
            }

            s_muted = m_muted;
            s_masterVolume = m_masterVolume;
            saveSettings();
        }

        void SettingsState::applyScreenSettings() {
            Renderer::WindowConfig config;
            config.title = "R-Type - " + m_context.playerName;
            config.width = m_screenWidth;
            config.height = m_screenHeight;
            config.fullscreen = m_fullscreen;
            config.resizable = !m_fullscreen;
            config.targetFramerate = m_targetFramerate;

            s_screenWidth = m_screenWidth;
            s_screenHeight = m_screenHeight;
            s_fullscreen = m_fullscreen;
            s_targetFramerate = m_targetFramerate;

            m_renderer->Destroy();
            if (!m_renderer->CreateWindow(config)) {
                Core::Logger::Error("[SettingsState] Failed to recreate window with new settings");
                m_screenWidth = 1280;
                m_screenHeight = 720;
                m_fullscreen = false;
                m_targetFramerate = 60;
                m_resolutionIndex = 0;
                m_framerateIndex = 2;
                s_screenWidth = 1280;
                s_screenHeight = 720;
                s_fullscreen = false;
                s_targetFramerate = 60;
                config.width = 1280;
                config.height = 720;
                config.fullscreen = false;
                config.targetFramerate = 60;
                m_renderer->CreateWindow(config);
            }

            saveSettings();
        }

        std::uint32_t SettingsState::GetScreenWidth() {
            return s_screenWidth;
        }

        std::uint32_t SettingsState::GetScreenHeight() {
            return s_screenHeight;
        }

        bool SettingsState::GetFullscreen() {
            return s_fullscreen;
        }

        std::uint32_t SettingsState::GetTargetFramerate() {
            return s_targetFramerate;
        }

        float SettingsState::GetMasterVolume() {
            return s_masterVolume;
        }

        bool SettingsState::IsMuted() {
            return s_muted;
        }

    }
}

