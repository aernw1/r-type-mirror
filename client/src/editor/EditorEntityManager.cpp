#include "editor/EditorEntityManager.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include <algorithm>

namespace RType {
    namespace Client {

        namespace {
            Math::Rectangle BuildRect(float x, float y, float width, float height) {
                Math::Rectangle rect;
                rect.position = {x, y};
                rect.size = {width, height};
                return rect;
            }
        }

        EditorEntityManager::EditorEntityManager(Renderer::IRenderer* renderer,
                                                 RType::ECS::Registry& registry,
                                                 EditorAssetLibrary& assets)
            : m_renderer(renderer)
            , m_registry(registry)
            , m_assets(assets)
        {
        }

        EditorEntityData* EditorEntityManager::PlaceEntity(const EditorPaletteSelection& selection, const Math::Vector2& worldPos) {
            auto* resource = getResource(selection.subtype);
            if (!resource) {
                Core::Logger::Warning("[EditorEntityManager] Unknown asset '{}'", selection.subtype);
                return nullptr;
            }

            EditorEntityData data;
            data.type = selection.entityType;
            data.presetId = resource->definition.id;
            data.textureKey = resource->definition.id;
            data.scaleWidth = resource->definition.defaultSize.x;
            data.scaleHeight = resource->definition.defaultSize.y;
            data.x = worldPos.x;
            data.y = worldPos.y;
            data.scrollSpeed = resource->definition.defaultScrollSpeed;
            data.layer = resource->definition.defaultLayer;
            data.enemyType = resource->definition.enemyType;
            data.powerUpType = resource->definition.powerUpType;

            data.colliders.clear();
            ECS::ColliderDef collider;
            collider.x = data.x - data.scaleWidth / 2.0f;
            collider.y = data.y - data.scaleHeight / 2.0f;
            collider.width = data.scaleWidth;
            collider.height = data.scaleHeight;
            data.colliders.push_back(collider);

            m_entities.push_back(data);
            EditorEntityData& stored = m_entities.back();
            applyEntityToComponents(stored);

            m_selectedIndex = static_cast<int>(m_entities.size()) - 1;
            stored.isSelected = true;

            return &stored;
        }

        void EditorEntityManager::DrawPlacementPreview(EditorMode mode,
                                                       EditorEntityType type,
                                                       const std::string& identifier,
                                                       const Math::Vector2& worldPos) const {
            if (!m_renderer || mode == EditorMode::SELECT) {
                return;
            }

            Math::Vector2 size = getDefaultSize(type);
            if (!identifier.empty()) {
                if (const auto* resource = getResource(identifier)) {
                    size = resource->definition.defaultSize;
                }
            }
            Math::Rectangle rect;
            rect.position = {worldPos.x - size.x / 2.0f, worldPos.y - size.y / 2.0f};
            rect.size = {size.x, size.y};

            Math::Color color = getColor(type);
            color.a = 0.35f;
            m_renderer->DrawRectangle(rect, color);
        }

        void EditorEntityManager::DrawSelectionOutline() const {
            if (!m_renderer || !GetSelectedEntity()) {
                return;
            }
            drawOutline(*GetSelectedEntity());
        }

        bool EditorEntityManager::SelectAt(const Math::Vector2& worldPos) {
            bool found = false;
            for (auto& entity : m_entities) {
                entity.isSelected = false;
            }

            for (int i = static_cast<int>(m_entities.size()) - 1; i >= 0; --i) {
                if (pointInside(m_entities[static_cast<size_t>(i)], worldPos)) {
                    m_entities[static_cast<size_t>(i)].isSelected = true;
                    m_selectedIndex = i;
                    found = true;
                    break;
                }
            }

            if (!found) {
                m_selectedIndex = -1;
            }

            return found;
        }

        void EditorEntityManager::ClearSelection() {
            for (auto& entity : m_entities) {
                entity.isSelected = false;
            }
            m_selectedIndex = -1;
        }

        bool EditorEntityManager::DeleteSelected() {
            if (!GetSelectedEntity()) {
                return false;
            }

            destroyEntity(m_entities[static_cast<size_t>(m_selectedIndex)]);
            m_entities.erase(m_entities.begin() + m_selectedIndex);

            if (m_entities.empty()) {
                m_selectedIndex = -1;
            } else {
                m_selectedIndex = std::min(m_selectedIndex, static_cast<int>(m_entities.size()) - 1);
                if (m_selectedIndex >= 0) {
                    m_entities[static_cast<size_t>(m_selectedIndex)].isSelected = true;
                }
            }

            return true;
        }

