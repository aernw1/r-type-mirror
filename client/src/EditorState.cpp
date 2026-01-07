/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorState
*/

#include "EditorState.hpp"
#include "ECS/Component.hpp"
#include "ECS/Components/TextLabel.hpp"
#include "Core/Logger.hpp"
#include <iostream>

namespace RType {
    namespace Client {

        EditorState::EditorState(GameStateMachine& machine, GameContext& context)
            : m_machine(machine)
            , m_context(context)
            , m_renderer(context.renderer)
        {
        }

        void EditorState::Init() {
            Core::Logger::Info("[EditorState] Initializing level editor...");

            // Initialize rendering systems
            m_renderingSystem = std::make_unique<ECS::RenderingSystem>(m_renderer.get());
            m_textSystem = std::make_unique<ECS::TextRenderingSystem>(m_renderer.get());

            // Load fonts
            m_fontSmall = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 12);
            if (m_fontSmall == Renderer::INVALID_FONT_ID) {
                m_fontSmall = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 12);
            }

            m_fontMedium = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 16);
            if (m_fontMedium == Renderer::INVALID_FONT_ID) {
                m_fontMedium = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 16);
            }

            // Create title text
            ECS::Entity titleEntity = m_registry.CreateEntity();
            m_entities.push_back(titleEntity);
            m_registry.AddComponent(titleEntity, ECS::Position{640.0f, 50.0f});

            ECS::TextLabel titleLabel;
            titleLabel.text = "LEVEL EDITOR";
            titleLabel.fontId = m_fontMedium;
            titleLabel.characterSize = 24;
            titleLabel.color = {0.5f, 1.0f, 0.5f, 1.0f};  // Green
            titleLabel.centered = true;
            m_registry.AddComponent(titleEntity, std::move(titleLabel));

            // Create instructions text
            ECS::Entity instructionsEntity = m_registry.CreateEntity();
            m_entities.push_back(instructionsEntity);
            m_registry.AddComponent(instructionsEntity, ECS::Position{640.0f, 360.0f});

            ECS::TextLabel instructionsLabel;
            instructionsLabel.text = "Press ESC to return to menu";
            instructionsLabel.fontId = m_fontSmall;
            instructionsLabel.characterSize = 12;
            instructionsLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};  // Gray
            instructionsLabel.centered = true;
            m_registry.AddComponent(instructionsEntity, std::move(instructionsLabel));

            Core::Logger::Info("[EditorState] Level editor initialized");

            // TODO Phase 2: Initialize canvas manager
            // TODO Phase 3: Initialize UI manager and entity manager
            // TODO Phase 6: Initialize file manager
        }

        void EditorState::Cleanup() {
            Core::Logger::Info("[EditorState] Cleaning up level editor...");

            // Destroy all UI entities
            for (ECS::Entity entity : m_entities) {
                if (m_registry.IsEntityAlive(entity)) {
                    m_registry.DestroyEntity(entity);
                }
            }
            m_entities.clear();

            Core::Logger::Info("[EditorState] Level editor cleaned up");
        }

        void EditorState::HandleInput() {
            // Handle escape key to return to menu
            if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escapeKeyPressed) {
                m_escapeKeyPressed = true;

                if (m_hasUnsavedChanges) {
                    // TODO: Show warning modal
                    Core::Logger::Warning("[EditorState] Exiting with unsaved changes");
                }

                std::cout << "[EditorState] Returning to menu..." << std::endl;
                m_machine.PopState();
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                m_escapeKeyPressed = false;
            }

            // TODO Phase 2: Handle camera input
            // TODO Phase 3: Handle entity placement input
            // TODO Phase 4: Handle selection and property editing input
        }

        void EditorState::Update(float dt) {
            // TODO Phase 2: Update canvas manager
            // TODO Phase 3: Update entity manager (preview entity)
            // TODO Phase 4: Update property editing

            (void)dt;  // Unused for now
        }

        void EditorState::Draw() {
            // Clear screen with dark background
            m_renderer->Clear({0.1f, 0.1f, 0.15f, 1.0f});

            // TODO Phase 2: Apply camera transform and draw grid
            // TODO Phase 3: Draw placed entities
            // TODO Phase 4: Draw selection highlights

            // Render UI
            m_renderingSystem->Update(m_registry, 0.0f);
            m_textSystem->Update(m_registry, 0.0f);
        }

    }
}
