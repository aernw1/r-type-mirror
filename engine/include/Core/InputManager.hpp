/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** InputManager - Action and axis-based input abstraction
*/

#pragma once

#include "../Renderer/IRenderer.hpp"
#include <string>
#include <unordered_map>
#include <functional>

namespace RType {

    namespace Core {

        class InputManager {
        public:
            InputManager() = default;
            ~InputManager() = default;

            // Non-copyable
            InputManager(const InputManager&) = delete;
            InputManager& operator=(const InputManager&) = delete;

            void bindAction(const std::string& action, Renderer::Key key);
            void bindActionMouse(const std::string& action, Renderer::MouseButton button);
            void unbindAction(const std::string& action);
            bool isActionPressed(const std::string& action) const;
            bool isActionJustPressed(const std::string& action) const;
            bool isActionJustReleased(const std::string& action) const;
            void bindAxis(const std::string& axis, Renderer::Key negative, Renderer::Key positive);
            void unbindAxis(const std::string& axis);
            float getAxisValue(const std::string& axis) const;

            void update(Renderer::IRenderer* renderer);

            void clearBindings();

            void loadDefaults();

        private:
            struct ActionBinding {
                Renderer::Key key = Renderer::Key::Unknown;
                Renderer::MouseButton mouseButton = Renderer::MouseButton::Left;
                bool isMouseBinding = false;
                bool currentlyPressed = false;
                bool wasPressed = false;
            };

            struct AxisBinding {
                Renderer::Key negative = Renderer::Key::Unknown;
                Renderer::Key positive = Renderer::Key::Unknown;
            };

            std::unordered_map<std::string, ActionBinding> m_actions;
            std::unordered_map<std::string, AxisBinding> m_axes;
            Renderer::IRenderer* m_renderer = nullptr;
        };

    }

}
