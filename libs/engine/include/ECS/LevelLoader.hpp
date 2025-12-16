/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LevelLoader - JSON-based level loading system
*/

#pragma once

#include "Registry.hpp"
#include "Component.hpp"
#include "Renderer/IRenderer.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace RType {

    namespace ECS {

        struct FontDef {
            std::string path;
            uint32_t size = 16;
        };

        struct ColliderDef {
            float x = 0.0f;
            float y = 0.0f;
            float width = 0.0f;
            float height = 0.0f;
        };

        struct ObstacleDef {
            std::string texture;
            float x = 0.0f;
            float y = 0.0f;
            float scaleWidth = 1200.0f;
            float scaleHeight = 720.0f;
            float scrollSpeed = -150.0f;
            int layer = 1;
            std::vector<ColliderDef> colliders;
        };

        struct EnemyDef {
            std::string type = "BASIC";
            float x = 0.0f;
            float y = 0.0f;
        };

        struct PlayerSpawnDef {
            float x = 100.0f;
            float y = 360.0f;
        };

        struct BackgroundDef {
            std::string texture;
            float scrollSpeed = -150.0f;
            int copies = 3;
            int layer = -100;
        };

        struct PlayerConfig {
            float movementSpeed = 200.0f;
            float fireRate = 0.2f;
            float bulletOffsetX = 50.0f;
            float bulletOffsetY = 25.0f;
            int maxHealth = 100;
        };

        struct LevelConfig {
            float screenWidth = 1280.0f;
            float screenHeight = 720.0f;
            float powerUpSpawnInterval = 5.0f;
            PlayerConfig playerDefaults;
        };

        struct LevelData {
            std::string name;

            // Asset paths
            std::unordered_map<std::string, std::string> textures;
            std::unordered_map<std::string, FontDef> fonts;

            // Level configuration
            LevelConfig config;

            // Level elements
            BackgroundDef background;
            std::vector<ObstacleDef> obstacles;
            std::vector<EnemyDef> enemies;
            std::vector<PlayerSpawnDef> playerSpawns;
        };

        struct LoadedAssets {
            std::unordered_map<std::string, Renderer::TextureId> textures;
            std::unordered_map<std::string, Renderer::SpriteId> sprites;
            std::unordered_map<std::string, Renderer::FontId> fonts;
        };

        struct CreatedEntities {
            std::vector<Entity> backgrounds;
            std::vector<Entity> obstacleVisuals;
            std::vector<Entity> obstacleColliders;
            std::vector<Entity> enemies;
        };

        class LevelLoader {
        public:
            static LevelData LoadFromFile(const std::string& path);
            static LevelData LoadFromString(const std::string& jsonString);

            static LoadedAssets LoadAssets(
                const LevelData& level,
                Renderer::IRenderer* renderer);

            static CreatedEntities CreateEntities(
                Registry& registry,
                const LevelData& level,
                const LoadedAssets& assets,
                Renderer::IRenderer* renderer);

            static CreatedEntities CreateServerEntities(
                Registry& registry,
                const LevelData& level);

            static const std::vector<PlayerSpawnDef>& GetPlayerSpawns(const LevelData& level);
        private:
            static EnemyType ParseEnemyType(const std::string& typeStr);

            static void CreateBackgrounds(
                Registry& registry,
                const BackgroundDef& background,
                const LoadedAssets& assets,
                Renderer::IRenderer* renderer,
                CreatedEntities& entities);

            static void CreateObstacles(
                Registry& registry,
                const std::vector<ObstacleDef>& obstacles,
                const LoadedAssets& assets,
                Renderer::IRenderer* renderer,
                CreatedEntities& entities,
                uint32_t& obstacleIdCounter);

            static void CreateEnemies(
                Registry& registry,
                const std::vector<EnemyDef>& enemies,
                const LoadedAssets& assets,
                Renderer::IRenderer* renderer,
                CreatedEntities& entities);

            static void CreateServerObstacles(
                Registry& registry,
                const std::vector<ObstacleDef>& obstacles,
                CreatedEntities& entities,
                uint32_t& obstacleIdCounter);

            static void CreateServerEnemies(
                Registry& registry,
                const std::vector<EnemyDef>& enemies,
                CreatedEntities& entities);
        };

    }

}
