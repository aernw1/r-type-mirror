/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** InputManager implementation
*/

#include "Core/InputManager.hpp"

namespace RType {

    namespace Core {

        void InputManager::bindAction(const std::string& action, Renderer::Key key) {
            ActionBinding binding;
            binding.key = key;
            binding.isMouseBinding = false;
            m_actions[action] = binding;
        }

        void InputManager::bindActionMouse(const std::string& action, Renderer::MouseButton button) {
            ActionBinding binding;
            binding.mouseButton = button;
            binding.isMouseBinding = true;
            m_actions[action] = binding;
        }

        void InputManager::unbindAction(const std::string& action) {
            m_actions.erase(action);
        }

        bool InputManager::isActionPressed(const std::string& action) const {
            auto it = m_actions.find(action);
            if (it == m_actions.end()) {
                return false;
            }
            return it->second.currentlyPressed;
        }

        bool InputManager::isActionJustPressed(const std::string& action) const {
            auto it = m_actions.find(action);
            if (it == m_actions.end()) {
                return false;
            }
            return it->second.currentlyPressed && !it->second.wasPressed;
        }

        bool InputManager::isActionJustReleased(const std::string& action) const {
            auto it = m_actions.find(action);
            if (it == m_actions.end()) {
                return false;
            }
            return !it->second.currentlyPressed && it->second.wasPressed;
        }

        void InputManager::bindAxis(const std::string& axis, Renderer::Key negative, Renderer::Key positive) {
            AxisBinding binding;
            binding.negative = negative;
            binding.positive = positive;
            m_axes[axis] = binding;
        }

        void InputManager::unbindAxis(const std::string& axis) {
            m_axes.erase(axis);
        }

        float InputManager::getAxisValue(const std::string& axis) const {
            auto it = m_axes.find(axis);
            if (it == m_axes.end() || m_renderer == nullptr) {
                return 0.0F;
            }

            float value = 0.0F;
            if (m_renderer->IsKeyPressed(it->second.negative)) {
                value -= 1.0F;
            }
            if (m_renderer->IsKeyPressed(it->second.positive)) {
                value += 1.0F;
            }
            return value;
        }

        void InputManager::update(Renderer::IRenderer* renderer) {
            m_renderer = renderer;
            if (m_renderer == nullptr) {
                return;
            }

            for (auto& [name, binding] : m_actions) {
                binding.wasPressed = binding.currentlyPressed;
                
                if (binding.isMouseBinding) {
                    binding.currentlyPressed = m_renderer->IsMouseButtonPressed(binding.mouseButton);
                } else {
                    binding.currentlyPressed = m_renderer->IsKeyPressed(binding.key);
                }
            }
        }

        void InputManager::clearBindings() {
            m_actions.clear();
            m_axes.clear();
        }

        void InputManager::loadDefaults() {
            // Movement
            bindAction("MoveLeft", Renderer::Key::A);
            bindAction("MoveRight", Renderer::Key::D);
            bindAction("MoveUp", Renderer::Key::W);
            bindAction("MoveDown", Renderer::Key::S);
            
            // Movement axes
            bindAxis("MoveX", Renderer::Key::A, Renderer::Key::D);
            bindAxis("MoveY", Renderer::Key::S, Renderer::Key::W);  // S=up (positive Y), W=down
            
            // Actions
            bindAction("Jump", Renderer::Key::Space);
            bindActionMouse("Shoot", Renderer::MouseButton::Left);
            bindAction("Pause", Renderer::Key::Escape);
        }

    }

}
