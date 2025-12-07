#pragma once

#include "ECS/Component.hpp"
#include "Serialization/Serializer.hpp"
#include "Serialization/Deserializer.hpp"
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

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(width);
                ser.serializeTrivial(height);
                ser.serializeTrivial(state);
                ser.serializeTrivial(actionId);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(width);
                deser.deserializeTrivial(height);
                deser.deserializeTrivial(state);
                deser.deserializeTrivial(actionId);
            }
        };

    }
}
