/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorFileManager - Save/Load level files
*/

#pragma once

#include "editor/EditorTypes.hpp"
#include "editor/EditorAssetLibrary.hpp"
#include "LevelLoader.hpp"
#include "ECS/Registry.hpp"
#include "Renderer/IRenderer.hpp"
#include <string>
#include <vector>

namespace RType {
    namespace Client {

        class EditorFileManager {
        public:
            explicit EditorFileManager(EditorAssetLibrary& assets);

            bool SaveLevel(const std::string& path,
                           const std::vector<EditorEntityData>& entities,
                           const std::string& levelName);

            std::vector<EditorEntityData> LoadLevel(const std::string& path,
                                                     ECS::Registry& registry,
                                                     Renderer::IRenderer* renderer);

            const std::string& GetLastError() const { return m_lastError; }

        private:
            ECS::LevelData GatherLevelData(const std::vector<EditorEntityData>& entities,
                                            const std::string& levelName);

            EditorEntityData ConvertObstacleToEditor(const ECS::ObstacleDef& obs,
                                                      ECS::Registry& registry,
                                                      Renderer::IRenderer* renderer);
            EditorEntityData ConvertEnemyToEditor(const ECS::EnemyDef& enemy,
                                                   ECS::Registry& registry,
                                                   Renderer::IRenderer* renderer);
            EditorEntityData ConvertSpawnToEditor(const ECS::PlayerSpawnDef& spawn,
                                                   ECS::Registry& registry,
                                                   Renderer::IRenderer* renderer);
            EditorEntityData ConvertBackgroundToEditor(const ECS::BackgroundDef& bg,
                                                        ECS::Registry& registry,
                                                        Renderer::IRenderer* renderer);

            EditorAssetLibrary& m_assets;
            std::string m_lastError;
        };

    }
}
