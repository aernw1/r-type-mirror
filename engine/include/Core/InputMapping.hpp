#pragma once

#include "../Renderer/IRenderer.hpp"
#include <string>
#include <unordered_map>

namespace RType {
    namespace Core {

        class InputMapping {
        public:
            static Renderer::Key GetKey(const std::string& action);
            static void SetKey(const std::string& action, Renderer::Key key);
            static void LoadDefaults();

        private:
            static std::unordered_map<std::string, Renderer::Key> s_mappings;
        };

    }
}

