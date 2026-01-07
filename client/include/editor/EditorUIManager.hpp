#pragma once

#include "editor/EditorTypes.hpp"
#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"
#include <optional>
#include <string>
#include <vector>

namespace RType {
    namespace Client {

        class EditorUIManager {
        public:
            EditorUIManager(Renderer::IRenderer* renderer,
                            ECS::Registry& registry,
                            std::vector<ECS::Entity>& trackedEntities,
                            Renderer::FontId fontSmall,
                            Renderer::FontId fontMedium);

            void InitializePalette();
            void UpdateHover(Math::Vector2 mouseScreen);
            std::optional<EditorPaletteSelection> HandleClick(Math::Vector2 mouseScreen);
            void SetActiveSelection(const EditorPaletteSelection& selection);
            const EditorPaletteSelection& GetActiveSelection() const { return m_activeSelection; }

        private:
            struct PaletteEntry {
                std::string label;
                EditorMode mode = EditorMode::SELECT;
                EditorEntityType entityType = EditorEntityType::OBSTACLE;
                std::string subtype;
                Math::Rectangle bounds;
                ECS::Entity textEntity = ECS::NULL_ENTITY;
                bool hovered = false;
            };

            void createCategoryLabel(const std::string& label, float y);
            void createPaletteButton(PaletteEntry entry, float y);
            bool pointInRect(Math::Vector2 point, const Math::Rectangle& rect) const;
            void refreshPaletteVisuals();

            Renderer::IRenderer* m_renderer;
            ECS::Registry& m_registry;
            std::vector<ECS::Entity>& m_trackedEntities;
            Renderer::FontId m_fontSmall;
            Renderer::FontId m_fontMedium;

            std::vector<PaletteEntry> m_entries;
            EditorPaletteSelection m_activeSelection;

            float m_panelLeft = 40.0f;
            float m_panelWidth = 220.0f;
            float m_buttonHeight = 22.0f;
        };

    }
}

