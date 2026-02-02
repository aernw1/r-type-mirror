/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** PhysicsCollisionSystem implementation
*/

#include "ECS/PhysicsCollisionSystem.hpp"
#include <cmath>
#include <algorithm>

namespace RType {
    namespace ECS {

        void PhysicsCollisionSystem::Update(Registry& registry, float deltaTime) {
            (void)deltaTime;
            
            // Iterate over all entities with CollisionEvent
            auto entities = registry.GetEntitiesWithComponent<CollisionEvent>();

            for (Entity entity : entities) {
                if (!registry.HasComponent<Rigidbody2D>(entity)) {
                    continue; // Skip non-physics entities
                }

                auto& collision = registry.GetComponent<CollisionEvent>(entity);
                Entity other = collision.other;

                if (!registry.IsEntityAlive(other)) {
                    continue;
                }

                // Check for Trigger
                if (registry.HasComponent<CollisionLayer>(entity)) {
                    const auto& layer = registry.GetComponent<CollisionLayer>(entity);
                    if (layer.layer & CollisionLayers::TRIGGER) continue;
                }
                if (registry.HasComponent<CollisionLayer>(other)) {
                    const auto& layer = registry.GetComponent<CollisionLayer>(other);
                    if (layer.layer & CollisionLayers::TRIGGER) continue;
                }

                ResolveCollision(registry, entity, other);
            }
        }

        void PhysicsCollisionSystem::ResolveCollision(Registry& registry, Entity a, Entity b) {
            if (!registry.HasComponent<Position>(a) || !registry.HasComponent<Position>(b)) return;
            // 'a' has Rigidbody2D (checked in Loop), 'b' might not (e.g., static obstacle)

            auto& posA = registry.GetComponent<Position>(a);
            auto& rbA = registry.GetComponent<Rigidbody2D>(a);
            
            bool bHasRb = registry.HasComponent<Rigidbody2D>(b);
            
            // Calculate relative velocity
            Math::Vector2 velA = {0.0f, 0.0f};
            if (registry.HasComponent<Velocity>(a)) {
                const auto& v = registry.GetComponent<Velocity>(a);
                velA = {v.dx, v.dy};
            }

            Math::Vector2 velB = {0.0f, 0.0f};
            if (registry.HasComponent<Velocity>(b)) {
                const auto& v = registry.GetComponent<Velocity>(b);
                velB = {v.dx, v.dy};
            }

            Math::Vector2 relativeVel = {velB.x - velA.x, velB.y - velA.y};

            // Calculate Normal and Depth
            float depth = 0.0f;
            Math::Vector2 normal = GetCollisionNormalAndDepth(registry, a, b, depth);

            // 1. Positional Correction (prevent sinking)
            const float percent = 0.8f; // Penetration percentage to correct
            const float slop = 0.01f;   // Penetration allowance
            float correctionMag = std::max(depth - slop, 0.0f) * percent;
            Math::Vector2 correction = {normal.x * correctionMag, normal.y * correctionMag};

            if (rbA.isKinematic) {
                // Do nothing positionally if kinematic? 
                // Usually kinematic objects are infinite mass, so they push others but don't move.
                // If A is kinematic, we can't move it.
            } else {
                float invMassA = (rbA.mass == 0.0f) ? 0.0f : 1.0f / rbA.mass;
                float invMassB = 0.0f;
                if (bHasRb) {
                    const auto& rbB = registry.GetComponent<Rigidbody2D>(b);
                    invMassB = (rbB.mass == 0.0f || rbB.isKinematic) ? 0.0f : 1.0f / rbB.mass;
                }

                float totalInvMass = invMassA + invMassB;
                if (totalInvMass > 0.0f) {
                    posA.x -= correction.x * (invMassA / totalInvMass);
                    posA.y -= correction.y * (invMassA / totalInvMass);
                    
                    if (bHasRb && registry.HasComponent<Position>(b)) {
                        auto& posB_ref = registry.GetComponent<Position>(b);
                         posB_ref.x += correction.x * (invMassB / totalInvMass);
                         posB_ref.y += correction.y * (invMassB / totalInvMass);
                    }
                }
            }

            // 2. Velocity Impulse Resolution
            float velAlongNormal = relativeVel.x * normal.x + relativeVel.y * normal.y;

            // Do not resolve if velocities are separating
            if (velAlongNormal > 0) return;

            // restitution (bounciness) - take max of both
            float restitution = 0.0f;
            if (registry.HasComponent<PhysicsMaterial>(a)) restitution = std::max(restitution, registry.GetComponent<PhysicsMaterial>(a).bounciness);
            if (registry.HasComponent<PhysicsMaterial>(b)) restitution = std::max(restitution, registry.GetComponent<PhysicsMaterial>(b).bounciness);

            float j = -(1.0f + restitution) * velAlongNormal;
            
            float invMassA = (rbA.isKinematic) ? 0.0f : ((rbA.mass == 0.0f) ? 0.0f : 1.0f / rbA.mass);
             float invMassB = 0.0f;
            if (bHasRb) {
                 const auto& rbB = registry.GetComponent<Rigidbody2D>(b);
                 invMassB = (rbB.isKinematic) ? 0.0f : ((rbB.mass == 0.0f) ? 0.0f : 1.0f / rbB.mass);
            }
             
            float totalInvMass = invMassA + invMassB;
            if (totalInvMass == 0.0f) return;
            
            j /= totalInvMass;

            Math::Vector2 impulse = {normal.x * j, normal.y * j};

            if (!rbA.isKinematic && registry.HasComponent<Velocity>(a)) {
                auto& vA = registry.GetComponent<Velocity>(a);
                vA.dx -= impulse.x * invMassA;
                vA.dy -= impulse.y * invMassA;
            }

            if (bHasRb && !registry.GetComponent<Rigidbody2D>(b).isKinematic && registry.HasComponent<Velocity>(b)) {
                auto& vB = registry.GetComponent<Velocity>(b);
                vB.dx += impulse.x * invMassB;
                vB.dy += impulse.y * invMassB;
            }
        }
        
