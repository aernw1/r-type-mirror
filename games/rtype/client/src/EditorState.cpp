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
#include "editor/EditorFileManager.hpp"
#include "editor/EditorConstants.hpp"
#include "ECS/Component.hpp"
#include "ECS/Components/TextLabel.hpp"
#include "Core/Logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace RType::Client::EditorConstants;

namespace RType {
    namespace Client {

        EditorState::EditorState(GameStateMachine& machine, GameContext& context)
            : m_machine(machine)
            , m_context(context)
            , m_renderer(context.renderer)
        {
        }

        EditorState::~EditorState() = default;

        void EditorState::Init() {
            Core::Logger::Info("[EditorState] Initializing level editor...");

            m_renderingSystem = std::make_unique<ECS::RenderingSystem>(m_renderer.get());
            m_textSystem = std::make_unique<ECS::TextRenderingSystem>(m_renderer.get());

            m_fontSmall = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 12);
            if (m_fontSmall == Renderer::INVALID_FONT_ID) {
                m_fontSmall = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 12);
            }

            m_fontMedium = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 16);
            if (m_fontMedium == Renderer::INVALID_FONT_ID) {
                m_fontMedium = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 16);
            }

            for (int i = 0; i < 3; ++i) {
                ECS::Entity statusEntity = m_registry.CreateEntity();
                m_entities.push_back(statusEntity);
                m_statusBarEntities.push_back(statusEntity);

                float yPos = UI::STATUS_BAR_Y + (i * UI::STATUS_BAR_LINE_HEIGHT);
                m_registry.AddComponent(statusEntity, ECS::Position{UI::STATUS_BAR_X, yPos});

                ECS::TextLabel statusLabel;
                statusLabel.text = "";
                statusLabel.fontId = m_fontSmall;
                statusLabel.characterSize = 10;
                statusLabel.color = Colors::UI_TEXT;
                m_registry.AddComponent(statusEntity, std::move(statusLabel));
            }

            m_inputHandler = std::make_unique<EditorInputHandler>(m_renderer.get());
            m_propertyManager = std::make_unique<EditorPropertyManager>(m_renderer.get());
            m_canvasManager = std::make_unique<EditorCanvasManager>(m_renderer.get());
            m_assetLibrary = std::make_unique<EditorAssetLibrary>(m_renderer.get());
            m_assetLibrary->Initialize();

            m_fileManager = std::make_unique<EditorFileManager>(*m_assetLibrary);
            m_entityManager = std::make_unique<EditorEntityManager>(m_renderer.get(), m_registry, *m_assetLibrary);
            m_uiManager = std::make_unique<EditorUIManager>(m_renderer.get(), *m_assetLibrary, m_registry, m_entities, m_fontSmall, m_fontMedium);
            m_uiManager->InitializePalette();
            m_uiManager->SetOnSaveRequested([this]() {
                saveCurrentLevel();
            });
            m_selection = m_uiManager->GetActiveSelection();

            m_propertyManager->SetOnPropertyChanged([this](EditorEntityData& entity) {
                m_entityManager->RebuildDefaultCollider(entity);
                m_entityManager->SyncEntity(entity);
                m_hasUnsavedChanges = true;
                updatePropertyPanel();
            });

            m_propertyManager->SetOnEntityDeleted([this]() {
                deleteSelectedEntity();
            });

            m_propertyManager->SetOnPropertyCycled([this]() {
                updatePropertyPanel();
            });

            updatePropertyPanel();

            Core::Logger::Info("[EditorState] Level editor initialized");
        }

        void EditorState::Cleanup() {
            Core::Logger::Info("[EditorState] Cleaning up level editor...");

            Renderer::Camera2D defaultCam{{0.0f, 0.0f}, {1280.0f, 720.0f}};
            m_renderer->SetCamera(defaultCam);
            m_renderer->ResetCamera();

            for (ECS::Entity entity : m_entities) {
                if (m_registry.IsEntityAlive(entity)) {
                    m_registry.DestroyEntity(entity);
                }
            }
            m_entities.clear();

            Core::Logger::Info("[EditorState] Level editor cleaned up");
        }

        void EditorState::HandleInput() {
            m_inputHandler->HandleKeyPress(Renderer::Key::Escape, [this]() {
                if (m_hasUnsavedChanges) {
                    Core::Logger::Warning("[EditorState] Exiting with unsaved changes");
                }
                Core::Logger::Info("[EditorState] Returning to menu...");
                if (m_machine.IsRunning() && m_machine.GetCurrentState() == this) {
                    m_machine.PopState();
                } else {
                    Core::Logger::Error("[EditorState] Cannot pop state - state machine is empty or invalid");
                }
            });

            m_inputHandler->HandleKeyPress(Renderer::Key::S, [this]() {
                if (m_renderer->IsKeyPressed(Renderer::Key::LControl) ||
                    m_renderer->IsKeyPressed(Renderer::Key::RControl)) {
                    saveCurrentLevel();
                }
            });

            m_inputHandler->HandleKeyPress(Renderer::Key::O, [this]() {
                if (m_renderer->IsKeyPressed(Renderer::Key::LControl) ||
                    m_renderer->IsKeyPressed(Renderer::Key::RControl)) {

                    std::string path = m_currentLevelPath.empty()
                        ? "assets/levels/level1.json"
                        : m_currentLevelPath;

                    auto loadedEntities = m_fileManager->LoadLevel(path, m_registry, m_renderer.get());
                    if (!loadedEntities.empty()) {
                        Core::Logger::Info("[EditorState] Level loaded from {}", path);
                        m_currentLevelPath = path;
                        m_hasUnsavedChanges = false;
                    } else {
                        Core::Logger::Error("[EditorState] Failed to load: {}", m_fileManager->GetLastError());
                    }
                }
            });

            bool blockCameraInput = false;
            if (m_propertyManager && m_entityManager && m_entityManager->GetSelectedEntity()) {
                auto handleKeyPress = [this](Renderer::Key key, std::function<void()> action) {
                    m_inputHandler->HandleKeyPress(key, action);
                };
                blockCameraInput = m_propertyManager->HandleInput(m_entityManager->GetSelectedEntity(), handleKeyPress);
            }

            Math::Vector2 mouseScreen = m_renderer->GetMousePosition();
            if (m_uiManager) {
                m_uiManager->UpdateHover(mouseScreen);
            }

            if (m_canvasManager && !blockCameraInput) {
                m_canvasManager->HandleCameraInput();
            }

            bool isLeftPressed = m_renderer->IsMouseButtonPressed(Renderer::IRenderer::MouseButton::Left);
            if (isLeftPressed && !m_leftMousePressed) {
                bool consumedByUI = false;

                if (m_uiManager) {
                    if (m_uiManager->HandleActionClick(mouseScreen)) {
                        consumedByUI = true;
                    } else {
                        auto selection = m_uiManager->HandleClick(mouseScreen);
                        if (selection.has_value()) {
                            m_selection = selection.value();
                            consumedByUI = true;
                            if (m_propertyManager) {
                                m_propertyManager->ClearInput();
                            }
                        }
                    }
                }

                if (!consumedByUI && m_canvasManager && m_entityManager) {
                    Math::Vector2 mouseWorld = m_canvasManager->ScreenToWorld(mouseScreen);
                    if (m_selection.mode != EditorMode::SELECT) {
                        if (m_entityManager->PlaceEntity(m_selection, mouseWorld)) {
                            m_hasUnsavedChanges = true;
                            if (m_propertyManager) {
                                m_propertyManager->ClearInput();
                            }

                            m_selection.mode = EditorMode::SELECT;
                            m_selection.subtype.clear();
                            if (m_uiManager) {
                                m_uiManager->SetActiveSelection(m_selection);
                            }
                            updatePropertyPanel();
                        }
                    } else {
                        handleSelectionAt(mouseWorld);
                    }
                }
            }
            m_leftMousePressed = isLeftPressed;
        }

        void EditorState::Update(float dt) {
            (void)dt;

            if (!m_canvasManager || m_statusBarEntities.size() < 3) {
                return;
            }

            const auto& camera = m_canvasManager->GetCamera();
            Math::Vector2 mouseScreen = m_renderer->GetMousePosition();
            Math::Vector2 mouseWorld = m_canvasManager->ScreenToWorld(mouseScreen);
            m_lastMouseWorld = mouseWorld;

            auto formatLine = [](int index, const auto&... args) {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(1);
                (oss << ... << args);
                return oss.str();
            };

            if (m_registry.IsEntityAlive(m_statusBarEntities[0])) {
                auto& label0 = m_registry.GetComponent<ECS::TextLabel>(m_statusBarEntities[0]);
                label0.text = formatLine(0, "camera: (", camera.x, ", ", camera.y, ")");
            }

            if (m_registry.IsEntityAlive(m_statusBarEntities[1])) {
                auto& label1 = m_registry.GetComponent<ECS::TextLabel>(m_statusBarEntities[1]);
                label1.text = formatLine(1, "zoom: ", camera.zoom, "x");
            }

            if (m_registry.IsEntityAlive(m_statusBarEntities[2])) {
                auto& label2 = m_registry.GetComponent<ECS::TextLabel>(m_statusBarEntities[2]);
                label2.text = formatLine(2, "mouse: (", mouseWorld.x, ", ", mouseWorld.y, ")");
            }
        }

        void EditorState::Draw() {
            m_renderer->Clear(Colors::BACKGROUND);

            if (m_canvasManager) {
                m_canvasManager->ApplyCamera();
                m_canvasManager->DrawGrid();
            }

            m_renderingSystem->Update(m_registry, 0.0f);

            if (m_entityManager) {
                m_entityManager->DrawSelectionOutline();

                if (m_entityManager->GetSelectedEntity()) {
                    m_entityManager->DrawColliders(m_entityManager->GetSelectedColliderIndex());
                }

                m_entityManager->DrawPlacementPreview(m_selection.mode,
                                                      m_selection.entityType,
                                                      m_selection.subtype,
                                                      m_lastMouseWorld);
            }

            if (m_canvasManager) {
                m_renderer->ResetCamera();
            }

            m_textSystem->Update(m_registry, 0.0f);
        }

        void EditorState::handleSelectionAt(const Math::Vector2& mouseWorld) {
            if (!m_entityManager) {
                return;
            }

            if (!m_entityManager->SelectAt(mouseWorld)) {
                m_entityManager->ClearSelection();
            }

            if (m_propertyManager) {
                m_propertyManager->ClearInput();
            }
            updatePropertyPanel();
        }

        void EditorState::updatePropertyPanel() {
            if (!m_uiManager || !m_entityManager || !m_propertyManager) {
                return;
            }
            const EditorEntityData* selected = m_entityManager->GetSelectedEntity();
            m_uiManager->UpdatePropertyPanel(selected, m_propertyManager->GetActiveProperty(), m_propertyManager->GetInputBuffer());
            m_uiManager->UpdateColliderPanel(selected, m_entityManager->GetSelectedColliderIndex());
        }

        void EditorState::deleteSelectedEntity() {
            if (!m_entityManager) {
                return;
            }

            if (m_entityManager->DeleteSelected()) {
                m_hasUnsavedChanges = true;
                if (m_propertyManager) {
                    m_propertyManager->ClearInput();
                }
                updatePropertyPanel();
            }
        }

        void EditorState::saveCurrentLevel() {
            if (!m_fileManager || !m_entityManager) {
                return;
            }

            std::string path = m_currentLevelPath.empty()
                ? "assets/levels/custom_level.json"
                : m_currentLevelPath;

            if (m_fileManager->SaveLevel(path, m_entityManager->GetEntities(), m_levelName)) {
                Core::Logger::Info("[EditorState] Level saved to {}", path);
                m_hasUnsavedChanges = false;
                m_currentLevelPath = path;
            } else {
                Core::Logger::Error("[EditorState] Failed to save: {}", m_fileManager->GetLastError());
            }
        }


    }
}
