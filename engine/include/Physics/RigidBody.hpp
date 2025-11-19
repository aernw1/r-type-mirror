#pragma once

#include <memory>
#include <utility>
#include <cstdint>

namespace Physics {

    class Collider;

    enum class BodyType : uint8_t {
        Static,
        Dynamic,
        Kinematic
    };

    class RigidBody {
    public:
        virtual ~RigidBody() = default;

        virtual void setPosition(float x, float y) = 0;
        virtual std::pair<float, float> getPosition() const = 0;
        virtual void setVelocity(float vx, float vy) = 0;
        virtual std::pair<float, float> getVelocity() const = 0;
        virtual void applyForce(float fx, float fy) = 0;
        virtual void setBodyType(BodyType type) = 0;
        virtual BodyType getBodyType() const = 0;
        virtual void setCollider(const std::shared_ptr<Collider>& collider) = 0;
        virtual std::shared_ptr<Collider> getCollider() const = 0;
    };

}
