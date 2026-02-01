#include "../include/Core/InputMapping.hpp"

namespace RType {
    namespace Core {

        std::unordered_map<std::string, Renderer::Key> InputMapping::s_mappings = {
            {"MOVE_UP", Renderer::Key::Up},
            {"MOVE_DOWN", Renderer::Key::Down},
            {"MOVE_LEFT", Renderer::Key::Left},
            {"MOVE_RIGHT", Renderer::Key::Right},
            {"SHOOT", Renderer::Key::Space}
        };

        Renderer::Key InputMapping::GetKey(const std::string& action) {
            auto it = s_mappings.find(action);
            if (it != s_mappings.end()) {
                return it->second;
            }
            return Renderer::Key::Unknown;
        }

        void InputMapping::SetKey(const std::string& action, Renderer::Key key) {
            s_mappings[action] = key;
        }

        void InputMapping::LoadDefaults() {
            s_mappings = {
                {"MOVE_UP", Renderer::Key::Up},
                {"MOVE_DOWN", Renderer::Key::Down},
                {"MOVE_LEFT", Renderer::Key::Left},
                {"MOVE_RIGHT", Renderer::Key::Right},
                {"SHOOT", Renderer::Key::Space}
            };
        }

    }
}

