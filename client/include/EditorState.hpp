/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorState
*/

#pragma once

#include "GameStateMachine.hpp"
#include "editor/EditorTypes.hpp"
#include "editor/EditorUIManager.hpp"
#include "editor/EditorEntityManager.hpp"
#include "editor/EditorAssetLibrary.hpp"
#include "editor/EditorInputHandler.hpp"
#include "editor/EditorPropertyManager.hpp"
#include "ECS/Registry.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/TextRenderingSystem.hpp"
#include "Renderer/IRenderer.hpp"
#include <memory>
#include <string>
#include <vector>
#include <array>

namespace RType {
    namespace Client {

        class EditorCanvasManager;
        class EditorUIManager;
        class EditorEntityManager;

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

            std::unique_ptr<EditorCanvasManager> m_canvasManager;
            std::unique_ptr<EditorAssetLibrary> m_assetLibrary;
            std::unique_ptr<EditorUIManager> m_uiManager;
            std::unique_ptr<EditorEntityManager> m_entityManager;
            std::unique_ptr<EditorInputHandler> m_inputHandler;
            std::unique_ptr<EditorPropertyManager> m_propertyManager;

            // Fonts
            Renderer::FontId m_fontSmall = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_fontMedium = Renderer::INVALID_FONT_ID;

            // UI entities for cleanup
            std::vector<RType::ECS::Entity> m_entities;
            std::vector<RType::ECS::Entity> m_statusBarEntities;

            // Input state
            bool m_leftMousePressed = false;

            // Editor state
            EditorPaletteSelection m_selection;
            Math::Vector2 m_lastMouseWorld{0.0f, 0.0f};
            std::string m_currentLevelPath;
            bool m_hasUnsavedChanges = false;

            void handleSelectionAt(const Math::Vector2& mouseWorld);
            void updatePropertyPanel();
            void deleteSelectedEntity();
        };

    }
}
