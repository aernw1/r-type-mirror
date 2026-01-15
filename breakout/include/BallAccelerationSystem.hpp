#pragma once

#include "ECS/ISystem.hpp"

namespace Breakout {
    namespace ECS {

        class BallAccelerationSystem : public RType::ECS::ISystem {
        public:
            BallAccelerationSystem() = default;
            ~BallAccelerationSystem() override = default;

            const char* GetName() const override { return "BallAccelerationSystem"; }
            void Update(RType::ECS::Registry& registry, float deltaTime) override;

        private:
            float m_elapsedTime = 0.0f;
            static constexpr float ACCELERATION_RATE = 2.5f;
            static constexpr float MAX_SPEED_MULTIPLIER = 2.5f;
        };

    }
}
