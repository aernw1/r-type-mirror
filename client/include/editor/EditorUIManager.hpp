#pragma once

#include "editor/EditorTypes.hpp"
#include "editor/EditorAssetLibrary.hpp"
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
                            EditorAssetLibrary& assets,
                            ECS::Registry& registry,
                            std::vector<ECS::Entity>& trackedEntities,
                            Renderer::FontId fontSmall,
                            Renderer::FontId fontMedium);

            void InitializePalette();
            void InitializePropertiesPanel();
            void UpdateHover(Math::Vector2 mouseScreen);
            std::optional<EditorPaletteSelection> HandleClick(Math::Vector2 mouseScreen);
            void SetActiveSelection(const EditorPaletteSelection& selection);
            const EditorPaletteSelection& GetActiveSelection() const { return m_activeSelection; }
            void UpdatePropertyPanel(const EditorEntityData* selected,
                                     EditableProperty activeProperty,
                                     const std::string& inputBuffer);

        private:
            struct PaletteEntry {
                std::string label;
                EditorMode mode = EditorMode::SELECT;
                EditorEntityType entityType = EditorEntityType::OBSTACLE;
                std::string subtype;
                Math::Rectangle bounds;
                ECS::Entity textEntity = ECS::NULL_ENTITY;
                bool hovered = false;
                const EditorAssetResource* resource = nullptr;
            };

            struct PropertyField {
                EditableProperty property = EditableProperty::POSITION_X;
                ECS::Entity nameEntity = ECS::NULL_ENTITY;
                ECS::Entity valueEntity = ECS::NULL_ENTITY;
            };

            void createCategoryLabel(const std::string& label, float y);
            void createPaletteButton(PaletteEntry entry, float y);
            bool pointInRect(Math::Vector2 point, const Math::Rectangle& rect) const;
            void refreshPaletteVisuals();

            Renderer::IRenderer* m_renderer;
            EditorAssetLibrary& m_assets;
            ECS::Registry& m_registry;
            std::vector<ECS::Entity>& m_trackedEntities;
            Renderer::FontId m_fontSmall;
            Renderer::FontId m_fontMedium;

            std::vector<PaletteEntry> m_entries;
            EditorPaletteSelection m_activeSelection;
            std::vector<PropertyField> m_propertyFields;
            ECS::Entity m_propertiesHeader = ECS::NULL_ENTITY;
            ECS::Entity m_selectedInfoEntity = ECS::NULL_ENTITY;
            ECS::Entity m_propertyHintEntity = ECS::NULL_ENTITY;

            float m_panelLeft = 40.0f;
            float m_panelWidth = 220.0f;
            float m_buttonHeight = 22.0f;
            float m_propertyPanelX = 980.0f;
            float m_propertyPanelWidth = 260.0f;
        };

    }
}

