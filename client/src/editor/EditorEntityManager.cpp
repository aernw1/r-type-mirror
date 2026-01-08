#include "editor/EditorEntityManager.hpp"
#include "editor/EditorConstants.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include <algorithm>
#include <cmath>

using namespace RType::Client::EditorConstants;

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

        void EditorEntityManager::DrawColliders(int selectedColliderIndex) const {
            const auto* entity = GetSelectedEntity();
            if (!entity || !m_renderer) {
                return;
            }

            for (size_t i = 0; i < entity->colliders.size(); ++i) {
                const auto& collider = entity->colliders[i];
                bool isSelected = (static_cast<int>(i) == selectedColliderIndex);
                Math::Color color = isSelected ? Collider::COLLIDER_SELECTED : Collider::COLLIDER_NORMAL;
                drawCollider(collider, color);
            }
        }

        void EditorEntityManager::DrawColliderHandles(int colliderIndex) const {
            const auto* entity = GetSelectedEntity();
            if (!entity || !m_renderer || colliderIndex < 0 || colliderIndex >= static_cast<int>(entity->colliders.size())) {
                return;
            }

            const auto& collider = entity->colliders[static_cast<size_t>(colliderIndex)];
            const float hs = Collider::HANDLE_SIZE / 2.0f;

            drawHandle({collider.x, collider.y});
            drawHandle({collider.x + collider.width, collider.y});
            drawHandle({collider.x, collider.y + collider.height});
            drawHandle({collider.x + collider.width, collider.y + collider.height});
        }

        ColliderHandle EditorEntityManager::GetColliderHandleAt(const Math::Vector2& worldPos) const {
            const auto* entity = GetSelectedEntity();
            if (!entity) {
                return {};
            }

            const float hs = Collider::HANDLE_SIZE / 2.0f;

            for (size_t i = 0; i < entity->colliders.size(); ++i) {
                const auto& collider = entity->colliders[i];

                Math::Rectangle tlHandle = BuildRect(collider.x - hs, collider.y - hs, Collider::HANDLE_SIZE, Collider::HANDLE_SIZE);
                Math::Rectangle trHandle = BuildRect(collider.x + collider.width - hs, collider.y - hs, Collider::HANDLE_SIZE, Collider::HANDLE_SIZE);
                Math::Rectangle blHandle = BuildRect(collider.x - hs, collider.y + collider.height - hs, Collider::HANDLE_SIZE, Collider::HANDLE_SIZE);
                Math::Rectangle brHandle = BuildRect(collider.x + collider.width - hs, collider.y + collider.height - hs, Collider::HANDLE_SIZE, Collider::HANDLE_SIZE);

                if (pointInRect(worldPos, tlHandle)) {
                    return {static_cast<int>(i), ColliderHandle::Type::TOP_LEFT};
                }
                if (pointInRect(worldPos, trHandle)) {
                    return {static_cast<int>(i), ColliderHandle::Type::TOP_RIGHT};
                }
                if (pointInRect(worldPos, blHandle)) {
                    return {static_cast<int>(i), ColliderHandle::Type::BOTTOM_LEFT};
                }
                if (pointInRect(worldPos, brHandle)) {
                    return {static_cast<int>(i), ColliderHandle::Type::BOTTOM_RIGHT};
                }

                Math::Rectangle bodyRect = getColliderRect(collider);
                if (pointInRect(worldPos, bodyRect)) {
                    return {static_cast<int>(i), ColliderHandle::Type::BODY};
                }
            }

            return {};
        }

        void EditorEntityManager::AddCollider(const Math::Vector2& worldPos) {
            auto* entity = GetSelectedEntity();
            if (!entity) {
                return;
            }

            ECS::ColliderDef newCollider;
            newCollider.x = worldPos.x - 25.0f;
            newCollider.y = worldPos.y - 25.0f;
            newCollider.width = 50.0f;
            newCollider.height = 50.0f;

            entity->colliders.push_back(newCollider);
            SyncEntity(*entity);

            m_selectedColliderIndex = static_cast<int>(entity->colliders.size()) - 1;
        }

        bool EditorEntityManager::RemoveCollider(int colliderIndex) {
            auto* entity = GetSelectedEntity();
            if (!entity || colliderIndex < 0 || colliderIndex >= static_cast<int>(entity->colliders.size())) {
                return false;
            }

            entity->colliders.erase(entity->colliders.begin() + colliderIndex);
            SyncEntity(*entity);

            if (m_selectedColliderIndex == colliderIndex) {
                m_selectedColliderIndex = -1;
            } else if (m_selectedColliderIndex > colliderIndex) {
                m_selectedColliderIndex--;
            }

            return true;
        }

        void EditorEntityManager::ResizeCollider(int colliderIndex, ColliderHandle::Type handleType, const Math::Vector2& worldPos) {
            auto* entity = GetSelectedEntity();
            if (!entity || colliderIndex < 0 || colliderIndex >= static_cast<int>(entity->colliders.size())) {
                return;
            }

            auto& collider = entity->colliders[static_cast<size_t>(colliderIndex)];
            const float minSize = Collider::MIN_COLLIDER_SIZE;

            float originalRight = collider.x + collider.width;
            float originalBottom = collider.y + collider.height;

            switch (handleType) {
            case ColliderHandle::Type::TOP_LEFT:
                collider.x = std::min(worldPos.x, originalRight - minSize);
                collider.y = std::min(worldPos.y, originalBottom - minSize);
                collider.width = originalRight - collider.x;
                collider.height = originalBottom - collider.y;
                break;

            case ColliderHandle::Type::TOP_RIGHT:
                collider.y = std::min(worldPos.y, originalBottom - minSize);
                collider.width = std::max(worldPos.x - collider.x, minSize);
                collider.height = originalBottom - collider.y;
                break;

            case ColliderHandle::Type::BOTTOM_LEFT:
                collider.x = std::min(worldPos.x, originalRight - minSize);
                collider.width = originalRight - collider.x;
                collider.height = std::max(worldPos.y - collider.y, minSize);
                break;

            case ColliderHandle::Type::BOTTOM_RIGHT:
                collider.width = std::max(worldPos.x - collider.x, minSize);
                collider.height = std::max(worldPos.y - collider.y, minSize);
                break;

            case ColliderHandle::Type::BODY: {
                Math::Vector2 delta = {worldPos.x - m_dragStart.x, worldPos.y - m_dragStart.y};
                collider.x += delta.x;
                collider.y += delta.y;
                m_dragStart = worldPos;
                break;
            }

            case ColliderHandle::Type::NONE:
                break;
            }

            SyncEntity(*entity);
        }

        void EditorEntityManager::drawCollider(const ECS::ColliderDef& collider, const Math::Color& color) const {
            Math::Rectangle colliderRect = BuildRect(collider.x, collider.y, collider.width, collider.height);

            Math::Color fillColor = color;
            fillColor.a = 0.2f;
            m_renderer->DrawRectangle(colliderRect, fillColor);

            const float thickness = Collider::COLLIDER_LINE_THICKNESS;
            Math::Rectangle top = BuildRect(collider.x, collider.y, collider.width, thickness);
            Math::Rectangle bottom = BuildRect(collider.x, collider.y + collider.height - thickness, collider.width, thickness);
            Math::Rectangle left = BuildRect(collider.x, collider.y, thickness, collider.height);
            Math::Rectangle right = BuildRect(collider.x + collider.width - thickness, collider.y, thickness, collider.height);

            m_renderer->DrawRectangle(top, color);
            m_renderer->DrawRectangle(bottom, color);
            m_renderer->DrawRectangle(left, color);
            m_renderer->DrawRectangle(right, color);
        }

        void EditorEntityManager::drawHandle(const Math::Vector2& pos) const {
            const float hs = Collider::HANDLE_SIZE / 2.0f;
            Math::Rectangle handle = BuildRect(pos.x - hs, pos.y - hs, Collider::HANDLE_SIZE, Collider::HANDLE_SIZE);
            m_renderer->DrawRectangle(handle, Collider::COLLIDER_HANDLE);
        }

        Math::Rectangle EditorEntityManager::getColliderRect(const ECS::ColliderDef& collider) const {
            return BuildRect(collider.x, collider.y, collider.width, collider.height);
        }

        bool EditorEntityManager::pointInRect(const Math::Vector2& point, const Math::Rectangle& rect) const {
            return point.x >= rect.position.x &&
                   point.x <= rect.position.x + rect.size.x &&
                   point.y >= rect.position.y &&
                   point.y <= rect.position.y + rect.size.y;
        }

    }
}

