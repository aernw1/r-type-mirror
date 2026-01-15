/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorFileManager - Save/Load level files implementation
*/

#include "editor/EditorFileManager.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include <stdexcept>
#include <filesystem>

namespace RType {
    namespace Client {

        EditorFileManager::EditorFileManager(EditorAssetLibrary& assets)
            : m_assets(assets), m_lastError("") {
        }

        bool EditorFileManager::SaveLevel(const std::string& path,
                                           const std::vector<EditorEntityData>& entities,
                                           const std::string& levelName) {
            m_lastError.clear();
            ECS::LevelData levelData = GatherLevelData(entities, levelName);

            std::vector<std::filesystem::path> candidatePaths;
            candidatePaths.emplace_back(std::filesystem::path("..") / path);
            candidatePaths.emplace_back(path);

            std::string lastFailure;

            for (const auto& candidate : candidatePaths) {
                try {
                    if (candidate.has_parent_path()) {
                        std::error_code ec;
                        std::filesystem::create_directories(candidate.parent_path(), ec);
                        if (ec) {
                            throw std::runtime_error("Failed to create directories for "
                                                     + candidate.parent_path().string() + ": " + ec.message());
                        }
                    }

                    ECS::LevelLoader::SaveToFile(levelData, candidate.string());

                    Core::Logger::Info("[EditorFileManager] Level '{}' saved to {}",
                                       levelName, candidate.string());
                    return true;

                } catch (const std::exception& e) {
                    lastFailure = e.what();
                    Core::Logger::Warning("[EditorFileManager] Save attempt failed for {}: {}",
                                          candidate.string(), e.what());
                }
            }

            m_lastError = lastFailure.empty()
                ? "Save failed: no valid path candidates"
                : std::string("Save failed: ") + lastFailure;
            Core::Logger::Error("[EditorFileManager] {}", m_lastError);
            return false;
        }

        std::vector<EditorEntityData> EditorFileManager::LoadLevel(const std::string& path,
                                                                     ECS::Registry& registry,
                                                                     Renderer::IRenderer* renderer) {
            std::vector<EditorEntityData> result;

            try {
                m_lastError.clear();

                ECS::LevelData levelData = ECS::LevelLoader::LoadFromFile(path);

                if (!levelData.background.texture.empty()) {
                    result.push_back(ConvertBackgroundToEditor(levelData.background, registry, renderer));
                }

                for (const auto& obs : levelData.obstacles) {
                    result.push_back(ConvertObstacleToEditor(obs, registry, renderer));
                }

                for (const auto& enemy : levelData.enemies) {
                    result.push_back(ConvertEnemyToEditor(enemy, registry, renderer));
                }

                for (const auto& spawn : levelData.playerSpawns) {
                    result.push_back(ConvertSpawnToEditor(spawn, registry, renderer));
                }

                Core::Logger::Info("[EditorFileManager] Successfully loaded {} entities from {}",
                                   result.size(), path);

            } catch (const std::exception& e) {
                m_lastError = std::string("Load failed: ") + e.what();
                Core::Logger::Error("[EditorFileManager] {}", m_lastError);
                result.clear();
            }

            return result;
        }

