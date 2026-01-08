/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BossAttackSystem - Handles boss attack patterns
*/

#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"
#include <cmath>

namespace RType {
    namespace ECS {

        class BossAttackSystem : public ISystem {
        public:
            BossAttackSystem() = default;
            ~BossAttackSystem() override = default;

            const char* GetName() const override { return "BossAttackSystem"; }

            void Update(Registry& registry, float deltaTime) override;

        private:
            void CreateFanSpray(Registry& registry, Entity bossEntity, float bossX, float bossY);
            void CreateBossBullet(Registry& registry, float x, float y, float angle, float speed);
        };

    }
}
