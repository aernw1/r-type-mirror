/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BulletCollisionResponseSystem - Handles bullet collision responses
*/

#pragma once

#include "ECS/ISystem.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"

namespace RType {
    namespace ECS {

        class EffectFactory;

        class BulletCollisionResponseSystem : public ISystem {
        public:
            BulletCollisionResponseSystem() = default;
            explicit BulletCollisionResponseSystem(EffectFactory* effectFactory) 
                : m_effectFactory(effectFactory) {}
            ~BulletCollisionResponseSystem() override = default;

            const char* GetName() const override { return "BulletCollisionResponseSystem"; }
            void Update(Registry& registry, float deltaTime) override;
            
            void SetEffectFactory(EffectFactory* effectFactory) { m_effectFactory = effectFactory; }

        private:
            EffectFactory* m_effectFactory = nullptr;
        };

    }
}
