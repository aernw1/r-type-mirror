/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorInputHandler
*/

#pragma once

#include "Renderer/IRenderer.hpp"
#include <functional>
#include <unordered_map>

namespace RType {
    namespace Client {

        class EditorInputHandler {
        public:
            explicit EditorInputHandler(Renderer::IRenderer* renderer);
            ~EditorInputHandler() = default;

            void HandleKeyPress(Renderer::Key key, const std::function<void()>& action);
            void Update();

        private:
            Renderer::IRenderer* m_renderer;
            std::unordered_map<Renderer::Key, bool> m_keyStates;
        };

    }
}
