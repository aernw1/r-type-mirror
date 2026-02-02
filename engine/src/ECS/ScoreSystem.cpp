#include "../../include/ECS/ScoreSystem.hpp"
#include "../../include/ECS/Component.hpp"


namespace RType {
    namespace ECS {

        void ScoreSystem::Update(Registry& registry, float deltaTime) {
            auto enemiesKilled = registry.GetEntitiesWithComponent<EnemyKilled>();

            if (enemiesKilled.empty()) {
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

            //  +10 toutes les 10 secondes
            constexpr float SCORE_INTERVAL_SECONDS = 10.0f;
            constexpr uint32_t SCORE_PER_INTERVAL = 10;

            auto timedEntities = registry.GetEntitiesWithComponent<ScoreTimer>();
            for (auto entity : timedEntities) {
                if (!registry.IsEntityAlive(entity) ||
                    !registry.HasComponent<ScoreTimer>(entity) ||
                    !registry.HasComponent<ScoreValue>(entity) ||
                    !registry.HasComponent<Player>(entity)) {
                    continue;
                }

                auto& timer = registry.GetComponent<ScoreTimer>(entity);
                timer.elapsed += deltaTime;

                if (timer.elapsed < SCORE_INTERVAL_SECONDS) {
                    continue;
                }

                uint32_t intervals = static_cast<uint32_t>(timer.elapsed / SCORE_INTERVAL_SECONDS);
                timer.elapsed -= static_cast<float>(intervals) * SCORE_INTERVAL_SECONDS;

                auto& score = registry.GetComponent<ScoreValue>(entity);
                score.points += intervals * SCORE_PER_INTERVAL;
            }
        }
    }
}