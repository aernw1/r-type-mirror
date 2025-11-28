#pragma once

#include "Registry.hpp"
#include "Component.hpp"
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"
#include <string>

namespace RType {

    namespace ECS {

        class EnemyFactory {
        public:
            static Entity CreateEnemy(Registry& registry, EnemyType type, float startX, float startY,
                                    Renderer::IRenderer* renderer);

            static float GetEnemySpeed(EnemyType type);
            static int GetEnemyHealth(EnemyType type);
            static int GetEnemyDamage(EnemyType type);
            static uint32_t GetEnemyScore(EnemyType type);

        private:
            static Math::Color GetEnemyColor(EnemyType type);
            static std::string GetEnemySpritePath(EnemyType type);
        };

    }

}

