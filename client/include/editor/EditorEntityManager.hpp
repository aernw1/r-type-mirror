#pragma once

#include "editor/EditorTypes.hpp"
#include "editor/EditorAssetLibrary.hpp"
#include "ECS/Registry.hpp"
#include "Renderer/IRenderer.hpp"
#include <vector>

namespace RType {
    namespace Client {

        class EditorEntityManager {
        public:
            EditorEntityManager(Renderer::IRenderer* renderer, RType::ECS::Registry& registry, EditorAssetLibrary& assets);

            EditorEntityData* PlaceEntity(const EditorPaletteSelection& selection, const Math::Vector2& worldPos);
            void DrawPlacementPreview(EditorMode mode,
                                      EditorEntityType type,
                                      const std::string& identifier,
                                      const Math::Vector2& worldPos) const;
            void DrawSelectionOutline() const;
            void DrawColliders(int selectedColliderIndex = -1) const;
            void DrawColliderHandles(int colliderIndex) const;

            bool SelectAt(const Math::Vector2& worldPos);
            void ClearSelection();
            bool DeleteSelected();

            ColliderHandle GetColliderHandleAt(const Math::Vector2& worldPos) const;
            void AddCollider(const Math::Vector2& worldPos);
            bool RemoveCollider(int colliderIndex);
            void ResizeCollider(int colliderIndex, ColliderHandle::Type handleType, const Math::Vector2& worldPos);
            int GetSelectedColliderIndex() const { return m_selectedColliderIndex; }
            void SetSelectedCollider(int index) { m_selectedColliderIndex = index; }

            const std::vector<EditorEntityData>& GetEntities() const { return m_entities; }
            EditorEntityData* GetSelectedEntity();
            const EditorEntityData* GetSelectedEntity() const;

            void SyncEntity(EditorEntityData& data);

        private:
            EditorEntityData* createEntityFromSelection(const EditorPaletteSelection& selection, const Math::Vector2& worldPos);
            Math::Vector2 getDefaultSize(EditorEntityType type) const;
            Math::Color getColor(EditorEntityType type) const;
            const EditorAssetResource* getResource(const std::string& id) const;
            void applyEntityToComponents(EditorEntityData& data);
            void destroyEntity(EditorEntityData& data);
            void rebuildColliders(EditorEntityData& data);
            bool pointInside(const EditorEntityData& entity, const Math::Vector2& worldPos) const;
            void drawOutline(const EditorEntityData& entity) const;

            void drawCollider(const ECS::ColliderDef& collider, const Math::Color& color) const;
            void drawHandle(const Math::Vector2& pos) const;
            Math::Rectangle getColliderRect(const ECS::ColliderDef& collider) const;
            bool pointInRect(const Math::Vector2& point, const Math::Rectangle& rect) const;

            Renderer::IRenderer* m_renderer;
            RType::ECS::Registry& m_registry;
            EditorAssetLibrary& m_assets;
            std::vector<EditorEntityData> m_entities;
            int m_selectedIndex = -1;
            int m_selectedColliderIndex = -1;
            Math::Vector2 m_dragStart{0.0f, 0.0f};
        };

    }
}
