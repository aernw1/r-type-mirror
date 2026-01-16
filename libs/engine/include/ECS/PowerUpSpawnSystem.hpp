/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PowerUpSpawnSystem
*/

#pragma once

#include "ISystem.hpp"
#include "Renderer/IRenderer.hpp"
#include <random>

namespace RType {
    namespace ECS {

        class EffectFactory;

        class PowerUpSpawnSystem : public ISystem {
        public:
            explicit PowerUpSpawnSystem(
                Renderer::IRenderer* renderer = nullptr,
                float screenWidth = 1280.0f,
                float screenHeight = 720.0f
            );
            ~PowerUpSpawnSystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "PowerUpSpawnSystem"; }

            void SpawnRandomPowerUp(Registry& registry);
            static void DestroyPowerUpsOffScreen(Registry& registry);

            void SetSpawnInterval(float interval) { m_spawnInterval = interval; }
            void SetEffectFactory(const EffectFactory* effectFactory) { m_effectFactory = effectFactory; }

        private:
            Renderer::IRenderer* m_renderer;
            float m_screenWidth;
            float m_screenHeight;
            float m_spawnTimer = 0.0f;
            float m_spawnInterval = 5.0f;
            const EffectFactory* m_effectFactory = nullptr;
            std::mt19937 m_rng;
        };

    }
}
