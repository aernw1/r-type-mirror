/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ResultsState
*/

#include "ResultsState.hpp"
#include "MenuState.hpp"
#include "ECS/Components/TextLabel.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

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
            if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escapePressed) {
                m_escapePressed = true;
                m_machine.ChangeState(std::make_unique<MenuState>(m_machine, m_context));
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

            if (m_drawScorePanel) {
                Renderer::Rectangle border = m_scorePanelRect;
                border.position.x -= 4.0f;
                border.position.y -= 4.0f;
                border.size.x += 8.0f;
                border.size.y += 8.0f;
                m_renderer->DrawRectangle(border, Renderer::Color(0.0f, 0.0f, 0.0f, 0.90f));
                m_renderer->DrawRectangle(m_scorePanelRect, Renderer::Color(0.0f, 0.0f, 0.0f, 0.62f));
                Renderer::Rectangle headerRect = m_scorePanelRect;
                headerRect.size.y = m_scorePanelHeaderHeight;
                m_renderer->DrawRectangle(headerRect, Renderer::Color(0.0f, 0.0f, 0.0f, 0.72f));
                Renderer::Rectangle sep = m_scorePanelRect;
                sep.position.y += m_scorePanelHeaderHeight;
                sep.size.y = 2.0f;
                m_renderer->DrawRectangle(sep, Renderer::Color(0.20f, 0.60f, 1.00f, 0.45f));

                for (size_t i = 0; i < m_scorePanelRowCount; ++i) {
                    Renderer::Rectangle rowRect;
                    rowRect.position = Renderer::Vector2(m_scorePanelRect.position.x + 12.0f,
                                                         m_scorePanelRowStartY + static_cast<float>(i) * m_scorePanelRowStepY - 16.0f);
                    rowRect.size = Renderer::Vector2(m_scorePanelRect.size.x - 24.0f, m_scorePanelRowStepY);
                    float a = (i % 2 == 0) ? 0.10f : 0.06f;
                    m_renderer->DrawRectangle(rowRect, Renderer::Color(0.02f, 0.08f, 0.18f, a));
                }

                Renderer::Rectangle hintStrip;
                hintStrip.position = Renderer::Vector2(0.0f, 630.0f);
                hintStrip.size = Renderer::Vector2(1280.0f, 60.0f);
                m_renderer->DrawRectangle(hintStrip, Renderer::Color(0.0f, 0.0f, 0.0f, 0.30f));
            }

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

            const float screenW = 1280.0f;
            const float panelW = 920.0f;
            const float panelH = 430.0f;
            const float panelX = (screenW - panelW) * 0.5f;
            const float panelY = 185.0f;

            m_drawScorePanel = true;
            m_scorePanelRect.position = Renderer::Vector2(panelX, panelY);
            m_scorePanelRect.size = Renderer::Vector2(panelW, panelH);
            m_scorePanelRowStepY = 36.0f;
            m_scorePanelRowStartY = panelY + m_scorePanelHeaderHeight + 46.0f;

            m_colRankX = panelX + 70.0f;
            m_colNameX = panelX + 170.0f;
            m_colScoreX = panelX + panelW - 130.0f;

            Entity titleEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(titleEntity, Position{640.0f, 90.0f});
            TextLabel titleLabel("RESULTS", m_fontLarge, 40);
            titleLabel.centered = true;
            titleLabel.color = {0.10f, 0.45f, 1.00f, 1.0f};
            m_registry.AddComponent<TextLabel>(titleEntity, std::move(titleLabel));

            Entity subtitleEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(subtitleEntity, Position{640.0f, 140.0f});
            TextLabel subtitle("Final scores", m_fontSmall != Renderer::INVALID_FONT_ID ? m_fontSmall : m_fontMedium, 16);
            subtitle.centered = true;
            subtitle.color = {0.20f, 0.60f, 1.00f, 0.95f};
            m_registry.AddComponent<TextLabel>(subtitleEntity, std::move(subtitle));

            // Table header
            {
                Entity headerRank = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(headerRank, Position{m_colRankX, panelY + 26.0f});
                TextLabel hRank("RANK", m_fontSmall != Renderer::INVALID_FONT_ID ? m_fontSmall : m_fontMedium, 14);
                hRank.centered = true;
                hRank.color = {0.75f, 0.88f, 1.00f, 0.95f};
                m_registry.AddComponent<TextLabel>(headerRank, std::move(hRank));

                Entity headerName = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(headerName, Position{m_colNameX, panelY + 26.0f});
                TextLabel hName("NAME", m_fontSmall != Renderer::INVALID_FONT_ID ? m_fontSmall : m_fontMedium, 14);
                hName.centered = false;
                hName.color = {0.75f, 0.88f, 1.00f, 0.95f};
                m_registry.AddComponent<TextLabel>(headerName, std::move(hName));

                Entity headerScore = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(headerScore, Position{m_colScoreX, panelY + 26.0f});
                TextLabel hScore("SCORE", m_fontSmall != Renderer::INVALID_FONT_ID ? m_fontSmall : m_fontMedium, 14);
                hScore.centered = true;
                hScore.color = {0.75f, 0.88f, 1.00f, 0.95f};
                m_registry.AddComponent<TextLabel>(headerScore, std::move(hScore));
            }

            size_t maxLines = std::min<size_t>(m_scores.size(), 8);
            m_scorePanelRowCount = maxLines;

            for (size_t i = 0; i < maxLines; i++) {
                const auto& [name, score] = m_scores[i];

                const float y = m_scorePanelRowStartY + static_cast<float>(i) * m_scorePanelRowStepY;

                Entity rankEnt = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(rankEnt, Position{m_colRankX, y});
                TextLabel rankLabel(std::to_string(i + 1), m_fontMedium != Renderer::INVALID_FONT_ID ? m_fontMedium : m_fontLarge, 18);
                rankLabel.centered = true;
                rankLabel.color = {0.75f, 0.88f, 1.00f, 1.0f};
                m_registry.AddComponent<TextLabel>(rankEnt, std::move(rankLabel));

                Entity nameEnt = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(nameEnt, Position{m_colNameX, y});
                std::string displayName = name;
                if (displayName.size() > 18) {
                    displayName = displayName.substr(0, 18);
                }
                TextLabel nameLabel(displayName, m_fontMedium != Renderer::INVALID_FONT_ID ? m_fontMedium : m_fontLarge, 18);
                nameLabel.centered = false;
                nameLabel.color = {0.90f, 0.95f, 1.00f, 1.0f};
                m_registry.AddComponent<TextLabel>(nameEnt, std::move(nameLabel));

                Entity scoreEnt = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(scoreEnt, Position{m_colScoreX, y});
                std::ostringstream ss;
                ss << std::setw(8) << std::setfill('0') << score;
                TextLabel scoreLabel(ss.str(), m_fontMedium != Renderer::INVALID_FONT_ID ? m_fontMedium : m_fontLarge, 18);
                scoreLabel.centered = true;
                scoreLabel.color = {0.75f, 0.88f, 1.00f, 1.0f};
                m_registry.AddComponent<TextLabel>(scoreEnt, std::move(scoreLabel));
            }

            Entity hint = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(hint, Position{640.0f, 650.0f});
            TextLabel hintLabel("Press ESC to return to menu", m_fontSmall != Renderer::INVALID_FONT_ID ? m_fontSmall : m_fontMedium, 14);
            hintLabel.centered = true;
            hintLabel.color = {0.20f, 0.60f, 1.00f, 0.95f};
            m_registry.AddComponent<TextLabel>(hint, std::move(hintLabel));
        }

    }
}


