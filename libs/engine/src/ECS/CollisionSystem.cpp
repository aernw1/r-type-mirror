#include "ECS/CollisionSystem.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"

namespace RType {
    namespace ECS {

        void CollisionSystem::Update(Registry& registry, float deltaTime) {
            (void)registry;
            (void)deltaTime;
        }

        bool CollisionSystem::CheckCollision(Registry& registry, Entity a, Entity b) {
            if (!registry.HasComponent<Position>(a) ||
                !registry.HasComponent<BoxCollider>(a)) {
                return false;
            }

            if (!registry.HasComponent<Position>(b) ||
                !registry.HasComponent<BoxCollider>(b)) {
                return false;
            }

            const auto& posA = registry.GetComponent<Position>(a);
            const auto& colA = registry.GetComponent<BoxCollider>(a);
            const auto& posB = registry.GetComponent<Position>(b);
            const auto& colB = registry.GetComponent<BoxCollider>(b);

            return CheckAABB(
                posA.x, posA.y, colA.width, colA.height,
                posB.x, posB.y, colB.width, colB.height
            );
        }

        bool CollisionSystem::CheckAABB(float x1, float y1, float w1, float h1,
            float x2, float y2, float w2, float h2) {

            bool separated =
                (x1 + w1 < x2) ||
                (x1 > x2 + w2) ||
                (y1 + h1 < y2) ||
                (y1 > y2 + h2);

            return !separated;
        }
    }
}
