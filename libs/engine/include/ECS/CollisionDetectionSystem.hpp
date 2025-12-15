/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CollisionDetectionSystem - Unified collision detection system
*/

#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"
#include "Component.hpp"

namespace RType {
    namespace ECS {

        class CollisionDetectionSystem : public ISystem {
        public:
            CollisionDetectionSystem() = default;
            ~CollisionDetectionSystem() override = default;

            const char* GetName() const override { return "CollisionDetectionSystem"; }
            void Update(Registry& registry, float deltaTime) override;

            static bool CheckCollision(Registry& registry, Entity a, Entity b);
        private:
            void ClearCollisionEvents(Registry& registry);
            std::vector<Entity> GetCollidableEntities(Registry& registry);
            bool ShouldCollide(Registry& registry, Entity a, Entity b);

            static bool CheckCircleCircle(float x1, float y1, float r1,
                                          float x2, float y2, float r2);

            static bool CheckAABB(float x1, float y1, float w1, float h1,
                                  float x2, float y2, float w2, float h2);

            static bool CheckCircleAABB(float cx, float cy, float radius,
                                        float bx, float by, float bw, float bh);
        };

    }
}
