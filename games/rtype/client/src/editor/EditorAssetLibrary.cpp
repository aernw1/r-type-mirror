#include "editor/EditorAssetLibrary.hpp"
#include "Core/Logger.hpp"

namespace RType {
    namespace Client {

        namespace {
            std::string ResolvePath(const std::string& relativePath) {
                return "../" + relativePath;
            }
        }

        EditorAssetLibrary::EditorAssetLibrary(Renderer::IRenderer* renderer)
            : m_renderer(renderer)
        {
        }

        bool EditorAssetLibrary::Initialize() {
            auto definitions = buildDefaultDefinitions();
            bool anyLoaded = false;

            for (auto& def : definitions) {
                EditorAssetResource resource;
                resource.definition = std::move(def);
                if (loadResource(resource)) {
                    const std::string& id = resource.definition.id;
                    m_resourcesById.emplace(id, std::move(resource));
                    anyLoaded = true;
                } else {
                    Core::Logger::Warning("[EditorAssetLibrary] Failed to load asset '{}'", resource.definition.id);
                }
            }

            m_resourcesByType.clear();
            for (auto& [id, resource] : m_resourcesById) {
                m_resourcesByType[static_cast<int>(resource.definition.type)].push_back(&resource);
            }

            return anyLoaded;
        }

        const EditorAssetResource* EditorAssetLibrary::GetResource(const std::string& id) const {
            auto it = m_resourcesById.find(id);
            if (it == m_resourcesById.end()) {
                return nullptr;
            }
            return &it->second;
        }

        const std::vector<const EditorAssetResource*>& EditorAssetLibrary::GetResources(EditorEntityType type) const {
            static const std::vector<const EditorAssetResource*> empty;
            auto it = m_resourcesByType.find(static_cast<int>(type));
            if (it == m_resourcesByType.end()) {
                return empty;
            }
            return it->second;
        }

        bool EditorAssetLibrary::loadResource(EditorAssetResource& resource) {
            if (!m_renderer) {
                return false;
            }

            const std::string fullPath = ResolvePath(resource.definition.texturePath);
            resource.textureId = m_renderer->LoadTexture(fullPath);
            if (resource.textureId == Renderer::INVALID_TEXTURE_ID) {
                resource.textureId = m_renderer->LoadTexture(resource.definition.texturePath);
            }

            if (resource.textureId == Renderer::INVALID_TEXTURE_ID) {
                return false;
            }

            resource.spriteId = m_renderer->CreateSprite(resource.textureId, {});
            resource.textureSize = m_renderer->GetTextureSize(resource.textureId);

            if (resource.definition.defaultSize.x <= 0.0f || resource.definition.defaultSize.y <= 0.0f) {
                resource.definition.defaultSize = resource.textureSize;
            }

            return resource.spriteId != Renderer::INVALID_SPRITE_ID;
        }

        std::vector<EditorAssetDefinition> EditorAssetLibrary::buildDefaultDefinitions() const {
            return {
                // Obstacles
                {"obstacle1", "Obstacle 1", EditorEntityType::OBSTACLE, "assets/backgrounds/obstacles/middle_obstacle.png", {441.0f, 200.0f}, -50.0f, 1, "", ""},
                {"obstacle2", "Obstacle 2", EditorEntityType::OBSTACLE, "assets/backgrounds/obstacles/obstacle_bas.png", {441.0f, 120.0f}, -50.0f, 1, "", ""},
                {"obstacle3", "Obstacle 3", EditorEntityType::OBSTACLE, "assets/backgrounds/obstacles/little_obstacle.png", {250.0f, 150.0f}, -50.0f, 1, "", ""},
                {"obstacle4", "Obstacle 4", EditorEntityType::OBSTACLE, "assets/backgrounds/obstacles/obstacle_square.png", {300.0f, 300.0f}, -50.0f, 1, "", ""},
                {"obstacle5", "Obstacle 5", EditorEntityType::OBSTACLE, "assets/backgrounds/obstacles/obstacle_bas_two.png", {441.0f, 120.0f}, -50.0f, 1, "", ""},
                {"obstacle6", "Obstacle 6", EditorEntityType::OBSTACLE, "assets/backgrounds/obstacles/obstacle_haut_two.png", {441.0f, 100.0f}, -50.0f, 1, "", ""},
                {"obstacle7", "Obstacle 7", EditorEntityType::OBSTACLE, "assets/backgrounds/obstacles/obstacle_yellow.png", {350.0f, 220.0f}, -50.0f, 1, "", ""},
                {"obstacle8", "Obstacle 8", EditorEntityType::OBSTACLE, "assets/backgrounds/obstacles/little_obstacle_haut.png", {250.0f, 120.0f}, -50.0f, 1, "", ""},

                // Enemies
                {"enemy_blue", "Enemy Blue", EditorEntityType::ENEMY, "assets/spaceships/enemy-blue.png", {96.0f, 96.0f}, -10.0f, 5, "BASIC", ""},
                {"enemy_red", "Enemy Red", EditorEntityType::ENEMY, "assets/spaceships/enemy-red.png", {96.0f, 96.0f}, -10.0f, 5, "FAST", ""},
                {"enemy_green", "Enemy Green", EditorEntityType::ENEMY, "assets/spaceships/enemy-green.png", {96.0f, 96.0f}, -10.0f, 5, "TANK", ""},
                {"enemy_boss", "Enemy Boss", EditorEntityType::ENEMY, "assets/spaceships/enemy-purple-boss.png", {200.0f, 200.0f}, -15.0f, 5, "BOSS", ""},

                // Power-ups
                {"power_spread", "Power Spread", EditorEntityType::POWERUP, "assets/powerups/spread.png", {64.0f, 64.0f}, -50.0f, 2, "", "SPREAD_SHOT"},
                {"power_laser", "Power Laser", EditorEntityType::POWERUP, "assets/powerups/laser.png", {64.0f, 64.0f}, -50.0f, 2, "", "LASER_BEAM"},
                {"power_force", "Power Force Pod", EditorEntityType::POWERUP, "assets/powerups/force_pod.png", {64.0f, 64.0f}, -50.0f, 2, "", "FORCE_POD"},
                {"power_speed", "Power Speed", EditorEntityType::POWERUP, "assets/powerups/speed.png", {64.0f, 64.0f}, -50.0f, 2, "", "SPEED_BOOST"},
                {"power_shield", "Power Shield", EditorEntityType::POWERUP, "assets/powerups/shield.png", {64.0f, 64.0f}, -50.0f, 2, "", "SHIELD"},

                // Player spawn
                {"player_spawn", "Player Spawn", EditorEntityType::PLAYER_SPAWN, "assets/spaceships/player_blue.png", {96.0f, 96.0f}, 0.0f, 10, "", ""},

                // Background
                {"background_space", "Space Background", EditorEntityType::BACKGROUND, "assets/backgrounds/Cave_one.png", {1280.0f, 720.0f}, -20.0f, -100, "", ""},
            };
        }

    }
}

