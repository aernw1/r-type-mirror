#pragma once

#include "ISystem.hpp"

namespace RType {
    namespace ECS {

        class ScoreSystem : public ISystem {
        public:
            ScoreSystem() = default;
            ~ScoreSystem() override = default;

            const char* GetName() const override { return "ScoreSystem"; }

            void Update(Registry& registry, float deltaTime) override;
        };
    }
}