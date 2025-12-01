#pragma once

#include "ECS/Component.hpp"
#include "Math/Types.hpp"
#include <string>

namespace RType {
    namespace ECS {

        struct TextLabel : public IComponent {
            std::string text;
            Renderer::FontId fontId = 0;
            unsigned int characterSize = 24;
            Math::Color color{1.0f, 1.0f, 1.0f, 1.0f};

            float offsetX = 0.0f;
            float offsetY = 0.0f;
            bool centered = false;

            TextLabel() = default;
            TextLabel(const std::string& t, Renderer::FontId font, unsigned int size = 24)
                : text(t), fontId(font), characterSize(size) {}
        };
    }
}
