/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorPropertyManager
*/

#pragma once

#include "editor/EditorTypes.hpp"
#include "Renderer/IRenderer.hpp"
#include <string>
#include <functional>

namespace RType {
    namespace Client {

        class EditorPropertyManager {
        public:
            explicit EditorPropertyManager(Renderer::IRenderer* renderer);
            ~EditorPropertyManager() = default;

            bool HandleInput(EditorEntityData* selectedEntity,
                             std::function<void(Renderer::Key, std::function<void()>)> handleKeyPress);

            EditableProperty GetActiveProperty() const { return m_activeProperty; }
            const std::string& GetInputBuffer() const { return m_inputBuffer; }
            void ClearInput();
            void SetOnPropertyChanged(std::function<void(EditorEntityData&)> callback);
            void SetOnEntityDeleted(std::function<void()> callback);

        private:
            Renderer::IRenderer* m_renderer;

            EditableProperty m_activeProperty = EditableProperty::POSITION_X;
            std::string m_inputBuffer;

            std::function<void(EditorEntityData&)> m_onPropertyChanged;
            std::function<void()> m_onEntityDeleted;

            void cycleProperty();
            void applyPropertyDelta(EditorEntityData& entity, float delta);
            void setPropertyValue(EditorEntityData& entity, float value);
            float getPropertyValue(const EditorEntityData& entity) const;
            float getPropertyStep() const;
            void handleNumberInput(Renderer::Key key);
            void handleBackspace(EditorEntityData* entity);
        };

    }
}
