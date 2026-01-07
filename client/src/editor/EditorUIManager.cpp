#include "editor/EditorUIManager.hpp"
#include "editor/EditorConstants.hpp"
#include "ECS/Components/TextLabel.hpp"
#include <sstream>

using namespace RType::Client::EditorConstants;

namespace RType {
    namespace Client {

        EditorUIManager::EditorUIManager(Renderer::IRenderer* renderer,
                                         EditorAssetLibrary& assets,
                                         ECS::Registry& registry,
                                         std::vector<ECS::Entity>& trackedEntities,
                                         Renderer::FontId fontSmall,
                                         Renderer::FontId fontMedium)
            : m_renderer(renderer)
            , m_assets(assets)
            , m_registry(registry)
            , m_trackedEntities(trackedEntities)
            , m_fontSmall(fontSmall)
            , m_fontMedium(fontMedium)
        {
        }

        void EditorUIManager::InitializePalette() {
            m_entries.clear();
            float cursorY = UI::PALETTE_START_Y;
            m_activeSelection = EditorPaletteSelection{};

            createCategoryLabel("TOOLS", cursorY);
            cursorY += UI::PALETTE_BUTTON_HEIGHT;
            PaletteEntry selectEntry;
            selectEntry.label = "SELECT";
            selectEntry.mode = EditorMode::SELECT;
            selectEntry.entityType = EditorEntityType::OBSTACLE;
            createPaletteButton(selectEntry, cursorY);
            cursorY += UI::PALETTE_BUTTON_HEIGHT * 1.5f;

            auto createSection = [&](const std::string& label, EditorEntityType type, EditorMode mode) {
                createCategoryLabel(label, cursorY);
                cursorY += UI::PALETTE_BUTTON_HEIGHT;
                const auto& resources = m_assets.GetResources(type);
                for (const auto* resource : resources) {
                    PaletteEntry entry;
                    entry.label = resource->definition.displayName;
                    entry.mode = mode;
                    entry.entityType = type;
                    entry.subtype = resource->definition.id;
                    entry.resource = resource;
                    createPaletteButton(entry, cursorY);
                    cursorY += UI::PALETTE_BUTTON_HEIGHT;
                }
                cursorY += UI::PALETTE_BUTTON_HEIGHT * 0.5f;
            };

            createSection("ENEMIES", EditorEntityType::ENEMY, EditorMode::PLACE_ENEMY);
            createSection("OBSTACLES", EditorEntityType::OBSTACLE, EditorMode::PLACE_OBSTACLE);
            createSection("POWERUPS", EditorEntityType::POWERUP, EditorMode::PLACE_POWERUP);
            createSection("PLAYER SPAWNS", EditorEntityType::PLAYER_SPAWN, EditorMode::PLACE_PLAYER_SPAWN);
            createSection("BACKGROUNDS", EditorEntityType::BACKGROUND, EditorMode::PLACE_BACKGROUND);

            SetActiveSelection(m_activeSelection);
            InitializePropertiesPanel();
        }

