/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CollisionDetectionSystem implementation
*/

#include "ECS/CollisionDetectionSystem.hpp"
#include <algorithm>
#include <cmath>

namespace RType {
    namespace ECS {

        void CollisionDetectionSystem::Update(Registry& registry, float deltaTime) {
            (void)deltaTime;
            ClearCollisionEvents(registry);

            auto entities = GetCollidableEntities(registry);

            for (size_t i = 0; i < entities.size(); ++i) {
                for (size_t j = i + 1; j < entities.size(); ++j) {
                    Entity a = entities[i];
                    Entity b = entities[j];

                    if (!ShouldCollide(registry, a, b)) {
                        continue;
                    }

                    if (CheckCollision(registry, a, b)) {
                        registry.AddComponent<CollisionEvent>(a, CollisionEvent(b));
                        registry.AddComponent<CollisionEvent>(b, CollisionEvent(a));
                    }
                }
            }
        }

        void CollisionDetectionSystem::ClearCollisionEvents(Registry& registry) {
            auto entitiesWithEvents = registry.GetEntitiesWithComponent<CollisionEvent>();
            for (auto entity : entitiesWithEvents) {
                if (registry.IsEntityAlive(entity)) {
                    registry.RemoveComponent<CollisionEvent>(entity);
                }
            }
        }

        std::vector<Entity> CollisionDetectionSystem::GetCollidableEntities(Registry& registry) {
            std::vector<Entity> collidableEntities;

            auto allEntities = registry.GetEntitiesWithComponent<Position>();

            for (auto entity : allEntities) {
                bool hasCircleCollider = registry.HasComponent<CircleCollider>(entity);
                bool hasBoxCollider = registry.HasComponent<BoxCollider>(entity);

                if (hasCircleCollider || hasBoxCollider) {
                    collidableEntities.push_back(entity);
                }
            }

            return collidableEntities;
        }

        bool CollisionDetectionSystem::ShouldCollide(Registry& registry, Entity a, Entity b) {
            if (!registry.HasComponent<CollisionLayer>(a) || !registry.HasComponent<CollisionLayer>(b)) {
                return true;
            }
            const auto& layerA = registry.GetComponent<CollisionLayer>(a);
            const auto& layerB = registry.GetComponent<CollisionLayer>(b);

            bool aCanCollideWithB = (layerA.mask & layerB.layer) != 0;
            bool bCanCollideWithA = (layerB.mask & layerA.layer) != 0;

            return aCanCollideWithB && bCanCollideWithA;
        }

        bool CollisionDetectionSystem::CheckCollision(Registry& registry, Entity a, Entity b) {
            if (!registry.HasComponent<Position>(a) || !registry.HasComponent<Position>(b)) {
                return false;
            }

            const auto& posA = registry.GetComponent<Position>(a);
            const auto& posB = registry.GetComponent<Position>(b);

            bool aHasCircle = registry.HasComponent<CircleCollider>(a);
            bool bHasCircle = registry.HasComponent<CircleCollider>(b);
            bool aHasBox = registry.HasComponent<BoxCollider>(a);
            bool bHasBox = registry.HasComponent<BoxCollider>(b);

            if (aHasCircle && bHasCircle) {
                const auto& circleA = registry.GetComponent<CircleCollider>(a);
                const auto& circleB = registry.GetComponent<CircleCollider>(b);
                return CheckCircleCircle(posA.x, posA.y, circleA.radius,
                                         posB.x, posB.y, circleB.radius);
            }

            if (aHasBox && bHasBox) {
                const auto& boxA = registry.GetComponent<BoxCollider>(a);
                const auto& boxB = registry.GetComponent<BoxCollider>(b);
                return CheckAABB(posA.x, posA.y, boxA.width, boxA.height,
                                 posB.x, posB.y, boxB.width, boxB.height);
            }

            if (aHasCircle && bHasBox) {
                const auto& circle = registry.GetComponent<CircleCollider>(a);
                const auto& box = registry.GetComponent<BoxCollider>(b);
                return CheckCircleAABB(posA.x, posA.y, circle.radius,
                                       posB.x, posB.y, box.width, box.height);
            }

            if (aHasBox && bHasCircle) {
                const auto& box = registry.GetComponent<BoxCollider>(a);
                const auto& circle = registry.GetComponent<CircleCollider>(b);
                return CheckCircleAABB(posB.x, posB.y, circle.radius,
                                       posA.x, posA.y, box.width, box.height);
            }

            return false;
        }

        bool CollisionDetectionSystem::CheckCircleCircle(float x1, float y1, float r1,
                                                         float x2, float y2, float r2) {
            float dx = x2 - x1;
            float dy = y2 - y1;
            float radiusSum = r1 + r2;
            float distanceSquared = dx * dx + dy * dy;
            float radiusSumSquared = radiusSum * radiusSum;

            return distanceSquared <= radiusSumSquared;
        }

        bool CollisionDetectionSystem::CheckAABB(float x1, float y1, float w1, float h1,
                                                 float x2, float y2, float w2, float h2) {
            bool separated =
                (x1 + w1 <= x2) ||
                (x1 >= x2 + w2) ||
                (y1 + h1 <= y2) ||
                (y1 >= y2 + h2);

            return !separated;
        }

        bool CollisionDetectionSystem::CheckCircleAABB(float cx, float cy, float radius,
                                                       float bx, float by, float bw, float bh) {
            float closestX = std::max(bx, std::min(cx, bx + bw));
            float closestY = std::max(by, std::min(cy, by + bh));
            float dx = cx - closestX;
            float dy = cy - closestY;
            float distanceSquared = dx * dx + dy * dy;

            return distanceSquared <= (radius * radius);
        }

    }
}
