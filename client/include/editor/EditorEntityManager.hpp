#pragma once

#include "editor/EditorTypes.hpp"
#include "editor/EditorAssetLibrary.hpp"
#include "ECS/Registry.hpp"
#include "Renderer/IRenderer.hpp"
#include <vector>
#include <memory>

namespace RType {
    namespace Client {

        class EditorColliderManager;

        class EditorEntityManager {
        public:
            EditorEntityManager(Renderer::IRenderer* renderer, RType::ECS::Registry& registry, EditorAssetLibrary& assets);
            ~EditorEntityManager();

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
            void RebuildDefaultCollider(EditorEntityData& data);

        private:
            Math::Vector2 getDefaultSize(EditorEntityType type) const;
            Math::Color getColor(EditorEntityType type) const;
            const EditorAssetResource* getResource(const std::string& id) const;
            void applyEntityToComponents(EditorEntityData& data);
            void destroyEntity(EditorEntityData& data);
            void drawOutline(const EditorEntityData& entity) const;
            void initializeDefaultCollider(EditorEntityData& data) const;
            void createColliderEntities(EditorEntityData& data);

            Renderer::IRenderer* m_renderer;
            RType::ECS::Registry& m_registry;
            EditorAssetLibrary& m_assets;
            std::unique_ptr<EditorColliderManager> m_colliderManager;
            std::vector<EditorEntityData> m_entities;
            int m_selectedIndex = -1;
            int m_selectedColliderIndex = -1;
        };

    }
}
