#pragma once

#include <memory>
#include <vector>
#include "Core/Module.hpp"
#include "Core/Logger.hpp"

namespace Physics {

    class RigidBody;

    class IPhysicsWorld : public RType::Core::IModule {
    public:
        virtual ~IPhysicsWorld() = default;

        virtual const char* GetName() const override = 0;
        virtual RType::Core::ModulePriority GetPriority() const override = 0;
        virtual bool Initialize(RType::Core::Engine* engine) override = 0;
        virtual void Shutdown() override = 0;
        virtual void Update(float deltaTime) override = 0;
        virtual std::shared_ptr<RigidBody> createRigidBody() = 0;
        virtual void destroyRigidBody(const std::shared_ptr<RigidBody>& body) = 0;
        virtual std::vector<std::shared_ptr<RigidBody>> getRigidBodies() const = 0;
        virtual bool raycast(float originX, float originY, float directionX, float directionY, float maxDistance) const = 0;
    };

}
