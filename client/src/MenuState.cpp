/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MenuState
*/

#include "MenuState.hpp"
#include "LobbyState.hpp"
#include "EditorState.hpp"
#include "ECS/Components/TextLabel.hpp"
#include "ECS/Component.hpp"
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

            m_playTextEntity = m_registry.CreateEntity();
            m_entities.push_back(m_playTextEntity);
            m_registry.AddComponent(m_playTextEntity, Position{640.0f, 380.0f});
            TextLabel playLabel("PRESS ENTER TO START", m_fontMedium, 28);
            playLabel.color = {1.0f, 0.08f, 0.58f, 1.0f};
            playLabel.centered = true;
            m_registry.AddComponent(m_playTextEntity, std::move(playLabel));

            m_editorTextEntity = m_registry.CreateEntity();
            m_entities.push_back(m_editorTextEntity);
            m_registry.AddComponent(m_editorTextEntity, Position{640.0f, 440.0f});
            TextLabel editorLabel("PRESS E FOR LEVEL EDITOR", m_fontSmall, 16);
            editorLabel.color = {0.5f, 1.0f, 0.5f, 0.85f};
            editorLabel.centered = true;
            m_registry.AddComponent(m_editorTextEntity, std::move(editorLabel));

            Entity controlsText = m_registry.CreateEntity();
            m_entities.push_back(controlsText);
            m_registry.AddComponent(controlsText, Position{640.0f, 480.0f});
            TextLabel controlsLabel("CONTROLS: ARROWS = MOVE  |  SPACE = SHOOT", m_fontSmall, 14);
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

            float blinkSpeed = 1.5f;
            float blink = (std::sin(m_animTime * blinkSpeed * 3.14159f) + 1.0f) * 0.5f;

            if (m_playTextEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_playTextEntity)) {
                auto& playLabel = m_registry.GetComponent<TextLabel>(m_playTextEntity);
                playLabel.color.a = 0.5f + blink * 0.5f;
            }
        }

        void MenuState::HandleInput() {
            if (m_renderer->IsKeyPressed(Renderer::Key::Enter) && !m_playKeyPressed) {
                m_playKeyPressed = true;

                std::cout << "[MenuState] Starting game... Transitioning to Lobby" << std::endl;
                std::cout << "[MenuState] Connecting to " << m_context.serverIp << ":" << m_context.serverPort << " as '" << m_context.playerName << "'" << std::endl;

                m_machine.PushState(std::make_unique<LobbyState>(m_machine, m_context));

            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Enter)) {
                m_playKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::E) && !m_eKeyPressed) {
                m_eKeyPressed = true;

                std::cout << "[MenuState] Opening Level Editor..." << std::endl;

                m_machine.PushState(std::make_unique<EditorState>(m_machine, m_context));

            } else if (!m_renderer->IsKeyPressed(Renderer::Key::E)) {
                m_eKeyPressed = false;
            }
        }

        void MenuState::Update(float dt) {
            updateAnimations(dt);
        }

        void MenuState::Draw() {
            m_renderer->Clear({0.05f, 0.05f, 0.1f, 1.0f});

            m_renderingSystem->Update(m_registry, 0.0f);
            m_textSystem->Update(m_registry, 0.0f);
        }

    }
}