        EditorEntityData* EditorEntityManager::GetSelectedEntity() {
            if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_entities.size())) {
                return nullptr;
            }
            return &m_entities[static_cast<size_t>(m_selectedIndex)];
        }

        const EditorEntityData* EditorEntityManager::GetSelectedEntity() const {
            if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_entities.size())) {
                return nullptr;
            }
            return &m_entities[static_cast<size_t>(m_selectedIndex)];
        }

        void EditorEntityManager::SyncEntity(EditorEntityData& data) {
            applyEntityToComponents(data);
        }

        const EditorAssetResource* EditorEntityManager::getResource(const std::string& id) const {
            return m_assets.GetResource(id);
        }

        void EditorEntityManager::applyEntityToComponents(EditorEntityData& data) {
            destroyEntity(data);

            auto* resource = getResource(data.presetId.empty() ? data.textureKey : data.presetId);
            if (!resource || !m_renderer) {
                return;
            }

            float left = data.x - data.scaleWidth / 2.0f;
            float top = data.y - data.scaleHeight / 2.0f;

            data.entity = m_registry.CreateEntity();
            m_registry.AddComponent(data.entity, ECS::Position{left, top});

            auto& drawable = m_registry.AddComponent(data.entity, ECS::Drawable(resource->spriteId, data.layer));
            drawable.scale = {data.scaleWidth / resource->textureSize.x, data.scaleHeight / resource->textureSize.y};
            drawable.origin = {0.0f, 0.0f};

            if (data.scrollSpeed != 0.0f) {
                m_registry.AddComponent(data.entity, ECS::Scrollable(data.scrollSpeed));
            }

            data.colliderEntities.clear();
            for (const auto& collider : data.colliders) {
                ECS::Entity colliderEntity = m_registry.CreateEntity();
                m_registry.AddComponent(colliderEntity, ECS::Position{collider.x, collider.y});
                m_registry.AddComponent(colliderEntity, ECS::BoxCollider{collider.width, collider.height});
                m_registry.AddComponent(colliderEntity, ECS::Scrollable(data.scrollSpeed));
                m_registry.AddComponent(colliderEntity, ECS::Obstacle(true));
                m_registry.AddComponent(colliderEntity, ECS::CollisionLayer(ECS::CollisionLayers::OBSTACLE, ECS::CollisionLayers::ALL));
                data.colliderEntities.push_back(colliderEntity);
            }
        }

        void EditorEntityManager::destroyEntity(EditorEntityData& data) {
            if (data.entity != ECS::NULL_ENTITY && m_registry.IsEntityAlive(data.entity)) {
                m_registry.DestroyEntity(data.entity);
            }
            data.entity = ECS::NULL_ENTITY;

            for (ECS::Entity collider : data.colliderEntities) {
                if (m_registry.IsEntityAlive(collider)) {
                    m_registry.DestroyEntity(collider);
                }
            }
            data.colliderEntities.clear();
        }

        bool EditorEntityManager::pointInside(const EditorEntityData& entity, const Math::Vector2& worldPos) const {
            float left = entity.x - entity.scaleWidth / 2.0f;
            float right = entity.x + entity.scaleWidth / 2.0f;
            float top = entity.y - entity.scaleHeight / 2.0f;
            float bottom = entity.y + entity.scaleHeight / 2.0f;

            return worldPos.x >= left && worldPos.x <= right &&
                worldPos.y >= top && worldPos.y <= bottom;
        }

        void EditorEntityManager::drawOutline(const EditorEntityData& entity) const {
            Math::Rectangle rect = BuildRect(entity.x - entity.scaleWidth / 2.0f,
                                             entity.y - entity.scaleHeight / 2.0f,
                                             entity.scaleWidth,
                                             entity.scaleHeight);
            Math::Color color = {1.0f, 0.85f, 0.35f, 1.0f};

            constexpr float thickness = 3.0f;
            Math::Rectangle top = BuildRect(rect.position.x, rect.position.y, rect.size.x, thickness);
            Math::Rectangle bottom = BuildRect(rect.position.x, rect.position.y + rect.size.y - thickness, rect.size.x, thickness);
            Math::Rectangle left = BuildRect(rect.position.x, rect.position.y, thickness, rect.size.y);
            Math::Rectangle right = BuildRect(rect.position.x + rect.size.x - thickness, rect.position.y, thickness, rect.size.y);

            m_renderer->DrawRectangle(top, color);
            m_renderer->DrawRectangle(bottom, color);
            m_renderer->DrawRectangle(left, color);
            m_renderer->DrawRectangle(right, color);
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

