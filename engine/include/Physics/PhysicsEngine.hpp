#pragma once

#include <memory>
#include <utility>

namespace Physics {

    class IPhysicsWorld;

    class PhysicsEngine {
    public:
        virtual ~PhysicsEngine() = default;

        virtual std::shared_ptr<IPhysicsWorld> createWorld() = 0;
        virtual void setGravity(float gx, float gy) = 0;
        virtual std::pair<float, float> getGravity() const = 0;
    };

}
