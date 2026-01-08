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

            m_canvasManager = std::make_unique<EditorCanvasManager>(m_renderer.get());
            m_assetLibrary = std::make_unique<EditorAssetLibrary>(m_renderer.get());
            m_assetLibrary->Initialize();

            m_entityManager = std::make_unique<EditorEntityManager>(m_renderer.get(), m_registry, *m_assetLibrary);
            m_uiManager = std::make_unique<EditorUIManager>(m_renderer.get(), *m_assetLibrary, m_registry, m_entities, m_fontSmall, m_fontMedium);
            m_uiManager->InitializePalette();
            m_selection = m_uiManager->GetActiveSelection();
            updatePropertyPanel();

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

            bool blockCameraInput = handlePropertyEditing();

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
                    auto selection = m_uiManager->HandleClick(mouseScreen);
                    if (selection.has_value()) {
                        m_selection = selection.value();
                        consumedByUI = true;
                        clearValueInput();
                    }
                }

                if (!consumedByUI && m_canvasManager && m_entityManager) {
                    Math::Vector2 mouseWorld = m_canvasManager->ScreenToWorld(mouseScreen);
                    if (m_selection.mode != EditorMode::SELECT) {
                        if (m_entityManager->PlaceEntity(m_selection, mouseWorld)) {
                            m_hasUnsavedChanges = true;
                            clearValueInput();

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

            m_activeProperty = EditableProperty::POSITION_X;
            clearValueInput();
            updatePropertyPanel();
        }

        bool EditorState::handlePropertyEditing() {
            if (!m_entityManager || !m_entityManager->GetSelectedEntity()) {
                return false;
            }

            auto handleKeyPress = [this](Renderer::Key key, bool& stateFlag, auto&& action) {
                if (m_renderer->IsKeyPressed(key)) {
                    if (!stateFlag) {
                        action();
                        stateFlag = true;
                    }
                } else {
                    stateFlag = false;
                }
            };

            bool consumedDirectional = m_renderer->IsKeyPressed(Renderer::Key::Up) ||
                m_renderer->IsKeyPressed(Renderer::Key::Down);

            handleKeyPress(Renderer::Key::Tab, m_tabKeyPressed, [this]() {
                cycleProperty();
            });

            handleKeyPress(Renderer::Key::Up, m_propUpPressed, [this]() {
                applyPropertyDelta(getPropertyStep(m_activeProperty));
            });
            handleKeyPress(Renderer::Key::Down, m_propDownPressed, [this]() {
                applyPropertyDelta(-getPropertyStep(m_activeProperty));
            });

            const Renderer::Key digitKeys[10] = {
                Renderer::Key::Num0, Renderer::Key::Num1, Renderer::Key::Num2, Renderer::Key::Num3, Renderer::Key::Num4,
                Renderer::Key::Num5, Renderer::Key::Num6, Renderer::Key::Num7, Renderer::Key::Num8, Renderer::Key::Num9
            };

            for (int i = 0; i < 10; ++i) {
                if (m_renderer->IsKeyPressed(digitKeys[i])) {
                    if (!m_numberKeyPressed[static_cast<size_t>(i)]) {
                        handleNumberInput(digitKeys[i]);
                        m_numberKeyPressed[static_cast<size_t>(i)] = true;
                    }
                } else {
                    m_numberKeyPressed[static_cast<size_t>(i)] = false;
                }
            }

            handleKeyPress(Renderer::Key::Backspace, m_backspacePressed, [this]() {
                if (!m_propertyInputBuffer.empty()) {
                    m_propertyInputBuffer.pop_back();
                    if (m_propertyInputBuffer.empty()) {
                        updatePropertyPanel();
                    } else {
                        setPropertyValue(std::stof(m_propertyInputBuffer));
                    }
                } else {
                    deleteSelectedEntity();
                }
            });

            handleKeyPress(Renderer::Key::Enter, m_enterPressed, [this]() {
                clearValueInput();
                updatePropertyPanel();
            });

            return consumedDirectional;
        }

        void EditorState::cycleProperty() {
            int current = static_cast<int>(m_activeProperty);
            current = (current + 1) % static_cast<int>(EditableProperty::COUNT);
            m_activeProperty = static_cast<EditableProperty>(current);
            clearValueInput();
            updatePropertyPanel();
        }

        void EditorState::applyPropertyDelta(float delta) {
            auto* entity = m_entityManager ? m_entityManager->GetSelectedEntity() : nullptr;
            if (!entity) {
                return;
            }

            float newValue = getPropertyValue(*entity, m_activeProperty) + delta;
            setPropertyValue(newValue);
            clearValueInput();
        }

        void EditorState::setPropertyValue(float value) {
            auto* entity = m_entityManager ? m_entityManager->GetSelectedEntity() : nullptr;
            if (!entity) {
                return;
            }

            switch (m_activeProperty) {
            case EditableProperty::POSITION_X:
                entity->x = value;
                break;
            case EditableProperty::POSITION_Y:
                entity->y = value;
                break;
            case EditableProperty::SCALE_WIDTH:
                entity->scaleWidth = std::max(PropertySteps::MIN_SCALE, value);
                break;
            case EditableProperty::SCALE_HEIGHT:
                entity->scaleHeight = std::max(PropertySteps::MIN_SCALE, value);
                break;
            case EditableProperty::LAYER:
                entity->layer = static_cast<int>(value);
                break;
            case EditableProperty::SCROLL_SPEED:
                entity->scrollSpeed = value;
                break;
            case EditableProperty::COUNT:
                break;
            }

            m_hasUnsavedChanges = true;

            if (m_entityManager) {
                entity->colliders.clear();
                ECS::ColliderDef collider;
                collider.x = entity->x - entity->scaleWidth / 2.0f;
                collider.y = entity->y - entity->scaleHeight / 2.0f;
                collider.width = entity->scaleWidth;
                collider.height = entity->scaleHeight;
                entity->colliders.push_back(collider);
                m_entityManager->SyncEntity(*entity);
            }

            updatePropertyPanel();
        }

        float EditorState::getPropertyValue(const EditorEntityData& entity, EditableProperty property) const {
            switch (property) {
            case EditableProperty::POSITION_X:
                return entity.x;
            case EditableProperty::POSITION_Y:
                return entity.y;
            case EditableProperty::SCALE_WIDTH:
                return entity.scaleWidth;
            case EditableProperty::SCALE_HEIGHT:
                return entity.scaleHeight;
            case EditableProperty::LAYER:
                return static_cast<float>(entity.layer);
            case EditableProperty::SCROLL_SPEED:
                return entity.scrollSpeed;
            case EditableProperty::COUNT:
                break;
            }
            return 0.0f;
        }

        float EditorState::getPropertyStep(EditableProperty property) const {
            switch (property) {
            case EditableProperty::POSITION_X:
            case EditableProperty::POSITION_Y:
                return PropertySteps::POSITION_STEP;
            case EditableProperty::SCALE_WIDTH:
            case EditableProperty::SCALE_HEIGHT:
                return PropertySteps::SCALE_STEP;
            case EditableProperty::LAYER:
                return PropertySteps::LAYER_STEP;
            case EditableProperty::SCROLL_SPEED:
                return PropertySteps::SCROLL_SPEED_STEP;
            case EditableProperty::COUNT:
                break;
            }
            return PropertySteps::LAYER_STEP;
        }

        void EditorState::updatePropertyPanel() {
            if (!m_uiManager || !m_entityManager) {
                return;
            }
            const EditorEntityData* selected = m_entityManager->GetSelectedEntity();
            m_uiManager->UpdatePropertyPanel(selected, m_activeProperty, m_propertyInputBuffer);
            m_uiManager->UpdateColliderPanel(selected, m_entityManager->GetSelectedColliderIndex());
        }

        void EditorState::clearValueInput() {
            m_propertyInputBuffer.clear();
        }

        void EditorState::handleNumberInput(Renderer::Key key) {
            if (m_propertyInputBuffer.size() >= Input::MAX_INPUT_BUFFER_SIZE) {
                return;
            }

            char digit = '0';
            switch (key) {
            case Renderer::Key::Num0: digit = '0'; break;
            case Renderer::Key::Num1: digit = '1'; break;
            case Renderer::Key::Num2: digit = '2'; break;
            case Renderer::Key::Num3: digit = '3'; break;
            case Renderer::Key::Num4: digit = '4'; break;
            case Renderer::Key::Num5: digit = '5'; break;
            case Renderer::Key::Num6: digit = '6'; break;
            case Renderer::Key::Num7: digit = '7'; break;
            case Renderer::Key::Num8: digit = '8'; break;
            case Renderer::Key::Num9: digit = '9'; break;
            default:
                return;
            }

            m_propertyInputBuffer.push_back(digit);
            setPropertyValue(std::stof(m_propertyInputBuffer));
        }

        void EditorState::deleteSelectedEntity() {
            if (!m_entityManager) {
                return;
            }

            if (m_entityManager->DeleteSelected()) {
                m_hasUnsavedChanges = true;
                clearValueInput();
                updatePropertyPanel();
            }
        }


    }
}
