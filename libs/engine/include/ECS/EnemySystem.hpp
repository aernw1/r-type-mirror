#pragma once

#include "ISystem.hpp"
#include "Renderer/IRenderer.hpp"
#include <random>

namespace RType {

    namespace ECS {

        class EnemySystem : public ISystem {
        public:
            explicit EnemySystem(Renderer::IRenderer* renderer = nullptr);
            ~EnemySystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "EnemySystem"; }

            void SpawnRandomEnemy(Registry& registry, float screenWidth, float screenHeight);
            static void DestroyEnemiesOffScreen(Registry& registry, float screenWidth);
            static void ApplyMovementPattern(Registry& registry, Entity enemy, float /* deltaTime */);

        private:
            Renderer::IRenderer* m_renderer;
            float m_spawnTimer = 0.0f;
            float m_spawnInterval = 3.0f;
            std::mt19937 m_rng;
        };

    }

}

