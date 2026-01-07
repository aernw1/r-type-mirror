/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorState
*/

#pragma once

#include "GameStateMachine.hpp"
#include "editor/EditorTypes.hpp"
#include "ECS/Registry.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/TextRenderingSystem.hpp"
#include "Renderer/IRenderer.hpp"
#include <memory>
#include <string>
#include <vector>

namespace RType {
    namespace Client {

        class EditorCanvasManager;
        class EditorUIManager;
        class EditorEntityManager;
        class EditorFileManager;

        class EditorState : public IState {
        public:
            EditorState(GameStateMachine& machine, GameContext& context);
            ~EditorState() override = default;

            void Init() override;
            void Cleanup() override;
            void HandleInput() override;
            void Update(float dt) override;
            void Draw() override;

        private:
            GameStateMachine& m_machine;
            GameContext& m_context;

            RType::ECS::Registry m_registry;
            std::shared_ptr<Renderer::IRenderer> m_renderer;
            std::unique_ptr<RType::ECS::RenderingSystem> m_renderingSystem;
            std::unique_ptr<RType::ECS::TextRenderingSystem> m_textSystem;

            // Editor subsystems (to be implemented in later phases)
            // std::unique_ptr<EditorCanvasManager> m_canvasManager;
            // std::unique_ptr<EditorUIManager> m_uiManager;
            // std::unique_ptr<EditorEntityManager> m_entityManager;
            // std::unique_ptr<EditorFileManager> m_fileManager;

            // Fonts
            Renderer::FontId m_fontSmall = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontMedium = Renderer::INVALID_FONT_ID;

            // UI entities for cleanup
            std::vector<RType::ECS::Entity> m_entities;

            // Input state
            bool m_escapeKeyPressed = false;

            // Editor state
            EditorMode m_mode = EditorMode::SELECT;
            std::string m_currentLevelPath;
            bool m_hasUnsavedChanges = false;
        };

    }
}
