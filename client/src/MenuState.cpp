/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MenuState
*/

#include "MenuState.hpp"
#include "LobbyState.hpp"
#include "EditorState.hpp"
#include "editor/EditorCanvasManager.hpp"
#include "RoomListState.hpp"
#include "ECS/Components/TextLabel.hpp"
#include "ECS/Component.hpp"
#include "ECS/AudioSystem.hpp"
#include <iostream>
#include <cmath>

using namespace RType::ECS;

namespace RType {
    namespace Client {

        MenuState::MenuState(GameStateMachine& machine, GameContext& context) : m_machine(machine), m_context(context) {

            m_renderer = context.renderer;
            m_renderingSystem = std::make_unique<RenderingSystem>(m_renderer.get());
            m_textSystem = std::make_unique<TextRenderingSystem>(m_renderer.get());
        }

        void MenuState::Init() {
            std::cout << "[MenuState] Initializing modern UI..." << std::endl;

            if (m_context.audio) {
                m_audioSystem = std::make_unique<RType::ECS::AudioSystem>(m_context.audio.get());
                m_menuMusic = m_context.audio->LoadMusic("assets/sounds/menu.flac");
                if (m_menuMusic == Audio::INVALID_MUSIC_ID) {
                    m_menuMusic = m_context.audio->LoadMusic("../assets/sounds/menu.flac");
                }
                m_menuMusicPlaying = false;

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

            createUI();
        }

        void MenuState::Cleanup() {
            std::cout << "[MenuState] Cleaning up..." << std::endl;

            if (m_context.audio && m_menuMusic != Audio::INVALID_MUSIC_ID) {
                m_context.audio->StopMusic(m_menuMusic);
                m_context.audio->UnloadMusic(m_menuMusic);
                m_menuMusic = Audio::INVALID_MUSIC_ID;
                m_menuMusicPlaying = false;
            }

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

        void MenuState::createUI() {
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
            m_registry.AddComponent(m_titleEntity, Position{640.0f, 200.0f});
            TextLabel titleLabel("R-TYPE", m_fontLarge, 72);
            titleLabel.color = {0.0f, 1.0f, 1.0f, 1.0f};
            titleLabel.centered = true;
            m_registry.AddComponent(m_titleEntity, std::move(titleLabel));

            m_subtitleEntity = m_registry.CreateEntity();
            m_entities.push_back(m_subtitleEntity);
            m_registry.AddComponent(m_subtitleEntity, Position{640.0f, 280.0f});
            TextLabel subtitleLabel("EPIC SPACE SHOOTER", m_fontSmall, 16);
            subtitleLabel.color = {0.5f, 0.86f, 1.0f, 0.95f};
            subtitleLabel.centered = true;
            m_registry.AddComponent(m_subtitleEntity, std::move(subtitleLabel));

            const char* menuLabels[] = {"PLAY", "LEVEL EDITOR", "SETTINGS", "QUIT"};
            float startY = 360.0f;
            float itemSpacing = 60.0f;

            for (int i = 0; i < 4; i++) {
                Entity menuItem = m_registry.CreateEntity();
                m_entities.push_back(menuItem);
                m_menuItems.push_back(menuItem);

                m_registry.AddComponent(menuItem, Position{640.0f, startY + i * itemSpacing});
                TextLabel label(menuLabels[i], m_fontMedium, 24);
                label.color = {0.7f, 0.7f, 0.7f, 1.0f};
                label.centered = true;
                m_registry.AddComponent(menuItem, std::move(label));
            }

            Entity controlsText = m_registry.CreateEntity();
            m_entities.push_back(controlsText);
            m_registry.AddComponent(controlsText, Position{640.0f, 620.0f});
            TextLabel controlsLabel("USE ARROWS TO NAVIGATE  |  ENTER TO SELECT", m_fontSmall, 12);
            controlsLabel.color = {0.5f, 0.86f, 1.0f, 0.85f};
            controlsLabel.centered = true;
            m_registry.AddComponent(controlsText, std::move(controlsLabel));

            Entity versionText = m_registry.CreateEntity();
            m_entities.push_back(versionText);
            m_registry.AddComponent(versionText, Position{20.0f, 680.0f});
            TextLabel versionLabel("v1.0.0 ALPHA", m_fontSmall, 12);
            versionLabel.color = {0.42f, 0.18f, 0.48f, 0.8f};
            m_registry.AddComponent(versionText, std::move(versionLabel));
        }

        void MenuState::updateAnimations(float dt) {
            m_animTime += dt;

            m_titlePulse = std::sin(m_animTime * 2.0f) * 0.3f + 1.0f;

            if (m_titleEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_titleEntity)) {
                auto& titleLabel = m_registry.GetComponent<TextLabel>(m_titleEntity);
                titleLabel.color.a = 0.7f + (m_titlePulse - 1.0f);
            }
        }

        void MenuState::updateMenuSelection() {
            for (size_t i = 0; i < m_menuItems.size(); i++) {
                if (m_registry.IsEntityAlive(m_menuItems[i])) {
                    auto& label = m_registry.GetComponent<TextLabel>(m_menuItems[i]);

                    if (static_cast<int>(i) == m_selectedIndex) {
                        float pulse = std::sin(m_animTime * 4.0f) * 0.3f + 0.7f;
                        label.color = {1.0f, 0.08f + pulse * 0.5f, 0.58f, 1.0f};
                    } else {
                        label.color = {0.5f, 0.5f, 0.5f, 0.8f};
                    }
                }
            }
        }

        void MenuState::HandleInput() {
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
                    m_selectedIndex = static_cast<int>(MenuItem::COUNT) - 1;
                }
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Up)) {
                m_upKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Down) && !m_downKeyPressed) {
                m_downKeyPressed = true;
                playSelectSound();
                m_selectedIndex++;
                if (m_selectedIndex >= static_cast<int>(MenuItem::COUNT)) {
                    m_selectedIndex = 0;
                }
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Down)) {
                m_downKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Enter) && !m_enterKeyPressed) {
                m_enterKeyPressed = true;
                playSelectSound();

                if (m_context.audio && m_menuMusic != Audio::INVALID_MUSIC_ID) {
                    m_context.audio->StopMusic(m_menuMusic);
                    m_menuMusicPlaying = false;
                }

                switch (static_cast<MenuItem>(m_selectedIndex)) {
                    case MenuItem::PLAY:
                        std::cout << "[MenuState] Starting game... Transitioning to Room Selection" << std::endl;
                        std::cout << "[MenuState] Connecting to " << m_context.serverIp << ":" << m_context.serverPort << " as '" << m_context.playerName << "'" << std::endl;
                        m_machine.PushState(std::make_unique<RoomListState>(m_machine, m_context));
                        break;

                    case MenuItem::EDITOR:
                        std::cout << "[MenuState] Opening Level Editor..." << std::endl;
                        m_machine.PushState(std::make_unique<EditorState>(m_machine, m_context));
                        break;

                    case MenuItem::SETTINGS:
                        std::cout << "[MenuState] Settings not yet implemented" << std::endl;
                        // TODO: Implement settings state
                        break;

                    case MenuItem::QUIT:
                        std::cout << "[MenuState] Quitting..." << std::endl;
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
                 std::cout << "[MenuState] Escape pressed. Quitting..." << std::endl;
                 m_machine.PopState();
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                m_escKeyPressed = false;
            }
        }

        void MenuState::Update(float dt) {
            updateAnimations(dt);
            updateMenuSelection();

            if (m_audioSystem && m_menuMusic != Audio::INVALID_MUSIC_ID && !m_menuMusicPlaying) {
                auto cmd = m_registry.CreateEntity();
                auto& me = m_registry.AddComponent<MusicEffect>(cmd, MusicEffect(m_menuMusic));
                me.play = true;
                me.stop = false;
                me.loop = true;
                me.volume = 0.35f;
                me.pitch = 1.0f;
                m_menuMusicPlaying = true;
            }

            if (m_audioSystem) {
                m_audioSystem->Update(m_registry, dt);
            }
        }

        void MenuState::Draw() {
            m_renderer->Clear({0.05f, 0.05f, 0.1f, 1.0f});

            m_renderingSystem->Update(m_registry, 0.0f);
            m_textSystem->Update(m_registry, 0.0f);
        }

    }
}
