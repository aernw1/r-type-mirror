/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorState
*/

#include "EditorState.hpp"
#include "editor/EditorCanvasManager.hpp"
#include "editor/EditorUIManager.hpp"
#include "editor/EditorEntityManager.hpp"
#include "ECS/Component.hpp"
#include "ECS/Components/TextLabel.hpp"
#include "Core/Logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

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

            m_statusBarEntity = m_registry.CreateEntity();
            m_entities.push_back(m_statusBarEntity);
            m_registry.AddComponent(m_statusBarEntity, ECS::Position{10.0f, 10.0f});

            ECS::TextLabel statusLabel;
            statusLabel.text = "camera: (0, 0)\nzoom: 1.0x\nmouse: (0, 0)\n";
            statusLabel.fontId = m_fontSmall;
            statusLabel.characterSize = 10;
            statusLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
            m_registry.AddComponent(m_statusBarEntity, std::move(statusLabel));

            m_canvasManager = std::make_unique<EditorCanvasManager>(m_renderer.get());
            m_entityManager = std::make_unique<EditorEntityManager>(m_renderer.get());
            m_uiManager = std::make_unique<EditorUIManager>(m_renderer.get(), m_registry, m_entities, m_fontSmall, m_fontMedium);
            m_uiManager->InitializePalette();
            m_selection = m_uiManager->GetActiveSelection();

            Core::Logger::Info("[EditorState] Level editor initialized");
        }

        void EditorState::Cleanup() {
            Core::Logger::Info("[EditorState] Cleaning up level editor...");

            Renderer::Camera2D defaultCam{{0.0f, 0.0f}, {1280.0f, 720.0f}};
            m_renderer->SetCamera(defaultCam);
            m_renderer->ResetCamera();

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
            if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escapeKeyPressed) {
                m_escapeKeyPressed = true;

                if (m_hasUnsavedChanges) {
                    Core::Logger::Warning("[EditorState] Exiting with unsaved changes");
                }

                std::cout << "[EditorState] Returning to menu..." << std::endl;
                m_renderer->ResetCamera();
                m_machine.PopState();
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                m_escapeKeyPressed = false;
            }

            if (m_canvasManager) {
                m_canvasManager->HandleCameraInput();
            }

            Math::Vector2 mouseScreen = m_renderer->GetMousePosition();
            if (m_uiManager) {
                m_uiManager->UpdateHover(mouseScreen);
            }

            bool isLeftPressed = m_renderer->IsMouseButtonPressed(Renderer::IRenderer::MouseButton::Left);
            if (isLeftPressed && !m_leftMousePressed) {
                bool consumedByUI = false;

                if (m_uiManager) {
                    auto selection = m_uiManager->HandleClick(mouseScreen);
                    if (selection.has_value()) {
                        m_selection = selection.value();
                        consumedByUI = true;
                    }
                }

                if (!consumedByUI && m_canvasManager && m_entityManager) {
                    if (m_selection.mode != EditorMode::SELECT) {
                        Math::Vector2 mouseWorld = m_canvasManager->ScreenToWorld(mouseScreen);
                        m_entityManager->PlaceEntity(m_selection.entityType, m_selection.subtype, mouseWorld);
                        m_hasUnsavedChanges = true;

                        m_selection.mode = EditorMode::SELECT;
                        m_selection.subtype.clear();
                        if (m_uiManager) {
                            m_uiManager->SetActiveSelection(m_selection);
                        }
                    }
                }
            }
            m_leftMousePressed = isLeftPressed;
        }

        void EditorState::Update(float dt) {
            (void)dt;

            if (m_canvasManager && m_registry.IsEntityAlive(m_statusBarEntity)) {
                const auto& camera = m_canvasManager->GetCamera();
                Math::Vector2 mouseScreen = m_renderer->GetMousePosition();
                Math::Vector2 mouseWorld = m_canvasManager->ScreenToWorld(mouseScreen);
                m_lastMouseWorld = mouseWorld;

                std::ostringstream oss;
                oss << std::fixed << std::setprecision(1);
                oss << "camera: (" << camera.x << ", " << camera.y << ")\n";
                oss << "zoom: " << camera.zoom << "x\n";
                oss << "mouse: (" << mouseWorld.x << ", " << mouseWorld.y << ")\n";

                auto& statusLabel = m_registry.GetComponent<ECS::TextLabel>(m_statusBarEntity);
                statusLabel.text = oss.str();
            }
        }

        void EditorState::Draw() {
            m_renderer->Clear({0.1f, 0.1f, 0.15f, 1.0f});

            if (m_canvasManager) {
                m_canvasManager->ApplyCamera();
                m_canvasManager->DrawGrid();
                if (m_entityManager) {
                    m_entityManager->DrawEntities();
                    m_entityManager->DrawPlacementPreview(m_selection.mode,
                                                          m_selection.entityType,
                                                          m_selection.subtype,
                                                          m_lastMouseWorld);
                }

                m_renderer->ResetCamera();
            }

            m_renderingSystem->Update(m_registry, 0.0f);
            m_textSystem->Update(m_registry, 0.0f);
        }

    }
}
