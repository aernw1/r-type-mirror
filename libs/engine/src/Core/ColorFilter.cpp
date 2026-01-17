#include "Core/ColorFilter.hpp"
#include <algorithm>

namespace RType {
    namespace Core {

        bool ColorFilter::s_colourBlindMode = false;

        bool ColorFilter::IsColourBlindModeEnabled() {
            return s_colourBlindMode;
        }

        void ColorFilter::SetColourBlindMode(bool enabled) {
            s_colourBlindMode = enabled;
        }

        Math::Color ColorFilter::ApplyColourBlindFilter(const Math::Color& color) {
            if (!s_colourBlindMode) {
                return color;
            }

            float r = color.r;
            float g = color.g;
            float b = color.b;

            float newR = 0.567f * r + 0.433f * g + 0.0f * b;
            float newG = 0.558f * r + 0.442f * g + 0.0f * b;
            float newB = 0.0f * r + 0.242f * g + 0.758f * b;

            newR = std::clamp(newR, 0.0f, 1.0f);
            newG = std::clamp(newG, 0.0f, 1.0f);
            newB = std::clamp(newB, 0.0f, 1.0f);

            return Math::Color(newR, newG, newB, color.a);
        }

    }
}

