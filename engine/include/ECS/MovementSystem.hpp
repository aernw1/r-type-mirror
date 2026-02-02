#pragma once

#include "ISystem.hpp"

namespace RType {

    namespace ECS {

        class MovementSystem : public ISystem {
        public:
            MovementSystem() = default;
            ~MovementSystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "MovementSystem"; }
        };

    }

}
