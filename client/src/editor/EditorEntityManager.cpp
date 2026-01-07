#include "editor/EditorEntityManager.hpp"
#include <utility>

namespace RType {
    namespace Client {

        EditorEntityManager::EditorEntityManager(Renderer::IRenderer* renderer)
            : m_renderer(renderer)
        {
        }

        void EditorEntityManager::PlaceEntity(EditorEntityType type, const std::string& identifier, const Math::Vector2& worldPos) {
            EditorEntityData data;
            data.type = type;
            data.x = worldPos.x;
            data.y = worldPos.y;

            Math::Vector2 size = getDefaultSize(type);
            data.scaleWidth = size.x;
            data.scaleHeight = size.y;

            switch (type) {
            case EditorEntityType::ENEMY:
                data.enemyType = identifier;
                break;
            case EditorEntityType::POWERUP:
                data.powerUpType = identifier;
                break;
            case EditorEntityType::OBSTACLE:
                data.textureKey = identifier;
                break;
            case EditorEntityType::BACKGROUND:
                data.textureKey = identifier;
                break;
            case EditorEntityType::PLAYER_SPAWN:
                data.textureKey = identifier;
                break;
            }

            m_entities.push_back(std::move(data));
        }

        void EditorEntityManager::DrawEntities() const {
            if (!m_renderer) {
                return;
            }

            for (const auto& entity : m_entities) {
                Math::Rectangle rect;
                rect.position = {entity.x - entity.scaleWidth / 2.0f, entity.y - entity.scaleHeight / 2.0f};
                rect.size = {entity.scaleWidth, entity.scaleHeight};
                Math::Color color = getColor(entity.type);
                color.a = 0.9f;
                m_renderer->DrawRectangle(rect, color);
            }
        }

        void EditorEntityManager::DrawPlacementPreview(EditorMode mode,
                                                       EditorEntityType type,
                                                       const std::string&,
                                                       const Math::Vector2& worldPos) const {
            if (!m_renderer || mode == EditorMode::SELECT) {
                return;
            }

            Math::Vector2 size = getDefaultSize(type);
            Math::Rectangle rect;
            rect.position = {worldPos.x - size.x / 2.0f, worldPos.y - size.y / 2.0f};
            rect.size = {size.x, size.y};

            Math::Color color = getColor(type);
            color.a = 0.35f;
            m_renderer->DrawRectangle(rect, color);
        }

        Math::Vector2 EditorEntityManager::getDefaultSize(EditorEntityType type) const {
            switch (type) {
            case EditorEntityType::ENEMY:
                return {80.0f, 60.0f};
            case EditorEntityType::POWERUP:
                return {50.0f, 50.0f};
            case EditorEntityType::PLAYER_SPAWN:
                return {40.0f, 40.0f};
            case EditorEntityType::BACKGROUND:
                return {1280.0f, 720.0f};
            case EditorEntityType::OBSTACLE:
            default:
                return {120.0f, 80.0f};
            }
        }

        Math::Color EditorEntityManager::getColor(EditorEntityType type) const {
            switch (type) {
            case EditorEntityType::ENEMY:
                return {0.95f, 0.35f, 0.35f, 1.0f};
            case EditorEntityType::POWERUP:
                return {0.4f, 0.9f, 0.6f, 1.0f};
            case EditorEntityType::PLAYER_SPAWN:
                return {0.9f, 0.9f, 0.1f, 1.0f};
            case EditorEntityType::BACKGROUND:
                return {0.4f, 0.5f, 0.95f, 1.0f};
            case EditorEntityType::OBSTACLE:
            default:
                return {0.6f, 0.7f, 0.95f, 1.0f};
            }
        }

    }
}

