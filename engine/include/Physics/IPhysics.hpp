#pragma once

#include <memory>
#include <utility>
#include "Core/Module.hpp"
#include "Core/Logger.hpp"
#include "Core/Engine.hpp"

namespace Physics {

    struct Vector2 {
        float x = 0.0f;
        float y = 0.0f;

        Vector2() = default;
        Vector2(float x, float y)
            : x(x), y(y) {}
    };

    struct BodyDef {
        Vector2 position{0.0f, 0.0f};
        Vector2 velocity{0.0f, 0.0f};
        float mass = 1.0f;
        bool isStatic = false;
    };

    class Body {
    public:
        virtual ~Body() = default;

        virtual Vector2 GetPosition() const = 0;
        virtual void SetPosition(const Vector2& position) = 0;
        virtual Vector2 GetVelocity() const = 0;
        virtual void SetVelocity(const Vector2& velocity) = 0;
    };

    class IPhysics : public RType::Core::IModule {
    public:
        virtual ~IPhysics() = default;

        virtual const char* GetName() const override = 0;
        virtual RType::Core::ModulePriority GetPriority() const override = 0;
        virtual bool Initialize(RType::Core::Engine* engine) override = 0;
        virtual void Shutdown() override = 0;
        virtual void Update(float deltaTime) override = 0;

        virtual std::shared_ptr<Body> CreateBody(const BodyDef& def) = 0;
        virtual void DestroyBody(Body* body) = 0;
        virtual void SetGravity(const Vector2& gravity) = 0;
    };

}
