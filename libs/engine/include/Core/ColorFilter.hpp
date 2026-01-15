#pragma once

#include "Math/Types.hpp"

namespace RType {
    namespace Core {

        class ColorFilter {
        public:
            static bool IsColourBlindModeEnabled();
            static void SetColourBlindMode(bool enabled);
            static Math::Color ApplyColourBlindFilter(const Math::Color& color);

        private:
            static bool s_colourBlindMode;
        };

    }
}

