#include "../../include/ECS/BulletCollisionSystem.hpp"
#include "../../include/ECS/ScoreSystem.hpp"
#include "../../include/ECS/Component.hpp"


namespace RType {
    namespace ECS {

        void ScoreSystem::Update(Registry& registry, float deltaTime) {
            (void)deltaTime;

            auto ennemiesKilled = registry.GetEntitiesWithComponent<EnnemyKilled>();

            if (ennemiesKilled.empty()) {
                return;
            }
            
            for (auto enemyKilled : ennemiesKilled) {
                if (!registry.IsEntityAlive(enemyKilled)) {
                        continue;
                }

                const auto& ennemyKilledComp = registry.GetComponent<EnnemyKilled>(enemyKilled);
                Entity killer = ennemyKilledComp.killedBy;
                const auto& ennemyScoreValue = registry.GetComponent<ScoreValue>(enemyKilled);

                if (killer == NULL_ENTITY || !registry.IsEntityAlive(killer) || !registry.HasComponent<ScoreValue>(killer)) {
                    registry.RemoveComponent<EnnemyKilled>(enemyKilled);
                    continue;
                }

                auto& killerScoreComp = registry.GetComponent<ScoreValue>(killer);
                killerScoreComp.points += ennemyScoreValue.points;
                
                registry.RemoveComponent<EnnemyKilled>(enemyKilled);
                continue;
            }
        }
    }
}