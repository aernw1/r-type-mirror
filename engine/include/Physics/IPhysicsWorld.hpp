#pragma once

#include <memory>
#include <vector>

namespace Physics {

    class RigidBody;

    class IPhysicsWorld {
    public:
        virtual ~IPhysicsWorld() = default;

        virtual void update(float deltaTime) = 0;
        virtual std::shared_ptr<RigidBody> createRigidBody() = 0;
        virtual void destroyRigidBody(const std::shared_ptr<RigidBody>& body) = 0;
        virtual std::vector<std::shared_ptr<RigidBody>> getRigidBodies() const = 0;
        virtual bool raycast(float originX, float originY, float directionX, float directionY, float maxDistance) const = 0;
    };

}
