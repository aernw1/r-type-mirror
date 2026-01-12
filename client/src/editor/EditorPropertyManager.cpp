/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorPropertyManager
*/

#include "editor/EditorPropertyManager.hpp"
#include "editor/EditorConstants.hpp"
#include <algorithm>

using namespace RType::Client::EditorConstants;

namespace RType {
    namespace Client {

        EditorPropertyManager::EditorPropertyManager(Renderer::IRenderer* renderer)
            : m_renderer(renderer)
        {
        }

        bool EditorPropertyManager::HandleInput(EditorEntityData* selectedEntity,
                                                 std::function<void(Renderer::Key, std::function<void()>)> handleKeyPress) {
            if (!selectedEntity) {
                return false;
            }

            bool consumedDirectional = m_renderer->IsKeyPressed(Renderer::Key::Up) ||
                m_renderer->IsKeyPressed(Renderer::Key::Down);

            // Tab: cycle through properties
            handleKeyPress(Renderer::Key::Tab, [this]() {
                cycleProperty();
            });

            // Arrow keys: adjust property values
            handleKeyPress(Renderer::Key::Up, [this, selectedEntity]() {
                applyPropertyDelta(*selectedEntity, getPropertyStep());
            });
            handleKeyPress(Renderer::Key::Down, [this, selectedEntity]() {
                applyPropertyDelta(*selectedEntity, -getPropertyStep());
            });

            // Number keys: direct value input
            const Renderer::Key digitKeys[10] = {
                Renderer::Key::Num0, Renderer::Key::Num1, Renderer::Key::Num2, Renderer::Key::Num3, Renderer::Key::Num4,
                Renderer::Key::Num5, Renderer::Key::Num6, Renderer::Key::Num7, Renderer::Key::Num8, Renderer::Key::Num9
            };

            for (int i = 0; i < 10; ++i) {
                handleKeyPress(digitKeys[i], [this, selectedEntity, key = digitKeys[i]]() {
                    handleNumberInput(key);
                    // Apply the new value immediately if buffer is not empty
                    if (!m_inputBuffer.empty()) {
                        setPropertyValue(*selectedEntity, std::stof(m_inputBuffer));
                    }
                });
            }

            // Backspace: remove digit or delete entity
            handleKeyPress(Renderer::Key::Backspace, [this, selectedEntity]() {
                handleBackspace(selectedEntity);
            });

            // Enter: commit current value
            handleKeyPress(Renderer::Key::Enter, [this]() {
                ClearInput();
            });

            return consumedDirectional;
        }

        void EditorPropertyManager::ClearInput() {
            m_inputBuffer.clear();
        }

        void EditorPropertyManager::SetOnPropertyChanged(std::function<void(EditorEntityData&)> callback) {
            m_onPropertyChanged = callback;
        }

        void EditorPropertyManager::SetOnEntityDeleted(std::function<void()> callback) {
            m_onEntityDeleted = callback;
        }

        void EditorPropertyManager::cycleProperty() {
            int current = static_cast<int>(m_activeProperty);
            current = (current + 1) % static_cast<int>(EditableProperty::COUNT);
            m_activeProperty = static_cast<EditableProperty>(current);
            ClearInput();
        }

        void EditorPropertyManager::applyPropertyDelta(EditorEntityData& entity, float delta) {
            float newValue = getPropertyValue(entity) + delta;
            setPropertyValue(entity, newValue);
            ClearInput();
        }

        void EditorPropertyManager::setPropertyValue(EditorEntityData& entity, float value) {
            switch (m_activeProperty) {
            case EditableProperty::POSITION_X:
                entity.x = value;
                break;
            case EditableProperty::POSITION_Y:
                entity.y = value;
                break;
            case EditableProperty::SCALE_WIDTH:
                entity.scaleWidth = std::max(PropertySteps::MIN_SCALE, value);
                break;
            case EditableProperty::SCALE_HEIGHT:
                entity.scaleHeight = std::max(PropertySteps::MIN_SCALE, value);
                break;
            case EditableProperty::LAYER:
                entity.layer = static_cast<int>(value);
                break;
            case EditableProperty::SCROLL_SPEED:
                entity.scrollSpeed = value;
                break;
            case EditableProperty::COUNT:
                break;
            }

            // Notify that property changed (for collider rebuild and UI update)
            if (m_onPropertyChanged) {
                m_onPropertyChanged(entity);
            }
        }

        float EditorPropertyManager::getPropertyValue(const EditorEntityData& entity) const {
            switch (m_activeProperty) {
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

        float EditorPropertyManager::getPropertyStep() const {
            switch (m_activeProperty) {
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

        void EditorPropertyManager::handleNumberInput(Renderer::Key key) {
            if (m_inputBuffer.size() >= Input::MAX_INPUT_BUFFER_SIZE) {
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

            m_inputBuffer.push_back(digit);
            // Note: We don't have access to selectedEntity here, so we rely on the
            // callback pattern where EditorState will call setPropertyValue
        }

        void EditorPropertyManager::handleBackspace(EditorEntityData* entity) {
            if (!m_inputBuffer.empty()) {
                m_inputBuffer.pop_back();
                if (!m_inputBuffer.empty() && entity) {
                    // Update property with new buffer value
                    setPropertyValue(*entity, std::stof(m_inputBuffer));
                }
            } else {
                // Empty buffer + backspace = delete entity
                if (m_onEntityDeleted) {
                    m_onEntityDeleted();
                }
            }
        }

    }
}
