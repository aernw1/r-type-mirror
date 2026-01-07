#pragma once

#include "editor/EditorTypes.hpp"
#include "Renderer/IRenderer.hpp"
#include <vector>

namespace RType {
    namespace Client {

        class EditorEntityManager {
        public:
            explicit EditorEntityManager(Renderer::IRenderer* renderer);

            void PlaceEntity(EditorEntityType type, const std::string& identifier, const Math::Vector2& worldPos);
            void DrawEntities() const;
            void DrawPlacementPreview(EditorMode mode,
                                      EditorEntityType type,
                                      const std::string& identifier,
                                      const Math::Vector2& worldPos) const;

            const std::vector<EditorEntityData>& GetEntities() const { return m_entities; }

        private:
            Math::Vector2 getDefaultSize(EditorEntityType type) const;
            Math::Color getColor(EditorEntityType type) const;

            Renderer::IRenderer* m_renderer;
            std::vector<EditorEntityData> m_entities;
        };

    }
}

