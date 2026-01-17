/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameState - Level transition logic
*/

#include "../../include/GameState.hpp"
#include "Core/Logger.hpp"
#include <unordered_set>
#include <algorithm>

namespace RType {
    namespace Client {

        void InGameState::UpdateLevelTransition(float dt) {
            if (m_levelProgress.transitionPhase == TransitionPhase::NONE) {
                return;
            }

            m_levelProgress.transitionTimer += dt;

            switch (m_levelProgress.transitionPhase) {
                case TransitionPhase::FADE_OUT: {
                    const float FADE_DURATION = 1.0f;
                    m_levelProgress.fadeAlpha = std::min(1.0f, m_levelProgress.transitionTimer / FADE_DURATION);

                    if (m_levelProgress.transitionTimer >= FADE_DURATION) {
                        m_levelProgress.transitionPhase = TransitionPhase::LOADING;
                        m_levelProgress.transitionTimer = 0.0f;
                        Core::Logger::Info("[Transition] FADE_OUT complete, entering LOADING");
                    }
                    break;
                }

                case TransitionPhase::LOADING: {
                    const float LOADING_DURATION = 2.0f;

                    if (m_levelProgress.transitionTimer >= LOADING_DURATION) {
                        if (m_levelProgress.currentLevelNumber >= m_levelProgress.totalLevels) {
                            m_levelProgress.allLevelsComplete = true;
                            m_levelProgress.transitionPhase = TransitionPhase::NONE;
                            m_levelProgress.fadeAlpha = 0.0f;
                            Core::Logger::Info("[Transition] ALL LEVELS COMPLETE! Victory!");
                        } else {
                            LoadNextLevel();

                            m_levelProgress.transitionPhase = TransitionPhase::FADE_IN;
                            m_levelProgress.transitionTimer = 0.0f;
                            Core::Logger::Info("[Transition] LOADING complete, entering FADE_IN");
                        }
                    }
                    break;
                }

                case TransitionPhase::FADE_IN: {
                    const float FADE_DURATION = 1.0f;
                    m_levelProgress.fadeAlpha = std::max(0.0f, 1.0f - (m_levelProgress.transitionTimer / FADE_DURATION));

                    if (m_levelProgress.transitionTimer >= FADE_DURATION) {
                        m_levelProgress.transitionPhase = TransitionPhase::NONE;
                        m_levelProgress.fadeAlpha = 0.0f;
                        m_levelProgress.levelComplete = false;
                        m_levelProgress.currentLevelNumber = m_levelProgress.nextLevelNumber;
                        Core::Logger::Info("[Transition] FADE_IN complete, transition finished");
                    }
                    break;
                }

                default:
                    break;
            }
        }

