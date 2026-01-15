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

            Entity backItem = m_registry.CreateEntity();
            m_entities.push_back(backItem);
            m_menuItems.push_back(backItem);
            m_registry.AddComponent(backItem, Position{640.0f, startY + itemSpacing});
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

        void SettingsState::updateAnimations(float dt) {
            m_animTime += dt;
        }

        void SettingsState::updateMenuSelection() {
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
            LoadSettingsFromFile();
            m_colourBlindMode = s_colourBlindMode;
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
                    return;
                }

                json j;
                file >> j;
                file.close();

                if (j.contains("colourBlindMode")) {
                    s_colourBlindMode = j["colourBlindMode"].get<bool>();
                    SetColourBlindMode(s_colourBlindMode);
                    Core::Logger::Info("[SettingsState] Settings loaded: colourBlindMode={}", s_colourBlindMode);
                } else {
                    s_colourBlindMode = false;
                    SetColourBlindMode(false);
                }
            } catch (const std::exception& e) {
                Core::Logger::Error("[SettingsState] Error loading settings: {}", e.what());
                s_colourBlindMode = false;
                SetColourBlindMode(false);
            }
        }

        bool SettingsState::IsColourBlindModeEnabled() {
            return s_colourBlindMode;
        }

        void SettingsState::SetColourBlindMode(bool enabled) {
            s_colourBlindMode = enabled;
            Core::ColorFilter::SetColourBlindMode(enabled);
        }

    }
}

