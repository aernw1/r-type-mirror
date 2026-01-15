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
#include <iostream>
#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace RType::ECS;
using json = nlohmann::json;

namespace RType {
    namespace Client {

        const std::string SettingsState::SETTINGS_FILE_PATH = "settings.json";
        bool SettingsState::s_colourBlindMode = false;
        std::uint32_t SettingsState::s_screenWidth = 1280;
        std::uint32_t SettingsState::s_screenHeight = 720;
        bool SettingsState::s_fullscreen = false;
        std::uint32_t SettingsState::s_targetFramerate = 60;

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

            float startY = 300.0f;
            float itemSpacing = 80.0f;

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

            Entity backItem = m_registry.CreateEntity();
            m_entities.push_back(backItem);
            m_menuItems.push_back(backItem);
            m_registry.AddComponent(backItem, Position{640.0f, startY + itemSpacing * 2});
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

        void SettingsState::updateAnimations(float dt) {
            m_animTime += dt;
        }

        void SettingsState::updateMenuSelection() {
            if (m_inScreenMenu) {
                updateScreenMenuSelection();
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
                        exitScreenMenu();
                    }
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Enter)) {
                    m_enterKeyPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escKeyPressed) {
                    m_escKeyPressed = true;
                    playSelectSound();
                    exitScreenMenu();
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
                        enterScreenMenu();
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

                Core::Logger::Info("[SettingsState] Settings loaded: colourBlindMode={}, resolution={}x{}, fullscreen={}, framerate={}",
                                   s_colourBlindMode, s_screenWidth, s_screenHeight, s_fullscreen, s_targetFramerate);
            } catch (const std::exception& e) {
                Core::Logger::Error("[SettingsState] Error loading settings: {}", e.what());
                s_colourBlindMode = false;
                SetColourBlindMode(false);
                s_screenWidth = 1280;
                s_screenHeight = 720;
                s_fullscreen = false;
                s_targetFramerate = 60;
            }
        }

        bool SettingsState::IsColourBlindModeEnabled() {
            return s_colourBlindMode;
        }

        void SettingsState::SetColourBlindMode(bool enabled) {
            s_colourBlindMode = enabled;
            Core::ColorFilter::SetColourBlindMode(enabled);
        }

        void SettingsState::enterScreenMenu() {
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

            m_inScreenMenu = true;
            m_screenSelectedIndex = 0;
            createScreenUI();
        }

        void SettingsState::exitScreenMenu() {
            for (Entity entity : m_screenMenuItems) {
                if (m_registry.IsEntityAlive(entity)) {
                    m_registry.DestroyEntity(entity);
                }
            }
            m_screenMenuItems.clear();

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

            m_inScreenMenu = false;
            m_screenSelectedIndex = 0;
            createUI();
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

    }
}

