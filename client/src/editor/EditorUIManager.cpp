#include "editor/EditorUIManager.hpp"
#include "ECS/Components/TextLabel.hpp"

namespace RType {
    namespace Client {

        EditorUIManager::EditorUIManager(Renderer::IRenderer* renderer,
                                         ECS::Registry& registry,
                                         std::vector<ECS::Entity>& trackedEntities,
                                         Renderer::FontId fontSmall,
                                         Renderer::FontId fontMedium)
            : m_renderer(renderer)
            , m_registry(registry)
            , m_trackedEntities(trackedEntities)
            , m_fontSmall(fontSmall)
            , m_fontMedium(fontMedium)
        {
        }

        void EditorUIManager::InitializePalette() {
            m_entries.clear();
            float cursorY = 90.0f;
            m_activeSelection = EditorPaletteSelection{};

            createCategoryLabel("TOOLS", cursorY);
            cursorY += m_buttonHeight;

            PaletteEntry selectEntry;
            selectEntry.label = "SELECT";
            selectEntry.mode = EditorMode::SELECT;
            selectEntry.entityType = EditorEntityType::OBSTACLE;
            createPaletteButton(selectEntry, cursorY);
            cursorY += m_buttonHeight * 1.5f;

            createCategoryLabel("ENEMIES", cursorY);
            cursorY += m_buttonHeight;

            const std::vector<std::pair<std::string, std::string>> enemyButtons = {
                {"BASIC", "BASIC"},
                {"FAST", "FAST"},
                {"TANK", "TANK"},
                {"BOSS", "BOSS"},
                {"FORMATION", "FORMATION"},
            };

            for (const auto& [label, subtype] : enemyButtons) {
                PaletteEntry entry;
                entry.label = "ENEMY - " + label;
                entry.mode = EditorMode::PLACE_ENEMY;
                entry.entityType = EditorEntityType::ENEMY;
                entry.subtype = subtype;
                createPaletteButton(entry, cursorY);
                cursorY += m_buttonHeight;
            }

            cursorY += m_buttonHeight * 0.5f;
            createCategoryLabel("OBSTACLES", cursorY);
            cursorY += m_buttonHeight;

            for (int idx = 1; idx <= 4; ++idx) {
                PaletteEntry entry;
                entry.label = "OBSTACLE " + std::to_string(idx);
                entry.mode = EditorMode::PLACE_OBSTACLE;
                entry.entityType = EditorEntityType::OBSTACLE;
                entry.subtype = "obstacle" + std::to_string(idx);
                createPaletteButton(entry, cursorY);
                cursorY += m_buttonHeight;
            }

            cursorY += m_buttonHeight * 0.5f;
            createCategoryLabel("POWERUPS", cursorY);
            cursorY += m_buttonHeight;

            const std::vector<std::pair<std::string, std::string>> powerButtons = {
                {"FIRE RATE", "FIRE_RATE_BOOST"},
                {"SPREAD", "SPREAD_SHOT"},
                {"LASER", "LASER_BEAM"},
                {"SHIELD", "SHIELD"},
                {"SPEED", "SPEED_BOOST"},
                {"FORCE POD", "FORCE_POD"},
            };

            for (const auto& [label, subtype] : powerButtons) {
                PaletteEntry entry;
                entry.label = "POWERUP - " + label;
                entry.mode = EditorMode::PLACE_POWERUP;
                entry.entityType = EditorEntityType::POWERUP;
                entry.subtype = subtype;
                createPaletteButton(entry, cursorY);
                cursorY += m_buttonHeight;
            }

            cursorY += m_buttonHeight * 0.5f;
            createCategoryLabel("SPAWNS", cursorY);
            cursorY += m_buttonHeight;

            PaletteEntry spawnEntry;
            spawnEntry.label = "PLAYER SPAWN";
            spawnEntry.mode = EditorMode::PLACE_PLAYER_SPAWN;
            spawnEntry.entityType = EditorEntityType::PLAYER_SPAWN;
            spawnEntry.subtype = "PLAYER";
            createPaletteButton(spawnEntry, cursorY);
            cursorY += m_buttonHeight * 1.5f;

            createCategoryLabel("BACKGROUND", cursorY);
            cursorY += m_buttonHeight;

            PaletteEntry bgEntry;
            bgEntry.label = "SPACE BACKGROUND";
            bgEntry.mode = EditorMode::PLACE_BACKGROUND;
            bgEntry.entityType = EditorEntityType::BACKGROUND;
            bgEntry.subtype = "space";
            createPaletteButton(bgEntry, cursorY);

            SetActiveSelection(m_activeSelection);
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

        void EditorUIManager::createCategoryLabel(const std::string& label, float y) {
            float textX = m_panelLeft + 10.0f;
            ECS::Entity entity = m_registry.CreateEntity();
            m_trackedEntities.push_back(entity);
            m_registry.AddComponent(entity, ECS::Position{textX, y});

            ECS::TextLabel textLabel(label, m_fontSmall, 12);
            textLabel.centered = false;
            textLabel.color = {0.4f, 0.86f, 0.9f, 1.0f};
            m_registry.AddComponent(entity, std::move(textLabel));
        }

        void EditorUIManager::createPaletteButton(PaletteEntry entry, float y) {
            entry.bounds.position = {m_panelLeft, y - (m_buttonHeight / 2.0f)};
            entry.bounds.size = {m_panelWidth, m_buttonHeight};

            float textX = m_panelLeft + 10.0f;
            ECS::Entity entity = m_registry.CreateEntity();
            m_trackedEntities.push_back(entity);
            m_registry.AddComponent(entity, ECS::Position{textX, y});

            ECS::TextLabel label(entry.label, m_fontSmall, 13);
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

