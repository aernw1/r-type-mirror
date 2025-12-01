#pragma once

#include "ISystem.hpp"

namespace RType {
    namespace ECS {

        class CollisionSystem : public ISystem {
        public:
            CollisionSystem() = default;
            ~CollisionSystem() override = default;

            const char* GetName() const override { return "CollisionSystem"; }

            void Update(Registry& registry, float deltaTime) override;

            static bool CheckCollision(Registry& registry, Entity a, Entity b);
        private:
            static bool CheckAABB(float x1, float y1, float w1, float h1,
                                  float x2, float y2, float w2, float h2);
        };
    }
}
