#pragma once

#include "ECS/Component.hpp"
#include <functional>

namespace RType {
    namespace ECS {

        enum class ButtonState {
            Idle,
            Hover,
            Active
        };

        struct Clickable : public IComponent {
            float width = 0.0f;
            float height = 0.0f;
            
            ButtonState state = ButtonState::Idle;
            
            int actionId = 0; 

            Clickable() = default;
            Clickable(float w, float h, int action)
                : width(w), height(h), actionId(action) {}
        };

    }
}

