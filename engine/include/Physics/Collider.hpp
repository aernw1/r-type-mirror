#pragma once

#include <cstdint>

namespace Physics {

    enum class ColliderShape : uint8_t {
        Box,
        Circle
    };

    class Collider {
    public:
        virtual ~Collider() = default;

        virtual void setAsBox(float width, float height) = 0;
        virtual void setAsCircle(float radius) = 0;
        virtual void setIsTrigger(bool isTrigger) = 0;
        virtual bool isTrigger() const = 0;
        virtual ColliderShape getShape() const = 0;
        virtual float getWidth() const = 0;
        virtual float getHeight() const = 0;
        virtual float getRadius() const = 0;
    };

}