        Math::Vector2 PhysicsCollisionSystem::GetCollisionNormalAndDepth(Registry& registry, Entity a, Entity b, float& depth) {
             const auto& posA = registry.GetComponent<Position>(a);
             const auto& posB = registry.GetComponent<Position>(b);
             
             bool aCircle = registry.HasComponent<CircleCollider>(a);
             bool bCircle = registry.HasComponent<CircleCollider>(b);
             bool aBox = registry.HasComponent<BoxCollider>(a);
             bool bBox = registry.HasComponent<BoxCollider>(b);

             // Circle vs Circle
             if (aCircle && bCircle) {
                 float rA = registry.GetComponent<CircleCollider>(a).radius;
                 float rB = registry.GetComponent<CircleCollider>(b).radius;
                 float dx = posB.x - posA.x;
                 float dy = posB.y - posA.y;
                 float dist = std::sqrt(dx*dx + dy*dy);
                 
                 if (dist == 0.0f) {
                     depth = rA + rB;
                     return {0.0f, -1.0f};
                 }
                 
                 depth = (rA + rB) - dist;
                 return {dx / dist, dy / dist};
             }
             
             // AABB vs AABB
             if (aBox && bBox) {
                 const auto& boxA = registry.GetComponent<BoxCollider>(a);
                 const auto& boxB = registry.GetComponent<BoxCollider>(b);
                 
                 float halfWA = boxA.width / 2.0f;
                 float halfHA = boxA.height / 2.0f;
                 float halfWB = boxB.width / 2.0f;
                 float halfHB = boxB.height / 2.0f;
                 
                 float centerAX = posA.x + halfWA;
                 float centerAY = posA.y + halfHA;
                 float centerBX = posB.x + halfWB;
                 float centerBY = posB.y + halfHB;
                 
                 float dx = centerBX - centerAX;
                 float dy = centerBY - centerAY;
                 
                 float xOverlap = (halfWA + halfWB) - std::abs(dx);
                 float yOverlap = (halfHA + halfHB) - std::abs(dy);
                 
                 if (xOverlap < yOverlap) {
                     depth = xOverlap;
                     return {dx < 0 ? -1.0f : 1.0f, 0.0f};
                 } else {
                     depth = yOverlap;
                     return {0.0f, dy < 0 ? -1.0f : 1.0f};
                 }
             }

             // Circle vs AABB (Simplified: treat as AABB vs AABB for normal)
             // Ideally this should be more precise but this suffices for basic platforming
             if (aCircle && bBox) {
                 // treat circle as box
                 float r = registry.GetComponent<CircleCollider>(a).radius;
                 // recursion with mock box
                 // ... for simplicity returning placeholder normal pointing to B
                 float dx = posB.x - posA.x;
                 float dy = posB.y - posA.y;
                 float dist = std::sqrt(dx*dx + dy*dy);
                   if (dist > 0) return {dx/dist, dy/dist};
             }
             
             depth = 0.0f;
             return {0.0f, 1.0f};
        }

    }
}