        void InGameState::LoadNextLevel() {
            Core::Logger::Info("[Transition] Loading level {}...", m_levelProgress.nextLevelNumber);

            std::unordered_set<ECS::Entity> hudEntities;

            for (size_t i = 0; i < MAX_PLAYERS; i++) {
                if (m_playersHUD[i].scoreEntity != ECS::NULL_ENTITY) {
                    hudEntities.insert(m_playersHUD[i].scoreEntity);
                }
                if (m_playersHUD[i].powerupSpreadEntity != ECS::NULL_ENTITY) {
                    hudEntities.insert(m_playersHUD[i].powerupSpreadEntity);
                }
                if (m_playersHUD[i].powerupLaserEntity != ECS::NULL_ENTITY) {
                    hudEntities.insert(m_playersHUD[i].powerupLaserEntity);
                }
                if (m_playersHUD[i].powerupSpeedEntity != ECS::NULL_ENTITY) {
                    hudEntities.insert(m_playersHUD[i].powerupSpeedEntity);
                }
                if (m_playersHUD[i].powerupShieldEntity != ECS::NULL_ENTITY) {
                    hudEntities.insert(m_playersHUD[i].powerupShieldEntity);
                }
            }

            if (m_bossHealthBar.titleEntity != ECS::NULL_ENTITY) {
                hudEntities.insert(m_bossHealthBar.titleEntity);
            }
            if (m_bossHealthBar.barBackgroundEntity != ECS::NULL_ENTITY) {
                hudEntities.insert(m_bossHealthBar.barBackgroundEntity);
            }
            if (m_bossHealthBar.barForegroundEntity != ECS::NULL_ENTITY) {
                hudEntities.insert(m_bossHealthBar.barForegroundEntity);
            }

            for (const auto& pair : m_playerNameLabels) {
                if (pair.second != ECS::NULL_ENTITY) {
                    hudEntities.insert(pair.second);
                }
            }

            if (m_gameOverTitleEntity != ECS::NULL_ENTITY) {
                hudEntities.insert(m_gameOverTitleEntity);
            }
            if (m_gameOverScoreEntity != ECS::NULL_ENTITY) {
                hudEntities.insert(m_gameOverScoreEntity);
            }
            if (m_gameOverHintEntity != ECS::NULL_ENTITY) {
                hudEntities.insert(m_gameOverHintEntity);
            }

            if (m_victoryTitleEntity != ECS::NULL_ENTITY) {
                hudEntities.insert(m_victoryTitleEntity);
            }
            if (m_victoryScoreEntity != ECS::NULL_ENTITY) {
                hudEntities.insert(m_victoryScoreEntity);
            }
            if (m_victoryHintEntity != ECS::NULL_ENTITY) {
                hudEntities.insert(m_victoryHintEntity);
            }

            std::vector<ECS::Entity> toDestroy;

            auto positionEntities = m_registry.GetEntitiesWithComponent<ECS::Position>();
            for (auto entity : positionEntities) {
                if (entity == m_localPlayerEntity) {
                    continue;
                }
                if (hudEntities.find(entity) != hudEntities.end()) {
                    continue;
                }
                toDestroy.push_back(entity);
            }

            auto colliderEntities = m_registry.GetEntitiesWithComponent<ECS::BoxCollider>();
            for (auto entity : colliderEntities) {
                if (entity == m_localPlayerEntity) {
                    continue;
                }
                if (hudEntities.find(entity) != hudEntities.end()) {
                    continue;
                }
                if (std::find(toDestroy.begin(), toDestroy.end(), entity) == toDestroy.end()) {
                    toDestroy.push_back(entity);
                }
            }

            auto drawableEntities = m_registry.GetEntitiesWithComponent<ECS::Drawable>();
            for (auto entity : drawableEntities) {
                if (entity == m_localPlayerEntity) {
                    continue;
                }
                if (hudEntities.find(entity) != hudEntities.end()) {
                    continue;
                }
                if (std::find(toDestroy.begin(), toDestroy.end(), entity) == toDestroy.end()) {
                    toDestroy.push_back(entity);
                }
            }

            for (auto entity : toDestroy) {
                if (m_registry.IsEntityAlive(entity)) {
                    m_registry.DestroyEntity(entity);
                }
            }

            Core::Logger::Info("[Transition] Destroyed {} entities (preserved player + {} HUD entities)",
                              toDestroy.size(), hudEntities.size());

            std::vector<uint32_t> networkIdsToRemove;
            for (const auto& [networkId, localEntity] : m_networkEntityMap) {
                if (!m_registry.IsEntityAlive(localEntity)) {
                    networkIdsToRemove.push_back(networkId);
                }
            }
            for (uint32_t networkId : networkIdsToRemove) {
                m_networkEntityMap.erase(networkId);
            }
            Core::Logger::Info("[Transition] Cleaned network map: removed {} dead entities, kept {} alive",
                              networkIdsToRemove.size(), m_networkEntityMap.size());

            m_obstacleColliderEntities.clear();
            m_obstacleSpriteEntities.clear();
            m_obstacleIdToCollider.clear();
            m_backgroundEntities.clear();

            m_serverScrollOffset = 0.0f;
            m_localScrollOffset = 0.0f;

            m_bossHealthBar.bossNetworkId = 0;
            m_bossHealthBar.currentHealth = 0;
            m_bossHealthBar.maxHealth = 0;
            m_bossHealthBar.active = false;

            m_bossWarningActive = false;
            m_bossWarningTriggered = false;
            m_bossWarningTimer = 0.0f;

            std::string levelPath = "assets/levels/level" + std::to_string(m_levelProgress.nextLevelNumber) + ".json";
            m_currentLevelPath = levelPath;

            try {
                m_levelData = ECS::LevelLoader::LoadFromFile(levelPath);
                m_levelAssets = ECS::LevelLoader::LoadAssets(m_levelData, m_renderer.get());

                m_levelEntities = ECS::LevelLoader::CreateEntities(m_registry, m_levelData, m_levelAssets, m_renderer.get());
                m_backgroundEntities = m_levelEntities.backgrounds;
                m_obstacleSpriteEntities = m_levelEntities.obstacleVisuals;
                m_obstacleColliderEntities = m_levelEntities.obstacleColliders;

                Core::Logger::Info("[Transition] Level {} loaded: {} textures, {} sprites",
                                  m_levelProgress.nextLevelNumber,
                                  m_levelAssets.textures.size(),
                                  m_levelAssets.sprites.size());

                for (auto collider : m_obstacleColliderEntities) {
                    if (!m_registry.IsEntityAlive(collider) ||
                        !m_registry.HasComponent<ECS::ObstacleMetadata>(collider)) {
                        continue;
                    }
                    const auto& metadata = m_registry.GetComponent<ECS::ObstacleMetadata>(collider);
                    m_obstacleIdToCollider[metadata.uniqueId] = collider;

                    if (metadata.visualEntity != ECS::NULL_ENTITY &&
                        m_registry.IsEntityAlive(metadata.visualEntity) &&
                        m_registry.HasComponent<ECS::Position>(metadata.visualEntity) &&
                        m_registry.HasComponent<ECS::Position>(collider)) {

                        const auto& visualPos = m_registry.GetComponent<ECS::Position>(metadata.visualEntity);
                        auto& colliderPos = m_registry.GetComponent<ECS::Position>(collider);

                        colliderPos.x = visualPos.x + metadata.offsetX;
                        colliderPos.y = visualPos.y + metadata.offsetY;
                    }
                }

                Core::Logger::Info("[Transition] Rebuilt obstacle mapping: {} obstacles tracked",
                                  m_obstacleIdToCollider.size());

            } catch (const std::exception& e) {
                Core::Logger::Error("[Transition] Failed to load level {}: {}", m_levelProgress.nextLevelNumber, e.what());
            }

            if (m_context.audio) {
                if (m_gameMusic != Audio::INVALID_MUSIC_ID) {
                    m_context.audio->StopMusic(m_gameMusic);
                    m_context.audio->UnloadMusic(m_gameMusic);
                }

                std::string musicPath = "assets/sounds/stage1.flac";
                if (m_levelProgress.nextLevelNumber == 2) {
                    musicPath = "assets/sounds/stage2.flac";
                } else if (m_levelProgress.nextLevelNumber == 3) {
                    musicPath = "assets/sounds/stage3.flac";
                }

                m_gameMusic = m_context.audio->LoadMusic(musicPath);
                if (m_gameMusic == Audio::INVALID_MUSIC_ID) {
                    m_gameMusic = m_context.audio->LoadMusic("../" + musicPath);
                }

                if (m_gameMusic != Audio::INVALID_MUSIC_ID) {
                    Audio::PlaybackOptions opts;
                    opts.loop = true;
                    opts.volume = 0.35f;
                    m_context.audio->PlayMusic(m_gameMusic, opts);
                    m_gameMusicPlaying = true;
                }
            }
        }

    }
}
