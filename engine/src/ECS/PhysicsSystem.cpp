/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** PhysicsSystem implementation
*/

#include "ECS/PhysicsSystem.hpp"
#include "ECS/Components/Physics.hpp"
#include "ECS/Components/Transform.hpp"

namespace RType {
    namespace ECS {

        void PhysicsSystem::Update(Registry& registry, float deltaTime) {
            auto entities = registry.GetEntitiesWithComponent<Rigidbody2D>();

            for (Entity entity : entities) {
                if (!registry.HasComponent<Position>(entity) || !registry.HasComponent<Velocity>(entity)) {
                    continue;
                }

                auto& rb = registry.GetComponent<Rigidbody2D>(entity);
                auto& pos = registry.GetComponent<Position>(entity);
                auto& vel = registry.GetComponent<Velocity>(entity);

                // Skip static or kinematic bodies only if they shouldn't move via physics
                // Kinematic bodies might move via setting velocity directly, so we typically 
                // still integrate velocity but don't apply forces like gravity.
                // However, often kinematic means "moved by script only", so we might skip gravity.
                
                if (rb.isKinematic) {
                    // Kinematic bodies just integrate velocity -> position
                    pos.x += vel.dx * deltaTime;
                    pos.y += vel.dy * deltaTime;
                    continue;
                }

                // Apply Gravity
                if (rb.useGravity) {
                    vel.dx += m_gravity.x * rb.gravityScale * deltaTime;
                    vel.dy += m_gravity.y * rb.gravityScale * deltaTime;
                }

                // Apply Drag (Linear approximation: F_drag = -drag * v)
                if (rb.drag > 0.0f) {
                    float dragFactor = 1.0f - (rb.drag * deltaTime);
                    if (dragFactor < 0.0f) dragFactor = 0.0f;
                    vel.dx *= dragFactor;
                    vel.dy *= dragFactor;
                }

                // Integrate Velocity -> Position (Euler integration)
                pos.x += vel.dx * deltaTime;
                pos.y += vel.dy * deltaTime;
            }
        }

    }
}
