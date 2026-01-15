/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LevelTransitionState - Handles level transitions after boss defeat
*/

#pragma once

#include "GameStateMachine.hpp"
#include <string>

namespace RType {
    namespace Client {

        class LevelTransitionState : public IState {
        public:
            LevelTransitionState(GameStateMachine& machine,
                                GameContext& context,
                                int completedLevel,
                                uint32_t playerScore,
                                const std::string& nextLevelPath);

            void Init() override;
            void Cleanup() override;
            void HandleInput() override;
            void Update(float dt) override;
            void Draw() override;

        private:
            enum class TransitionPhase {
                FADE_OUT,
                SHOW_STATS,
                LOADING,
                FADE_IN
            };

            GameStateMachine& m_machine;
            GameContext& m_context;

            TransitionPhase m_phase = TransitionPhase::FADE_OUT;
            float m_phaseTimer = 0.0f;

            int m_completedLevel;
            uint32_t m_playerScore;
            std::string m_nextLevelPath;

            float m_fadeAlpha = 0.0f;

            Renderer::FontId m_fontLarge = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontMedium = Renderer::INVALID_FONT_ID;

            void updateFadeOut(float dt);
            void updateShowStats(float dt);
            void updateLoading(float dt);
            void updateFadeIn(float dt);

            void renderFadeOverlay();
            void renderStats();
            void renderLoadingScreen();
        };

    }
}
