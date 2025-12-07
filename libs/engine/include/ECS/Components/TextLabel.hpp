#pragma once

#include "ECS/Component.hpp"
#include "Math/Types.hpp"
#include "Serialization/Serializer.hpp"
#include "Serialization/Deserializer.hpp"
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

            void serialize(Engine::Serializer& ser) const {
                ser.serializePascalString<uint16_t>(text);
                ser.serializeTrivial(fontId);
                ser.serializeTrivial(characterSize);
                ser.serializeTrivial(color.r);
                ser.serializeTrivial(color.g);
                ser.serializeTrivial(color.b);
                ser.serializeTrivial(color.a);
                ser.serializeTrivial(offsetX);
                ser.serializeTrivial(offsetY);
                ser.serializeTrivial(centered);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializePascalString<uint16_t>(text);
                deser.deserializeTrivial(fontId);
                deser.deserializeTrivial(characterSize);
                deser.deserializeTrivial(color.r);
                deser.deserializeTrivial(color.g);
                deser.deserializeTrivial(color.b);
                deser.deserializeTrivial(color.a);
                deser.deserializeTrivial(offsetX);
                deser.deserializeTrivial(offsetY);
                deser.deserializeTrivial(centered);
            }
        };
    }
}