        void EditorUIManager::InitializePropertiesPanel() {
            m_propertyFields.clear();

            float headerY = UI::PROPERTY_PANEL_START_Y;
            m_propertiesHeader = m_registry.CreateEntity();
            m_trackedEntities.push_back(m_propertiesHeader);
            m_registry.AddComponent(m_propertiesHeader, ECS::Position{UI::PROPERTY_PANEL_X, headerY});

            ECS::TextLabel headerLabel("PROPERTIES", m_fontMedium, 18);
            headerLabel.centered = false;
            headerLabel.color = Colors::UI_HEADER;
            m_registry.AddComponent(m_propertiesHeader, std::move(headerLabel));

            m_selectedInfoEntity = m_registry.CreateEntity();
            m_trackedEntities.push_back(m_selectedInfoEntity);
            m_registry.AddComponent(m_selectedInfoEntity, ECS::Position{UI::PROPERTY_PANEL_X, headerY + 32.0f});

            ECS::TextLabel infoLabel("No entity selected", m_fontSmall, 14);
            infoLabel.centered = false;
            infoLabel.color = Colors::UI_TEXT;
            m_registry.AddComponent(m_selectedInfoEntity, std::move(infoLabel));

            m_propertyHintEntity = m_registry.CreateEntity();
            m_trackedEntities.push_back(m_propertyHintEntity);
            m_registry.AddComponent(m_propertyHintEntity, ECS::Position{UI::PROPERTY_PANEL_X, headerY + 240.0f});

            ECS::TextLabel hintLabel("Tab cycle | ↑↓ adjust | 0-9 set | Backspace delete", m_fontSmall, 11);
            hintLabel.centered = false;
            hintLabel.color = Colors::UI_HINT;
            m_registry.AddComponent(m_propertyHintEntity, std::move(hintLabel));

            const std::vector<std::pair<std::string, EditableProperty>> propertyRows = {
                {"POS X", EditableProperty::POSITION_X},
                {"POS Y", EditableProperty::POSITION_Y},
                {"WIDTH", EditableProperty::SCALE_WIDTH},
                {"HEIGHT", EditableProperty::SCALE_HEIGHT},
                {"LAYER", EditableProperty::LAYER},
                {"SCROLL", EditableProperty::SCROLL_SPEED},
            };

            float rowY = headerY + 80.0f;
            for (const auto& [label, property] : propertyRows) {
                PropertyField field;
                field.property = property;

                field.nameEntity = m_registry.CreateEntity();
                m_trackedEntities.push_back(field.nameEntity);
                m_registry.AddComponent(field.nameEntity, ECS::Position{UI::PROPERTY_PANEL_X, rowY});

                ECS::TextLabel nameLabel(label, m_fontSmall, 13);
                nameLabel.centered = false;
                nameLabel.color = Colors::UI_TEXT;
                m_registry.AddComponent(field.nameEntity, std::move(nameLabel));

                field.valueEntity = m_registry.CreateEntity();
                m_trackedEntities.push_back(field.valueEntity);
                m_registry.AddComponent(field.valueEntity, ECS::Position{UI::PROPERTY_PANEL_X + UI::PROPERTY_VALUE_OFFSET_X, rowY});

                ECS::TextLabel valueLabel("--", m_fontSmall, 13);
                valueLabel.centered = false;
                valueLabel.color = Colors::UI_TEXT;
                m_registry.AddComponent(field.valueEntity, std::move(valueLabel));

                m_propertyFields.push_back(field);
                rowY += UI::PROPERTY_ROW_HEIGHT;
            }
        }

        void EditorUIManager::UpdateHover(Math::Vector2 mouseScreen) {
            bool hoverChanged = false;
            for (auto& entry : m_entries) {
                bool inside = pointInRect(mouseScreen, entry.bounds);
                if (entry.hovered != inside) {
                    entry.hovered = inside;
                    hoverChanged = true;
                }
            }

            if (hoverChanged) {
                refreshPaletteVisuals();
            }
        }

        std::optional<EditorPaletteSelection> EditorUIManager::HandleClick(Math::Vector2 mouseScreen) {
            for (const auto& entry : m_entries) {
                if (pointInRect(mouseScreen, entry.bounds)) {
                    EditorPaletteSelection selection;
                    selection.mode = entry.mode;
                    selection.entityType = entry.entityType;
                    selection.subtype = entry.subtype;
                    SetActiveSelection(selection);
                    return selection;
                }
            }

            return std::nullopt;
        }

        void EditorUIManager::SetActiveSelection(const EditorPaletteSelection& selection) {
            m_activeSelection = selection;
            refreshPaletteVisuals();
        }

