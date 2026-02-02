/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorInputHandler
*/

#include "editor/EditorInputHandler.hpp"

namespace RType {
    namespace Client {

        EditorInputHandler::EditorInputHandler(Renderer::IRenderer* renderer)
            : m_renderer(renderer)
        {
        }

        void EditorInputHandler::HandleKeyPress(Renderer::Key key, const std::function<void()>& action) {
            if (!m_renderer) {
                return;
            }

            bool isCurrentlyPressed = m_renderer->IsKeyPressed(key);
            bool wasPreviouslyPressed = m_keyStates[key];

            // Action triggers only on transition from not pressed to pressed
            if (isCurrentlyPressed && !wasPreviouslyPressed) {
                action();
            }

            m_keyStates[key] = isCurrentlyPressed;
        }

        void EditorInputHandler::Update() {
            // Update is now handled within HandleKeyPress itself
            // This method is kept for potential future frame-based reset logic
            // if needed, but currently the state is updated immediately
        }

    }
}
