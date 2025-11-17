#pragma once

#include <string>
#include <cstdint>
#include <typeindex>
#include <type_traits>

namespace RType {

    namespace ECS {
        using ComponentID = std::uuid;

        struct IComponent {
            virtual ~IComponent() = default;
        };

        struct Position : public IComponent {
            float x = 0.0f;
            float y = 0.0f;

            Position() = default;
            Position(float x, float y) : x(x), y(y) {}
        };

        struct Velocity : public IComponent {
            float dx = 0.0f;
            float dy = 0.0f;

            Velocity() = default;
            Velocity(float dx, float dy) : dx(dx), dy(dy) {}
        };
    }
}