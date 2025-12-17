/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ResultsState
*/

#include "ResultsState.hpp"
#include "LobbyState.hpp"
#include "ECS/Components/TextLabel.hpp"

#include <algorithm>
#include <iostream>

using namespace RType::ECS;

namespace RType {
    namespace Client {

        ResultsState::ResultsState(GameStateMachine& machine,
                                   GameContext& context,
                                   std::vector<std::pair<std::string, uint32_t>> scores)
            : m_machine(machine), m_context(context), m_scores(std::move(scores)) {
            m_renderer = context.renderer;
            m_renderingSystem = std::make_unique<RenderingSystem>(m_renderer.get());
            m_textSystem = std::make_unique<TextRenderingSystem>(m_renderer.get());
        }

        void ResultsState::Init() {
            m_fontLarge = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 36);
            if (m_fontLarge == Renderer::INVALID_FONT_ID) {
                m_fontLarge = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 36);
            }

            m_fontMedium = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 18);
            if (m_fontMedium == Renderer::INVALID_FONT_ID) {
                m_fontMedium = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 18);
            }

            m_fontSmall = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 14);
            if (m_fontSmall == Renderer::INVALID_FONT_ID) {
                m_fontSmall = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 14);
            }

            m_bgTexture = m_renderer->LoadTexture("assets/backgrounds/game over/preview.png");
            if (m_bgTexture == Renderer::INVALID_TEXTURE_ID) {
                m_bgTexture = m_renderer->LoadTexture("../assets/backgrounds/game over/preview.png");
            }
            if (m_bgTexture != Renderer::INVALID_TEXTURE_ID) {
                m_bgSprite = m_renderer->CreateSprite(m_bgTexture, {});
                m_bgTextureSize = m_renderer->GetTextureSize(m_bgTexture);
            } else {
                m_bgTexture = m_renderer->LoadTexture("assets/backgrounds/1.jpg");
                if (m_bgTexture == Renderer::INVALID_TEXTURE_ID) {
                    m_bgTexture = m_renderer->LoadTexture("../assets/backgrounds/1.jpg");
                }
                if (m_bgTexture != Renderer::INVALID_TEXTURE_ID) {
                    m_bgSprite = m_renderer->CreateSprite(m_bgTexture, {});
                    m_bgTextureSize = m_renderer->GetTextureSize(m_bgTexture);
                }
            }

            
            m_enterPressed = m_renderer->IsKeyPressed(Renderer::Key::Enter);
            m_escapePressed = m_renderer->IsKeyPressed(Renderer::Key::Escape);

            std::sort(m_scores.begin(), m_scores.end(),
                      [](const auto& a, const auto& b) { return a.second > b.second; });

            createUI();
        }

        void ResultsState::Cleanup() {
            auto entities = m_registry.GetEntitiesWithComponent<Position>();
            for (auto e : entities) {
                if (m_registry.IsEntityAlive(e)) {
                    m_registry.DestroyEntity(e);
                }
            }
        }

        void ResultsState::HandleInput() {
            if (m_renderer->IsKeyPressed(Renderer::Key::Enter) && !m_enterPressed) {
                m_enterPressed = true;
                m_machine.ChangeState(std::make_unique<LobbyState>(m_machine, m_context));
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Enter)) {
                m_enterPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escapePressed) {
                m_escapePressed = true;
                m_machine.ChangeState(std::make_unique<LobbyState>(m_machine, m_context));
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                m_escapePressed = false;
            }
        }

        void ResultsState::Update(float dt) {
            (void)dt;
        }

        void ResultsState::Draw() {
            m_renderer->Clear({0.05f, 0.05f, 0.12f, 1.0f});
            m_renderingSystem->Update(m_registry, 0.0f);
            m_textSystem->Update(m_registry, 0.0f);
        }

        void ResultsState::createUI() {
            if (m_bgSprite != Renderer::INVALID_SPRITE_ID) {
                m_bgEntity = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(m_bgEntity, Position{0.0f, 0.0f});

                Drawable drawable(m_bgSprite, -10);

                if (m_bgTextureSize.x > 0 && m_bgTextureSize.y > 0) {
                    drawable.scale = {1280.0f / m_bgTextureSize.x, 720.0f / m_bgTextureSize.y};
                }

                m_registry.AddComponent<Drawable>(m_bgEntity, std::move(drawable));
            }

            if (m_fontLarge == Renderer::INVALID_FONT_ID) {
                return;
            }

            Entity titleEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(titleEntity, Position{640.0f, 90.0f});
            TextLabel titleLabel("RESULTS", m_fontLarge, 40);
            titleLabel.centered = true;
            titleLabel.color = {1.0f, 0.08f, 0.58f, 1.0f};
            m_registry.AddComponent<TextLabel>(titleEntity, std::move(titleLabel));

            Entity subtitleEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(subtitleEntity, Position{640.0f, 140.0f});
            TextLabel subtitle("Final scores", m_fontSmall != Renderer::INVALID_FONT_ID ? m_fontSmall : m_fontMedium, 16);
            subtitle.centered = true;
            subtitle.color = {0.5f, 0.86f, 1.0f, 0.9f};
            m_registry.AddComponent<TextLabel>(subtitleEntity, std::move(subtitle));

            float startY = 220.0f;
            float stepY = 38.0f;
            size_t maxLines = std::min<size_t>(m_scores.size(), 8);

            for (size_t i = 0; i < maxLines; i++) {
                const auto& [name, score] = m_scores[i];
                Entity line = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(line, Position{640.0f, startY + static_cast<float>(i) * stepY});

                std::string text = std::to_string(i + 1) + ". " + name + "  " + std::to_string(score);
                TextLabel label(text, m_fontMedium != Renderer::INVALID_FONT_ID ? m_fontMedium : m_fontLarge, 20);
                label.centered = true;
                label.color = {0.9f, 0.95f, 1.0f, 1.0f};
                m_registry.AddComponent<TextLabel>(line, std::move(label));
            }

            Entity hint = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(hint, Position{640.0f, 650.0f});
            TextLabel hintLabel("Press ENTER to return to lobby", m_fontSmall != Renderer::INVALID_FONT_ID ? m_fontSmall : m_fontMedium, 14);
            hintLabel.centered = true;
            hintLabel.color = {0.5f, 0.86f, 1.0f, 0.85f};
            m_registry.AddComponent<TextLabel>(hint, std::move(hintLabel));
        }

    }
}


