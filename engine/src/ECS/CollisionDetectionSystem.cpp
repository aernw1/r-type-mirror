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

        std::optional<RaycastHit> CollisionDetectionSystem::Raycast(Registry& registry, 
                                                                  const Math::Vector2& origin, 
                                                                  const Math::Vector2& direction, 
                                                                  float maxDistance, 
                                                                  uint16_t layerMask) {
            std::optional<RaycastHit> closestHit;
            auto entities = GetCollidableEntities(registry); // reusing existing helper

            for (Entity entity : entities) {
                // Layer Check
                if (registry.HasComponent<CollisionLayer>(entity)) {
                    auto& layer = registry.GetComponent<CollisionLayer>(entity);
                    if ((layer.layer & layerMask) == 0) {
                        continue;
                    }
                }

                if (!registry.HasComponent<Position>(entity)) continue;
                const auto& pos = registry.GetComponent<Position>(entity);

                // Check Circle Collider
                if (registry.HasComponent<CircleCollider>(entity)) {
                    const auto& circle = registry.GetComponent<CircleCollider>(entity);
                    
                    // Simple Ray vs Circle intersection
                    float cx = pos.x;
                    float cy = pos.y;
                    float radius = circle.radius;

                    float dx = cx - origin.x;
                    float dy = cy - origin.y;
                    float t = dx * direction.x + dy * direction.y;

                    // Closest point on ray line
                    float closestX = origin.x + t * direction.x;
                    float closestY = origin.y + t * direction.y;

                    float distSq = (cx - closestX) * (cx - closestX) + (cy - closestY) * (cy - closestY);
                    
                    if (distSq <= radius * radius) {
                        // Intersection
                        float distToCenter = std::sqrt((dx * dx) + (dy * dy));
                        // Approximation: hit point is on the surface
                        // Exact calculation requires resolving the quadratic equation
                        float range = std::sqrt(radius * radius - distSq);
                        float t0 = t - range; // First intersection point
                        
                        if (t0 >= 0 && t0 <= maxDistance) {
                            if (!closestHit || t0 < closestHit->distance) {
                                RaycastHit hit;
                                hit.entity = entity;
                                hit.distance = t0;
                                hit.point = {origin.x + t0 * direction.x, origin.y + t0 * direction.y};
                                // Normal calculation (point - center) normalized
                                float nx = hit.point.x - cx;
                                float ny = hit.point.y - cy;
                                float len = std::sqrt(nx * nx + ny * ny);
                                if (len > 0) {
                                    hit.normal = {nx / len, ny / len};
                                }
                                closestHit = hit;
                            }
                        }
                    }
                }
                
                // Check Box Collider
                else if (registry.HasComponent<BoxCollider>(entity)) {
                    const auto& box = registry.GetComponent<BoxCollider>(entity);
                    
                    // Ray vs AABB (Slab method)
                    float tmin = 0.0f;
                    float tmax = maxDistance;

                    // X Axis
                    float tx1 = (pos.x - origin.x) / direction.x;
                    float tx2 = (pos.x + box.width - origin.x) / direction.x;

                    float tminX = std::min(tx1, tx2);
                    float tmaxX = std::max(tx1, tx2);

                    tmin = std::max(tmin, tminX);
                    tmax = std::min(tmax, tmaxX);

                    // Y Axis
                    float ty1 = (pos.y - origin.y) / direction.y;
                    float ty2 = (pos.y + box.height - origin.y) / direction.y;

                    float tminY = std::min(ty1, ty2);
                    float tmaxY = std::max(ty1, ty2);

                    tmin = std::max(tmin, tminY);
                    tmax = std::min(tmax, tmaxY);

                    if (tmax >= tmin && tmin >= 0 && tmin <= maxDistance) {
                        if (!closestHit || tmin < closestHit->distance) {
                            RaycastHit hit;
                            hit.entity = entity;
                            hit.distance = tmin;
                            hit.point = {origin.x + tmin * direction.x, origin.y + tmin * direction.y};
                            
                            // Determine normal based on hit face
                            // Simple heuristic for AABB normal
                            if (std::abs(hit.point.x - pos.x) < 0.001f) hit.normal = {-1, 0};
                            else if (std::abs(hit.point.x - (pos.x + box.width)) < 0.001f) hit.normal = {1, 0};
                            else if (std::abs(hit.point.y - pos.y) < 0.001f) hit.normal = {0, -1};
                            else if (std::abs(hit.point.y - (pos.y + box.height)) < 0.001f) hit.normal = {0, 1};
                            
                            closestHit = hit;
                        }
                    }
                }
            }

            return closestHit;
        }

    }
}
