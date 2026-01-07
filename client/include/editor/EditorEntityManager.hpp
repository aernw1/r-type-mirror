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

            bool SelectAt(const Math::Vector2& worldPos);
            void ClearSelection();
            bool DeleteSelected();

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

            Renderer::IRenderer* m_renderer;
            RType::ECS::Registry& m_registry;
            EditorAssetLibrary& m_assets;
            std::vector<EditorEntityData> m_entities;
            int m_selectedIndex = -1;
        };

    }
}
