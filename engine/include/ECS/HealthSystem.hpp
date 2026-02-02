#pragma once

#include "ISystem.hpp"

namespace RType {

    namespace ECS {

        class HealthSystem : public ISystem {
        public:
            HealthSystem() = default;
            ~HealthSystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "HealthSystem"; }
        private:
            void checkAndDestroyDeadEntities(Registry& registry);
        };

    }

}
