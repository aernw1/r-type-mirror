#include "../../include/ECS/BulletCollisionSystem.hpp"
#include "../../include/ECS/ScoreSystem.hpp"
#include "../../include/ECS/Component.hpp"


namespace RType {
    namespace ECS {

        void ScoreSystem::Update(Registry& registry, float deltaTime) {
            (void)deltaTime;

            auto enemiesKilled = registry.GetEntitiesWithComponent<EnemyKilled>();

            if (enemiesKilled.empty()) {
                return;
            }
            
            for (auto enemyKilled : enemiesKilled) {
                if (!registry.IsEntityAlive(enemyKilled) || !registry.HasComponent<EnemyKilled>(enemyKilled) || !registry.HasComponent<ScoreValue>(enemyKilled)) {
                    continue;
                }
                const auto& enemyKilledComp = registry.GetComponent<EnemyKilled>(enemyKilled);
                Entity killer = enemyKilledComp.killedBy;
                const auto& ennemyScoreValue = registry.GetComponent<ScoreValue>(enemyKilled);

                if (killer == NULL_ENTITY || !registry.IsEntityAlive(killer) || !registry.HasComponent<ScoreValue>(killer)) {
                    registry.RemoveComponent<EnemyKilled>(enemyKilled);
                    continue;
                }

                auto& killerScoreComp = registry.GetComponent<ScoreValue>(killer);
                killerScoreComp.points += ennemyScoreValue.points;
                
                registry.RemoveComponent<EnemyKilled>(enemyKilled);
            }
        }
    }
}