#pragma once

#include <string>
#include <cstdint>
#include <typeindex>
#include <type_traits>
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"

namespace RType {

    namespace ECS {
        using ComponentID = std::type_index;

        struct IComponent {
            virtual ~IComponent() = default;
        };

        struct Position : public IComponent {
            float x = 0.0f;
            float y = 0.0f;

            Position() = default;
            Position(float x, float y)
                : x(x), y(y) {}
        };

        struct Velocity : public IComponent {
            float dx = 0.0f;
            float dy = 0.0f;

            Velocity() = default;
            Velocity(float dx, float dy)
                : dx(dx), dy(dy) {}
        };

        struct Drawable : public IComponent {
            Renderer::SpriteId spriteId = Renderer::INVALID_SPRITE_ID;
            Math::Vector2 scale{1.0f, 1.0f};
            float rotation = 0.0f;
            Math::Vector2 origin{0.0f, 0.0f};
            Math::Color tint{1.0f, 1.0f, 1.0f, 1.0f};
            int layer = 0;

            Drawable() = default;
            Drawable(Renderer::SpriteId sprite, int renderLayer = 0)
                : spriteId(sprite), layer(renderLayer) {}
        };

        struct Controllable : public IComponent {
            float speed = 200.0f;  // Pixels per second

            Controllable() = default;
            Controllable(float moveSpeed)
                : speed(moveSpeed) {}
        };
    }

}