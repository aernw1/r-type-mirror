/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorColliderManager
*/

#pragma once

#include "editor/EditorTypes.hpp"
#include "ECS/LevelLoader.hpp"
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"
#include <vector>

namespace RType {
    namespace Client {

        class EditorColliderManager {
        public:
            explicit EditorColliderManager(Renderer::IRenderer* renderer);
            ~EditorColliderManager() = default;

            void DrawColliders(const EditorEntityData* entity, int selectedIndex = -1) const;
            void DrawHandles(const EditorEntityData* entity, int colliderIndex) const;
            ColliderHandle GetHandleAt(const EditorEntityData* entity,
                                       const Math::Vector2& worldPos) const;
            int AddCollider(EditorEntityData& entity, const Math::Vector2& worldPos);
            bool RemoveCollider(EditorEntityData& entity, int colliderIndex);
            void ResizeCollider(EditorEntityData& entity, int colliderIndex,
                               ColliderHandle::Type handleType,
                               const Math::Vector2& worldPos);

            void SetDragStart(const Math::Vector2& pos) { m_dragStart = pos; }
            const Math::Vector2& GetDragStart() const { return m_dragStart; }

        private:
            Renderer::IRenderer* m_renderer;
            Math::Vector2 m_dragStart{0.0f, 0.0f};

            void drawCollider(const ECS::ColliderDef& collider, const Math::Color& color) const;
            void drawHandle(const Math::Vector2& pos) const;
            Math::Rectangle getColliderRect(const ECS::ColliderDef& collider) const;
        };

    }
}
