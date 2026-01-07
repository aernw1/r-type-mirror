/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorTypes
*/

#pragma once

#include "ECS/Entity.hpp"
#include "ECS/LevelLoader.hpp"
#include "Math/Types.hpp"
#include <string>
#include <vector>

namespace RType {
    namespace Client {

        enum class EditorEntityType {
            BACKGROUND,
            OBSTACLE,
            ENEMY,
            POWERUP,
            PLAYER_SPAWN
        };

        enum class EditorMode {
            SELECT,
            PLACE_ENEMY,
            PLACE_OBSTACLE,
            PLACE_POWERUP,
            PLACE_PLAYER_SPAWN,
            PLACE_BACKGROUND
        };

        struct EditorEntityData {
            ECS::Entity entity = ECS::NULL_ENTITY;
            EditorEntityType type = EditorEntityType::OBSTACLE;

            // Common properties
            float x = 0.0f;
            float y = 0.0f;
            float scaleWidth = 100.0f;
            float scaleHeight = 100.0f;
            int layer = 1;
            float scrollSpeed = -50.0f;

            // Type-specific data
            std::string textureKey;
            std::string enemyType;
            std::string powerUpType;
            std::vector<ECS::ColliderDef> colliders;

            // Selection state
            bool isSelected = false;
        };

        struct EditorCamera {
            float x = 640.0f;
            float y = 360.0f;
            float zoom = 1.0f;

            // Camera bounds
            float minX = -1000.0f;
            float maxX = 15000.0f;
            float minY = -500.0f;
            float maxY = 1200.0f;

            float minZoom = 0.25f;
            float maxZoom = 2.0f;
        };

        struct EditorGrid {
            bool enabled = true;
            float cellSize = 50.0f;
            bool snapToGrid = false;
            Math::Color gridColor{0.3f, 0.3f, 0.3f, 0.5f};
        };

        struct EditorPaletteSelection {
            EditorMode mode = EditorMode::SELECT;
            EditorEntityType entityType = EditorEntityType::OBSTACLE;
            std::string subtype;
        };

    }
}