        void EditorUIManager::UpdatePropertyPanel(const EditorEntityData* selected,
                                                  EditableProperty activeProperty,
                                                  const std::string& inputBuffer) {
            std::string infoText = "No entity selected";
            if (selected) {
                std::ostringstream oss;
                oss << "Type: ";
                switch (selected->type) {
                case EditorEntityType::ENEMY:
                    oss << "Enemy (" << (selected->enemyType.empty() ? "BASIC" : selected->enemyType) << ")";
                    break;
                case EditorEntityType::POWERUP:
                    oss << "PowerUp (" << (selected->powerUpType.empty() ? "DEFAULT" : selected->powerUpType) << ")";
                    break;
                case EditorEntityType::PLAYER_SPAWN:
                    oss << "Player Spawn";
                    break;
                case EditorEntityType::BACKGROUND:
                    oss << "Background";
                    break;
                case EditorEntityType::OBSTACLE:
                default:
                    oss << "Obstacle";
                    break;
                }
                infoText = oss.str();
            }

            if (m_selectedInfoEntity != ECS::NULL_ENTITY && m_registry.IsEntityAlive(m_selectedInfoEntity)) {
                auto& label = m_registry.GetComponent<ECS::TextLabel>(m_selectedInfoEntity);
                label.text = infoText;
            }

            for (auto& field : m_propertyFields) {
                if (!m_registry.IsEntityAlive(field.valueEntity)) {
                    continue;
                }

                auto& valueLabel = m_registry.GetComponent<ECS::TextLabel>(field.valueEntity);
                if (!selected) {
                    valueLabel.text = "--";
                    valueLabel.color = {0.5f, 0.5f, 0.5f, 0.7f};
                    continue;
                }

                float value = 0.0f;
                switch (field.property) {
                case EditableProperty::POSITION_X: value = selected->x; break;
                case EditableProperty::POSITION_Y: value = selected->y; break;
                case EditableProperty::SCALE_WIDTH: value = selected->scaleWidth; break;
                case EditableProperty::SCALE_HEIGHT: value = selected->scaleHeight; break;
                case EditableProperty::LAYER: value = static_cast<float>(selected->layer); break;
                case EditableProperty::SCROLL_SPEED: value = selected->scrollSpeed; break;
                case EditableProperty::COUNT: break;
                }

                if (!inputBuffer.empty() && field.property == activeProperty) {
                    valueLabel.text = inputBuffer;
                } else {
                    std::ostringstream oss;
                    oss.setf(std::ios::fixed);
                    oss.precision(field.property == EditableProperty::LAYER ? 0 : 1);
                    oss << value;
                    valueLabel.text = oss.str();
                }

                if (field.property == activeProperty) {
                    valueLabel.color = {1.0f, 0.85f, 0.35f, 1.0f};
                } else {
                    valueLabel.color = {0.95f, 0.95f, 0.95f, 1.0f};
                }
            }
        }

        void EditorUIManager::createCategoryLabel(const std::string& label, float y) {
            float textX = UI::PALETTE_PANEL_LEFT + 10.0f;
            ECS::Entity entity = m_registry.CreateEntity();
            m_trackedEntities.push_back(entity);
            m_registry.AddComponent(entity, ECS::Position{textX, y});

            ECS::TextLabel textLabel(label, m_fontSmall, 12);
            textLabel.centered = false;
            textLabel.color = Colors::UI_HEADER;
            m_registry.AddComponent(entity, std::move(textLabel));
        }

        void EditorUIManager::createPaletteButton(PaletteEntry entry, float y) {
            entry.bounds.position = {UI::PALETTE_PANEL_LEFT, y - (UI::PALETTE_BUTTON_HEIGHT / 2.0f)};
            entry.bounds.size = {UI::PALETTE_PANEL_WIDTH, UI::PALETTE_BUTTON_HEIGHT};

            float textX = UI::PALETTE_PANEL_LEFT + 10.0f;
            ECS::Entity entity = m_registry.CreateEntity();
            m_trackedEntities.push_back(entity);
            m_registry.AddComponent(entity, ECS::Position{textX, y});

            std::string labelText = entry.label;
            if (entry.resource) {
                labelText = entry.resource->definition.displayName;
            }

            ECS::TextLabel label(labelText, m_fontSmall, 13);
            label.centered = false;
            label.color = {0.75f, 0.75f, 0.8f, 1.0f};
            m_registry.AddComponent(entity, std::move(label));

            entry.textEntity = entity;
            m_entries.push_back(std::move(entry));
        }

        bool EditorUIManager::pointInRect(Math::Vector2 point, const Math::Rectangle& rect) const {
            return point.x >= rect.position.x &&
                point.x <= rect.position.x + rect.size.x &&
                point.y >= rect.position.y &&
                point.y <= rect.position.y + rect.size.y;
        }

        void EditorUIManager::refreshPaletteVisuals() {
            for (auto& entry : m_entries) {
                if (!m_registry.IsEntityAlive(entry.textEntity)) {
                    continue;
                }

                auto& label = m_registry.GetComponent<ECS::TextLabel>(entry.textEntity);
                bool isSelected = entry.mode == m_activeSelection.mode;
                if (!entry.subtype.empty() || !m_activeSelection.subtype.empty()) {
                    isSelected = isSelected && entry.subtype == m_activeSelection.subtype;
                }

                if (isSelected) {
                    label.color = {1.0f, 0.85f, 0.35f, 1.0f};
                } else if (entry.hovered) {
                    label.color = {0.95f, 0.95f, 0.95f, 1.0f};
                } else {
                    label.color = {0.75f, 0.75f, 0.8f, 1.0f};
                }
            }
        }

    }
}

