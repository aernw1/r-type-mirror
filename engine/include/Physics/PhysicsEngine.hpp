#pragma once

#include <memory>
#include <utility>
#include "Core/Module.hpp"
#include "Core/Logger.hpp"

namespace Physics {

    class IPhysicsWorld;

    class PhysicsEngine : public RType::Core::IModule {
    public:
        virtual ~PhysicsEngine() = default;

        virtual const char* GetName() const override = 0;
        virtual RType::Core::ModulePriority GetPriority() const override = 0;
        virtual bool Initialize(RType::Core::Engine* engine) override = 0;
        virtual void Shutdown() override = 0;
        virtual void Update(float deltaTime) override = 0;

        virtual std::shared_ptr<IPhysicsWorld> createWorld() = 0;
        virtual void setGravity(float gx, float gy) = 0;
        virtual std::pair<float, float> getGravity() const = 0;
    };

}