        ECS::LevelData EditorFileManager::GatherLevelData(const std::vector<EditorEntityData>& entities,
                                                           const std::string& levelName) {
            ECS::LevelData level;
            level.name = levelName;

            level.textures["background"] = "assets/backgrounds/Cave_one.png";
            level.textures["obstacle1"] = "assets/backgrounds/obstacles/middle_obstacle.png";
            level.textures["obstacle2"] = "assets/backgrounds/obstacles/obstacle_bas.png";
            level.textures["obstacle3"] = "assets/backgrounds/obstacles/little_obstacle.png";
            level.textures["obstacle4"] = "assets/backgrounds/obstacles/obstacle_square.png";
            level.textures["obstacle5"] = "assets/backgrounds/obstacles/obstacle_bas_two.png";
            level.textures["obstacle6"] = "assets/backgrounds/obstacles/obstacle_haut_two.png";
            level.textures["obstacle7"] = "assets/backgrounds/obstacles/obstacle_yellow.png";
            level.textures["obstacle8"] = "assets/backgrounds/obstacles/little_obstacle_haut.png";
            level.textures["player_blue"] = "assets/spaceships/player_blue.png";
            level.textures["player_green"] = "assets/spaceships/player_green.png";
            level.textures["player_red"] = "assets/spaceships/player_red.png";
            level.textures["enemy-green"] = "assets/spaceships/enemy-green.png";
            level.textures["enemy-red"] = "assets/spaceships/enemy-red.png";
            level.textures["enemy-blue"] = "assets/spaceships/enemy-blue.png";
            level.textures["bullet"] = "assets/projectiles/bullet.png";
            level.textures["powerup-spread"] = "assets/powerups/spread.png";
            level.textures["powerup-laser"] = "assets/powerups/laser.png";
            level.textures["powerup-force-pod"] = "assets/powerups/force_pod.png";
            level.textures["powerup-speed"] = "assets/powerups/speed.png";
            level.textures["powerup-shield"] = "assets/powerups/shield.png";

            level.fonts["main"] = {"assets/fonts/PressStart2P-Regular.ttf", 16};
            level.fonts["small"] = {"assets/fonts/PressStart2P-Regular.ttf", 12};

            bool hasBackground = false;

            for (const auto& entity : entities) {
                switch (entity.type) {
                    case EditorEntityType::BACKGROUND: {
                        if (!hasBackground) {
                            level.background.texture = entity.textureKey;
                            level.background.scrollSpeed = entity.scrollSpeed;
                            level.background.copies = 3;
                            level.background.layer = entity.layer;
                            hasBackground = true;
                        }
                        break;
                    }

                    case EditorEntityType::OBSTACLE: {
                        ECS::ObstacleDef obs;
                        obs.texture = entity.textureKey;
                        obs.x = entity.x;
                        obs.y = entity.y;
                        obs.scaleWidth = entity.scaleWidth;
                        obs.scaleHeight = entity.scaleHeight;
                        obs.scrollSpeed = entity.scrollSpeed;
                        obs.layer = entity.layer;
                        obs.colliders = entity.colliders;
                        level.obstacles.push_back(obs);
                        break;
                    }

                    case EditorEntityType::ENEMY: {
                        ECS::EnemyDef enemy;
                        enemy.type = entity.enemyType;
                        enemy.x = entity.x;
                        enemy.y = entity.y;
                        level.enemies.push_back(enemy);
                        break;
                    }

                    case EditorEntityType::PLAYER_SPAWN: {
                        ECS::PlayerSpawnDef spawn;
                        spawn.x = entity.x;
                        spawn.y = entity.y;
                        level.playerSpawns.push_back(spawn);
                        break;
                    }

                    case EditorEntityType::POWERUP:
                        break;
                }
            }

            if (!hasBackground) {
                level.background.texture = "background";
                level.background.scrollSpeed = -50.0f;
                level.background.copies = 3;
                level.background.layer = -100;
            }

            if (level.playerSpawns.empty()) {
                level.playerSpawns.push_back({100.0f, 200.0f});
                level.playerSpawns.push_back({100.0f, 360.0f});
                level.playerSpawns.push_back({100.0f, 520.0f});
                level.playerSpawns.push_back({100.0f, 680.0f});
            }

            return level;
        }

        EditorEntityData EditorFileManager::ConvertObstacleToEditor(const ECS::ObstacleDef& obs,
                                                                      ECS::Registry&,
                                                                      Renderer::IRenderer*) {
            EditorEntityData data;
            data.type = EditorEntityType::OBSTACLE;
            data.textureKey = obs.texture;
            data.presetId = obs.texture;
            data.x = obs.x;
            data.y = obs.y;
            data.scaleWidth = obs.scaleWidth;
            data.scaleHeight = obs.scaleHeight;
            data.scrollSpeed = obs.scrollSpeed;
            data.layer = obs.layer;
            data.colliders = obs.colliders;
            data.entity = ECS::NULL_ENTITY;

            return data;
        }

        EditorEntityData EditorFileManager::ConvertEnemyToEditor(const ECS::EnemyDef& enemy,
                                                                   ECS::Registry& registry,
                                                                   Renderer::IRenderer*) {
            EditorEntityData data;
            data.type = EditorEntityType::ENEMY;
            data.enemyType = enemy.type;
            data.x = enemy.x;
            data.y = enemy.y;
            data.scaleWidth = 50.0f;
            data.scaleHeight = 50.0f;
            data.layer = 10;
            data.entity = ECS::NULL_ENTITY;

            return data;
        }

        EditorEntityData EditorFileManager::ConvertSpawnToEditor(const ECS::PlayerSpawnDef& spawn,
                                                                   ECS::Registry&,
                                                                   Renderer::IRenderer*) {
            EditorEntityData data;
            data.type = EditorEntityType::PLAYER_SPAWN;
            data.x = spawn.x;
            data.y = spawn.y;
            data.scaleWidth = 40.0f;
            data.scaleHeight = 40.0f;
            data.layer = 5;
            data.entity = ECS::NULL_ENTITY;

            return data;
        }

        EditorEntityData EditorFileManager::ConvertBackgroundToEditor(const ECS::BackgroundDef& bg,
                                                                        ECS::Registry&,
                                                                        Renderer::IRenderer*) {
            EditorEntityData data;
            data.type = EditorEntityType::BACKGROUND;
            data.textureKey = bg.texture;
            data.presetId = bg.texture;
            data.x = 0.0f;
            data.y = 0.0f;
            data.scaleWidth = 1280.0f;
            data.scaleHeight = 720.0f;
            data.scrollSpeed = bg.scrollSpeed;
            data.layer = bg.layer;
            data.entity = ECS::NULL_ENTITY;

            return data;
        }

    }
}
