/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorColliderManager
*/

#include "editor/EditorColliderManager.hpp"
#include "editor/EditorConstants.hpp"
#include "editor/EditorGeometry.hpp"
#include "editor/EditorDrawing.hpp"
#include <algorithm>

using namespace RType::Client::EditorConstants;

namespace RType {
    namespace Client {

        EditorColliderManager::EditorColliderManager(Renderer::IRenderer* renderer)
            : m_renderer(renderer)
        {
        }

        void EditorColliderManager::DrawColliders(const EditorEntityData* entity, int selectedIndex) const {
            if (!entity || !m_renderer) {
                return;
            }

            for (size_t i = 0; i < entity->colliders.size(); ++i) {
                const auto& collider = entity->colliders[i];
                bool isSelected = (static_cast<int>(i) == selectedIndex);
                Math::Color color = isSelected ? Collider::COLLIDER_SELECTED : Collider::COLLIDER_NORMAL;
                drawCollider(collider, color);
            }
        }

        void EditorColliderManager::DrawHandles(const EditorEntityData* entity, int colliderIndex) const {
            if (!entity || !m_renderer || colliderIndex < 0 || colliderIndex >= static_cast<int>(entity->colliders.size())) {
                return;
            }

            const auto& collider = entity->colliders[static_cast<size_t>(colliderIndex)];

            // Draw handles at the four corners
            drawHandle({collider.x, collider.y});
            drawHandle({collider.x + collider.width, collider.y});
            drawHandle({collider.x, collider.y + collider.height});
            drawHandle({collider.x + collider.width, collider.y + collider.height});
        }

        ColliderHandle EditorColliderManager::GetHandleAt(const EditorEntityData* entity,
                                                           const Math::Vector2& worldPos) const {
            if (!entity) {
                return {};
            }

            const float hs = Collider::HANDLE_SIZE / 2.0f;

            for (size_t i = 0; i < entity->colliders.size(); ++i) {
                const auto& collider = entity->colliders[i];

                // Check corner handles first (they have priority over body)
                Math::Rectangle tlHandle = EditorGeometry::BuildRect(
                    collider.x - hs, collider.y - hs, Collider::HANDLE_SIZE, Collider::HANDLE_SIZE);
                Math::Rectangle trHandle = EditorGeometry::BuildRect(
                    collider.x + collider.width - hs, collider.y - hs, Collider::HANDLE_SIZE, Collider::HANDLE_SIZE);
                Math::Rectangle blHandle = EditorGeometry::BuildRect(
                    collider.x - hs, collider.y + collider.height - hs, Collider::HANDLE_SIZE, Collider::HANDLE_SIZE);
                Math::Rectangle brHandle = EditorGeometry::BuildRect(
                    collider.x + collider.width - hs, collider.y + collider.height - hs, Collider::HANDLE_SIZE, Collider::HANDLE_SIZE);

                if (EditorGeometry::PointInRect(worldPos, tlHandle)) {
                    return {static_cast<int>(i), ColliderHandle::Type::TOP_LEFT};
                }
                if (EditorGeometry::PointInRect(worldPos, trHandle)) {
                    return {static_cast<int>(i), ColliderHandle::Type::TOP_RIGHT};
                }
                if (EditorGeometry::PointInRect(worldPos, blHandle)) {
                    return {static_cast<int>(i), ColliderHandle::Type::BOTTOM_LEFT};
                }
                if (EditorGeometry::PointInRect(worldPos, brHandle)) {
                    return {static_cast<int>(i), ColliderHandle::Type::BOTTOM_RIGHT};
                }

                // Check body last
                Math::Rectangle bodyRect = getColliderRect(collider);
                if (EditorGeometry::PointInRect(worldPos, bodyRect)) {
                    return {static_cast<int>(i), ColliderHandle::Type::BODY};
                }
            }

            return {};
        }

        int EditorColliderManager::AddCollider(EditorEntityData& entity, const Math::Vector2& worldPos) {
            // Create a default-sized collider at the specified position
            ECS::ColliderDef newCollider;
            newCollider.x = worldPos.x - 25.0f;
            newCollider.y = worldPos.y - 25.0f;
            newCollider.width = 50.0f;
            newCollider.height = 50.0f;

            entity.colliders.push_back(newCollider);

            return static_cast<int>(entity.colliders.size()) - 1;
        }

        bool EditorColliderManager::RemoveCollider(EditorEntityData& entity, int colliderIndex) {
            if (colliderIndex < 0 || colliderIndex >= static_cast<int>(entity.colliders.size())) {
                return false;
            }

            entity.colliders.erase(entity.colliders.begin() + colliderIndex);
            return true;
        }

        void EditorColliderManager::ResizeCollider(EditorEntityData& entity, int colliderIndex,
                                                    ColliderHandle::Type handleType,
                                                    const Math::Vector2& worldPos) {
            if (colliderIndex < 0 || colliderIndex >= static_cast<int>(entity.colliders.size())) {
                return;
            }

            auto& collider = entity.colliders[static_cast<size_t>(colliderIndex)];
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
        }

        void EditorColliderManager::drawCollider(const ECS::ColliderDef& collider, const Math::Color& color) const {
            // Draw semi-transparent fill
            Math::Rectangle colliderRect = EditorGeometry::BuildRect(
                collider.x, collider.y, collider.width, collider.height);
            Math::Color fillColor = color;
            fillColor.a = 0.2f;
            m_renderer->DrawRectangle(colliderRect, fillColor);

            // Draw outline using utility
            EditorDrawing::DrawCollider(m_renderer, collider, color, Collider::COLLIDER_LINE_THICKNESS);
        }

        void EditorColliderManager::drawHandle(const Math::Vector2& pos) const {
            EditorDrawing::DrawHandle(m_renderer, pos, Collider::COLLIDER_HANDLE, Collider::HANDLE_SIZE);
        }

        Math::Rectangle EditorColliderManager::getColliderRect(const ECS::ColliderDef& collider) const {
            return EditorGeometry::BuildRect(collider.x, collider.y, collider.width, collider.height);
        }

    }
}
