/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LevelTransitionState implementation
*/

#include "../../include/LevelTransitionState.hpp"
#include "../../include/GameState.hpp"
#include "Core/Logger.hpp"
#include <algorithm>
#include <ctime>

namespace RType {
    namespace Client {

        LevelTransitionState::LevelTransitionState(
            GameStateMachine& machine,
            GameContext& context,
            int completedLevel,
            uint32_t playerScore,
            const std::string& nextLevelPath)
            : m_machine(machine)
            , m_context(context)
            , m_completedLevel(completedLevel)
            , m_playerScore(playerScore)
            , m_nextLevelPath(nextLevelPath)
        {
        }

        void LevelTransitionState::Init() {
            Core::Logger::Info("[LevelTransition] Initializing transition from level {}",
                              m_completedLevel);
            m_phase = TransitionPhase::FADE_OUT;
            m_phaseTimer = 0.0f;
            m_fadeAlpha = 0.0f;

            // Load fonts for text rendering
            if (m_context.renderer) {
                m_fontLarge = m_context.renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 36);
                if (m_fontLarge == Renderer::INVALID_FONT_ID) {
                    m_fontLarge = m_context.renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 36);
                }

                m_fontMedium = m_context.renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 20);
                if (m_fontMedium == Renderer::INVALID_FONT_ID) {
                    m_fontMedium = m_context.renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 20);
                }
            }
        }

        void LevelTransitionState::Cleanup() {
            Core::Logger::Info("[LevelTransition] Cleaning up transition state");

            if (m_context.renderer) {
                if (m_fontLarge != Renderer::INVALID_FONT_ID) {
                    m_context.renderer->UnloadFont(m_fontLarge);
                }
                if (m_fontMedium != Renderer::INVALID_FONT_ID) {
                    m_context.renderer->UnloadFont(m_fontMedium);
                }
            }
        }

        void LevelTransitionState::HandleInput() {
            // No input during transition (optional: ESC to skip)
        }

        void LevelTransitionState::Update(float dt) {
            m_phaseTimer += dt;

            switch (m_phase) {
                case TransitionPhase::FADE_OUT:
                    updateFadeOut(dt);
                    break;
                case TransitionPhase::SHOW_STATS:
                    updateShowStats(dt);
                    break;
                case TransitionPhase::LOADING:
                    updateLoading(dt);
                    break;
                case TransitionPhase::FADE_IN:
                    updateFadeIn(dt);
                    break;
            }
        }

        void LevelTransitionState::updateFadeOut(float dt) {
            const float FADE_DURATION = 1.0f;
            m_fadeAlpha = std::min(1.0f, m_phaseTimer / FADE_DURATION);

            if (m_phaseTimer >= FADE_DURATION) {
                Core::Logger::Debug("[LevelTransition] Fade out complete, showing stats");
                m_phase = TransitionPhase::SHOW_STATS;
                m_phaseTimer = 0.0f;
                m_fadeAlpha = 0.0f;
            }
        }

        void LevelTransitionState::updateShowStats(float dt) {
            const float STATS_DURATION = 3.0f;

            if (m_phaseTimer >= STATS_DURATION) {
                Core::Logger::Debug("[LevelTransition] Stats display complete, loading next level");
                m_phase = TransitionPhase::LOADING;
                m_phaseTimer = 0.0f;
            }
        }

        void LevelTransitionState::updateLoading(float dt) {
            const float MIN_LOADING_TIME = 4.0f;  // Increased to give server time to reload level

            if (m_phaseTimer >= MIN_LOADING_TIME) {
                if (m_nextLevelPath.empty()) {
                    // All levels complete - TODO: create VictoryState
                    Core::Logger::Info("[LevelTransition] All levels complete! VICTORY!");
                } else {
                    Core::Logger::Info("[LevelTransition] Loading next level: {}", m_nextLevelPath);

                    uint32_t newSeed = static_cast<uint32_t>(std::time(nullptr));

                    m_machine.ChangeState(std::make_unique<InGameState>(
                        m_machine,
                        m_context,
                        newSeed,
                        m_nextLevelPath
                    ));
                }
            }
        }

        void LevelTransitionState::updateFadeIn(float dt) {
            const float FADE_DURATION = 1.0f;
            m_fadeAlpha = std::max(0.0f, 1.0f - (m_phaseTimer / FADE_DURATION));

            if (m_phaseTimer >= FADE_DURATION) {
                Core::Logger::Info("[LevelTransition] Fade in complete, transition finished");
            }
        }

        void LevelTransitionState::Draw() {
            if (!m_context.renderer) {
                return;
            }

            Renderer::Rectangle fullScreen;
            fullScreen.position = Math::Vector2(0.0f, 0.0f);
            fullScreen.size = Math::Vector2(1280.0f, 720.0f);
            m_context.renderer->DrawRectangle(fullScreen,
                Math::Color(0.0f, 0.0f, 0.0f, 1.0f));

            switch (m_phase) {
                case TransitionPhase::SHOW_STATS:
                    renderStats();
                    break;
                case TransitionPhase::LOADING:
                    renderLoadingScreen();
                    break;
                default:
                    break;
            }

            if (m_fadeAlpha > 0.0f) {
                renderFadeOverlay();
            }
        }

        void LevelTransitionState::renderFadeOverlay() {
            Renderer::Rectangle overlay;
            overlay.position = Math::Vector2(0.0f, 0.0f);
            overlay.size = Math::Vector2(1280.0f, 720.0f);
            m_context.renderer->DrawRectangle(overlay,
                Math::Color(0.0f, 0.0f, 0.0f, m_fadeAlpha));
        }

        void LevelTransitionState::renderStats() {
            if (m_fontLarge == Renderer::INVALID_FONT_ID || m_fontMedium == Renderer::INVALID_FONT_ID) {
                return;
            }

            std::string levelText = "LEVEL " + std::to_string(m_completedLevel) + " COMPLETE!";

            Renderer::TextParams titleParams;
            titleParams.position = Math::Vector2(640.0f, 250.0f);
            titleParams.color = Math::Color(0.0f, 1.0f, 0.0f, 1.0f);  // Green
            titleParams.centered = true;
            m_context.renderer->DrawText(m_fontLarge, levelText, titleParams);

            std::string scoreText = "SCORE: " + std::to_string(m_playerScore);
            Renderer::TextParams scoreParams;
            scoreParams.position = Math::Vector2(640.0f, 350.0f);
            scoreParams.color = Math::Color(1.0f, 1.0f, 1.0f, 1.0f);  // White
            scoreParams.centered = true;
            m_context.renderer->DrawText(m_fontMedium, scoreText, scoreParams);

            Core::Logger::Debug("[LevelTransition] Rendering stats - Level: {}, Score: {}",
                               m_completedLevel, m_playerScore);
        }

        void LevelTransitionState::renderLoadingScreen() {
            if (m_fontMedium == Renderer::INVALID_FONT_ID) {
                return;
            }

            float dots = fmodf(m_phaseTimer * 2.0f, 4.0f);
            int numDots = static_cast<int>(dots);

            std::string loadingText = "LOADING";
            for (int i = 0; i < numDots; i++) {
                loadingText += ".";
            }

            Renderer::TextParams loadingParams;
            loadingParams.position = Math::Vector2(640.0f, 320.0f);
            loadingParams.color = Math::Color(1.0f, 1.0f, 0.0f, 1.0f);
            loadingParams.centered = true;
            m_context.renderer->DrawText(m_fontMedium, loadingText, loadingParams);

            Core::Logger::Debug("[LevelTransition] Rendering loading screen");
        }

    }
}
