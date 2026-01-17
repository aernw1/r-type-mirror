/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LevelLoader - JSON-based level loading implementation
*/

#include "ECS/LevelLoader.hpp"
#include "ECS/EnemyFactory.hpp"
#include "Core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace RType {

    namespace ECS {

        LevelData LevelLoader::LoadFromFile(const std::string& path) {
            std::string sourcePath = "../" + path;
            std::ifstream file(sourcePath);

            if (!file.is_open()) {
                file.open(path);
                if (!file.is_open()) {
                    throw std::runtime_error("Failed to open level file: " + path + " (tried: " + sourcePath + " and " + path + ")");
                }
            }

            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            return LoadFromString(content);
        }

        LevelData LevelLoader::LoadFromString(const std::string& jsonString) {
            LevelData level;

            try {
                json j = json::parse(jsonString);

                if (j.contains("name")) {
                    level.name = j["name"].get<std::string>();
                }

                if (j.contains("assets")) {
                    const auto& assets = j["assets"];

                    if (assets.contains("textures")) {
                        for (auto& [key, value] : assets["textures"].items()) {
                            level.textures[key] = value.get<std::string>();
                        }
                    }

                    if (assets.contains("fonts")) {
                        for (auto& [key, value] : assets["fonts"].items()) {
                            FontDef font;
                            if (value.is_object()) {
                                font.path = value.value("path", "");
                                font.size = value.value("size", 16u);
                            } else {
                                font.path = value.get<std::string>();
                            }
                            level.fonts[key] = font;
                        }
                    }
                }

                if (j.contains("background")) {
                    const auto& bg = j["background"];
                    level.background.texture = bg.value("texture", "");
                    level.background.scrollSpeed = bg.value("scrollSpeed", -150.0f);
                    level.background.copies = bg.value("copies", 3);
                    level.background.layer = bg.value("layer", -100);
                }

                if (j.contains("obstacles")) {
                    for (const auto& obs : j["obstacles"]) {
                        ObstacleDef obstacle;
                        obstacle.texture = obs.value("texture", "");

                        if (obs.contains("position")) {
                            obstacle.x = obs["position"].value("x", 0.0f);
                            obstacle.y = obs["position"].value("y", 0.0f);
                        } else {
                            obstacle.x = obs.value("x", 0.0f);
                            obstacle.y = obs.value("y", 0.0f);
                        }

                        obstacle.scaleWidth = obs.value("scaleWidth", 1200.0f);
                        obstacle.scaleHeight = obs.value("scaleHeight", 720.0f);
                        obstacle.scrollSpeed = obs.value("scrollSpeed", -150.0f);
                        obstacle.layer = obs.value("layer", 1);

                        if (obs.contains("colliders")) {
                            for (const auto& col : obs["colliders"]) {
                                ColliderDef collider;
                                collider.x = col.value("x", 0.0f);
                                collider.y = col.value("y", 0.0f);
                                collider.width = col.value("width", 0.0f);
                                collider.height = col.value("height", 0.0f);
                                obstacle.colliders.push_back(collider);
                            }
                        }

                        level.obstacles.push_back(obstacle);
                    }
                }

                if (j.contains("enemies")) {
                    for (const auto& en : j["enemies"]) {
                        EnemyDef enemy;
                        enemy.type = en.value("type", "BASIC");

                        if (en.contains("position")) {
                            enemy.x = en["position"].value("x", 0.0f);
                            enemy.y = en["position"].value("y", 0.0f);
                        } else {
                            enemy.x = en.value("x", 0.0f);
                            enemy.y = en.value("y", 0.0f);
                        }

                        level.enemies.push_back(enemy);
                    }
                }

                if (j.contains("playerSpawns")) {
                    for (const auto& spawn : j["playerSpawns"]) {
                        PlayerSpawnDef ps;
                        ps.x = spawn.value("x", 100.0f);
                        ps.y = spawn.value("y", 360.0f);
                        level.playerSpawns.push_back(ps);
                    }
                }

                if (level.playerSpawns.empty()) {
                    level.playerSpawns.push_back({100.0f, 200.0f});
                    level.playerSpawns.push_back({100.0f, 360.0f});
                    level.playerSpawns.push_back({100.0f, 520.0f});
                    level.playerSpawns.push_back({100.0f, 680.0f});
                }

                if (j.contains("boss")) {
                    const auto& bossJson = j["boss"];
                    BossDef boss;
                    boss.texture = bossJson.value("texture", "");

                    if (bossJson.contains("position")) {
                        boss.x = bossJson["position"].value("x", 0.0f);
                        boss.y = bossJson["position"].value("y", 0.0f);
                    } else {
                        boss.x = bossJson.value("x", 0.0f);
                        boss.y = bossJson.value("y", 0.0f);
                    }

                    boss.width = bossJson.value("width", 200.0f);
                    boss.height = bossJson.value("height", 200.0f);
                    boss.health = bossJson.value("health", 1000);
                    boss.scrollSpeed = bossJson.value("scrollSpeed", -300.0f);
                    boss.attackPattern = bossJson.value("attackPattern", 1);
                    boss.bossId = bossJson.value("bossId", static_cast<uint8_t>(1));

                    level.boss = boss;
                }

                Core::Logger::Info("Loaded level '{}' with {} obstacles, {} enemies, {} player spawns",
                                   level.name.empty() ? "unnamed" : level.name,
                                   level.obstacles.size(),
                                   level.enemies.size(),
                                   level.playerSpawns.size());

            } catch (const json::parse_error& e) {
                throw std::runtime_error("JSON parse error: " + std::string(e.what()));
            } catch (const json::type_error& e) {
                throw std::runtime_error("JSON type error: " + std::string(e.what()));
            }

            return level;
        }

        LoadedAssets LevelLoader::LoadAssets(
            const LevelData& level,
            Renderer::IRenderer* renderer) {
            LoadedAssets assets;

            if (!renderer) {
                return assets;
            }

            for (const auto& [key, path] : level.textures) {
                Renderer::TextureId texId = renderer->LoadTexture("../" + path);
                if (texId == Renderer::INVALID_TEXTURE_ID) {
                    texId = renderer->LoadTexture(path);
                }

                if (texId != Renderer::INVALID_TEXTURE_ID) {
                    assets.textures[key] = texId;
                    assets.sprites[key] = renderer->CreateSprite(texId, {});
                    Core::Logger::Debug("Loaded texture '{}' from '{}'", key, path);
                } else {
                    Core::Logger::Warning("Failed to load texture '{}' from '{}'", key, path);
                }
            }

            for (const auto& [key, fontDef] : level.fonts) {
                Renderer::FontId fontId = renderer->LoadFont("../" + fontDef.path, fontDef.size);
                if (fontId == Renderer::INVALID_FONT_ID) {
                    fontId = renderer->LoadFont(fontDef.path, fontDef.size);
                }

                if (fontId != Renderer::INVALID_FONT_ID) {
                    assets.fonts[key] = fontId;
                    Core::Logger::Debug("Loaded font '{}' from '{}'", key, fontDef.path);
                } else {
                    Core::Logger::Warning("Failed to load font '{}' from '{}'", key, fontDef.path);
                }
            }

            return assets;
        }

        CreatedEntities LevelLoader::CreateEntities(
            Registry& registry,
            const LevelData& level,
            const LoadedAssets& assets,
            Renderer::IRenderer* renderer) {
            CreatedEntities entities;
            uint32_t obstacleIdCounter = 1;

            CreateBackgrounds(registry, level.background, assets, renderer, entities);
            CreateObstacles(registry, level.obstacles, assets, renderer, entities, obstacleIdCounter);
            CreateEnemies(registry, level.enemies, assets, renderer, entities);

            Core::Logger::Info("Created {} backgrounds, {} obstacle visuals, {} obstacle colliders, {} enemy entities",
                               entities.backgrounds.size(),
                               entities.obstacleVisuals.size(),
                               entities.obstacleColliders.size(),
                               entities.enemies.size());

            return entities;
        }

        CreatedEntities LevelLoader::CreateServerEntities(
            Registry& registry,
            const LevelData& level) {
            CreatedEntities entities;
            uint32_t obstacleIdCounter = 1;

            CreateServerObstacles(registry, level.obstacles, entities, obstacleIdCounter);
            CreateServerEnemies(registry, level.enemies, entities);
            CreateServerBoss(registry, level.boss, entities);

            Core::Logger::Info("Created server entities: {} obstacle visuals, {} obstacle colliders, {} enemies, boss: {}",
                               entities.obstacleVisuals.size(),
                               entities.obstacleColliders.size(),
                               entities.enemies.size(),
                               entities.boss != NULL_ENTITY ? "yes" : "no");

            return entities;
        }

        const std::vector<PlayerSpawnDef>& LevelLoader::GetPlayerSpawns(const LevelData& level) {
            return level.playerSpawns;
        }

        EnemyType LevelLoader::ParseEnemyType(const std::string& typeStr) {
            if (typeStr == "BASIC")
                return EnemyType::BASIC;
            if (typeStr == "FAST")
                return EnemyType::FAST;
            if (typeStr == "TANK")
                return EnemyType::TANK;
            if (typeStr == "BOSS")
                return EnemyType::BOSS;
            if (typeStr == "FORMATION")
                return EnemyType::FORMATION;

            Core::Logger::Warning("Unknown enemy type '{}', defaulting to BASIC", typeStr);
            return EnemyType::BASIC;
        }

        void LevelLoader::CreateBackgrounds(
            Registry& registry,
            const BackgroundDef& background,
            const LoadedAssets& assets,
            Renderer::IRenderer* renderer,
            CreatedEntities& entities) {
            if (background.texture.empty()) {
                return;
            }

            auto spriteIt = assets.sprites.find(background.texture);
            auto texIt = assets.textures.find(background.texture);

            if (spriteIt == assets.sprites.end() || texIt == assets.textures.end()) {
                Core::Logger::Warning("Background texture '{}' not found in loaded assets", background.texture);
                return;
            }

            Renderer::SpriteId bgSprite = spriteIt->second;
            Renderer::TextureId bgTexture = texIt->second;

            Math::Vector2 bgSize = renderer->GetTextureSize(bgTexture);
            float scaleX = 1280.0f / bgSize.x;
            float scaleY = 720.0f / bgSize.y;

            for (int i = 0; i < background.copies; i++) {
                Entity bgEntity = registry.CreateEntity();

                registry.AddComponent<Position>(bgEntity, Position{i * 1280.0f, 0.0f});

                auto& drawable = registry.AddComponent<Drawable>(bgEntity, Drawable(bgSprite, background.layer));
                drawable.scale = {scaleX, scaleY};

                registry.AddComponent<Scrollable>(bgEntity, Scrollable(background.scrollSpeed));

                entities.backgrounds.push_back(bgEntity);
            }
        }

        void LevelLoader::CreateObstacles(
            Registry& registry,
            const std::vector<ObstacleDef>& obstacles,
            const LoadedAssets& assets,
            Renderer::IRenderer* renderer,
            CreatedEntities& entities,
            uint32_t& obstacleIdCounter) {
            for (const auto& obs : obstacles) {
                auto spriteIt = assets.sprites.find(obs.texture);
                auto texIt = assets.textures.find(obs.texture);

                bool hasTexture = (spriteIt != assets.sprites.end() && texIt != assets.textures.end());

                Entity obsEntity = NULL_ENTITY;
                if (hasTexture) {
                    obsEntity = registry.CreateEntity();

                    registry.AddComponent<Position>(obsEntity, Position{obs.x, obs.y});

                    Math::Vector2 obsSize = renderer->GetTextureSize(texIt->second);
                    auto& drawable = registry.AddComponent<Drawable>(obsEntity, Drawable(spriteIt->second, obs.layer));
                    drawable.scale = {obs.scaleWidth / obsSize.x, obs.scaleHeight / obsSize.y};
                    drawable.origin = {0.0f, 0.0f};

                    registry.AddComponent<Scrollable>(obsEntity, Scrollable(obs.scrollSpeed));
                    registry.AddComponent<ObstacleVisual>(obsEntity, ObstacleVisual{});
                    entities.obstacleVisuals.push_back(obsEntity);
                } else {
                    Core::Logger::Warning("Obstacle texture '{}' not found in loaded assets", obs.texture);
                }

                for (const auto& col : obs.colliders) {
                    Entity colliderEntity = registry.CreateEntity();
                    // Store collider position as offset from visual entity (not absolute)
                    registry.AddComponent<Position>(colliderEntity, Position{col.x - obs.x, col.y - obs.y});
                    registry.AddComponent<BoxCollider>(colliderEntity, BoxCollider{col.width, col.height});
                    registry.AddComponent<Scrollable>(colliderEntity, Scrollable(obs.scrollSpeed));
                    registry.AddComponent<Obstacle>(colliderEntity, Obstacle(true));
                    registry.AddComponent<ObstacleMetadata>(colliderEntity,
                                                            ObstacleMetadata(obstacleIdCounter++, obsEntity, col.x - obs.x, col.y - obs.y));
                    registry.AddComponent<CollisionLayer>(colliderEntity,
                                                          CollisionLayer(CollisionLayers::OBSTACLE, CollisionLayers::ALL));

                    entities.obstacleColliders.push_back(colliderEntity);
                }
            }
        }

        void LevelLoader::CreateEnemies(
            Registry& registry,
            const std::vector<EnemyDef>& enemies,
            const LoadedAssets& assets,
            Renderer::IRenderer* renderer,
            CreatedEntities& entities) {
            for (const auto& en : enemies) {
                EnemyType type = ParseEnemyType(en.type);
                Entity enemy = EnemyFactory::CreateEnemy(registry, type, en.x, en.y, renderer);
                entities.enemies.push_back(enemy);
            }
        }

        void LevelLoader::CreateServerObstacles(
            Registry& registry,
            const std::vector<ObstacleDef>& obstacles,
            CreatedEntities& entities,
            uint32_t& obstacleIdCounter) {
            for (const auto& obs : obstacles) {
                Entity obsEntity = registry.CreateEntity();

                registry.AddComponent<Position>(obsEntity, Position{obs.x, obs.y});
                registry.AddComponent<Scrollable>(obsEntity, Scrollable(obs.scrollSpeed));
                registry.AddComponent<ObstacleVisual>(obsEntity, ObstacleVisual{});
                entities.obstacleVisuals.push_back(obsEntity);

                for (const auto& col : obs.colliders) {
                    Entity colliderEntity = registry.CreateEntity();
                    // Store collider position as offset from visual entity (not absolute)
                    registry.AddComponent<Position>(colliderEntity, Position{col.x - obs.x, col.y - obs.y});
                    registry.AddComponent<BoxCollider>(colliderEntity, BoxCollider{col.width, col.height});
                    registry.AddComponent<Scrollable>(colliderEntity, Scrollable(obs.scrollSpeed));
                    registry.AddComponent<Obstacle>(colliderEntity, Obstacle(true));
                    registry.AddComponent<ObstacleMetadata>(colliderEntity,
                                                            ObstacleMetadata(obstacleIdCounter++, obsEntity, col.x - obs.x, col.y - obs.y));
                    registry.AddComponent<CollisionLayer>(colliderEntity,
                                                          CollisionLayer(CollisionLayers::OBSTACLE, CollisionLayers::ALL));

                    entities.obstacleColliders.push_back(colliderEntity);
                }
            }
        }

        void LevelLoader::CreateServerEnemies(
            Registry& registry,
            const std::vector<EnemyDef>& enemies,
            CreatedEntities& entities) {
            for (const auto& en : enemies) {
                EnemyType type = ParseEnemyType(en.type);
                Entity enemy = EnemyFactory::CreateEnemy(registry, type, en.x, en.y, nullptr);
                entities.enemies.push_back(enemy);
            }
        }

        void LevelLoader::CreateServerBoss(
            Registry& registry,
            const std::optional<BossDef>& bossOpt,
            CreatedEntities& entities) {
            if (!bossOpt.has_value()) {
                return;
            }

            const BossDef& boss = bossOpt.value();

            Entity bossEntity = registry.CreateEntity();

            registry.AddComponent<Boss>(bossEntity, Boss{boss.bossId});

            registry.AddComponent<Position>(bossEntity, Position{boss.x, boss.y});

            registry.AddComponent<Velocity>(bossEntity, Velocity{0.0f, 0.0f});

            registry.AddComponent<Health>(bossEntity, Health{boss.health});

            registry.AddComponent<BoxCollider>(bossEntity, BoxCollider{boss.width, boss.height});

            registry.AddComponent<Scrollable>(bossEntity, Scrollable{boss.scrollSpeed});

            auto& bossAttack = registry.AddComponent<BossAttack>(bossEntity, BossAttack{3.0f});
            if (boss.bossId == 2) {
                bossAttack = registry.AddComponent<BossAttack>(bossEntity, BossAttack{1.3f});
                bossAttack.currentPattern = BossAttackPattern::ANIMATED_ORB;

                registry.AddComponent<BossMovementPattern>(bossEntity,
                    BossMovementPattern{150.0f, 60.0f, 0.3f, 0.2f, boss.y, boss.x});
            } else {
                bossAttack.currentPattern = static_cast<BossAttackPattern>(boss.attackPattern);
            }

            registry.AddComponent<DamageFlash>(bossEntity, DamageFlash{0.1f});

            registry.AddComponent<CollisionLayer>(bossEntity,
                CollisionLayer(CollisionLayers::ENEMY, CollisionLayers::PLAYER | CollisionLayers::PLAYER_BULLET));

            entities.boss = bossEntity;

            Core::Logger::Info("Created boss entity at position ({}, {}) with {} health",
                boss.x, boss.y, boss.health);
        }

        std::string LevelLoader::SerializeToString(const LevelData& level) {
            json j;

            j["name"] = level.name;

            if (!level.textures.empty() || !level.fonts.empty()) {
                j["assets"] = json::object();

                if (!level.textures.empty()) {
                    j["assets"]["textures"] = json::object();
                    for (const auto& [key, path] : level.textures) {
                        j["assets"]["textures"][key] = path;
                    }
                }

                if (!level.fonts.empty()) {
                    j["assets"]["fonts"] = json::object();
                    for (const auto& [key, font] : level.fonts) {
                        j["assets"]["fonts"][key] = {
                            {"path", font.path},
                            {"size", font.size}
                        };
                    }
                }
            }

            j["background"] = {
                {"texture", level.background.texture},
                {"scrollSpeed", level.background.scrollSpeed},
                {"copies", level.background.copies},
                {"layer", level.background.layer}
            };

            j["obstacles"] = json::array();
            for (const auto& obs : level.obstacles) {
                json obsJson = {
                    {"texture", obs.texture},
                    {"position", {{"x", obs.x}, {"y", obs.y}}},
                    {"scaleWidth", obs.scaleWidth},
                    {"scaleHeight", obs.scaleHeight},
                    {"scrollSpeed", obs.scrollSpeed},
                    {"layer", obs.layer}
                };

                if (!obs.colliders.empty()) {
                    obsJson["colliders"] = json::array();
                    for (const auto& col : obs.colliders) {
                        obsJson["colliders"].push_back({
                            {"x", col.x},
                            {"y", col.y},
                            {"width", col.width},
                            {"height", col.height}
                        });
                    }
                }

                j["obstacles"].push_back(obsJson);
            }

            j["enemies"] = json::array();
            for (const auto& enemy : level.enemies) {
                j["enemies"].push_back({
                    {"type", enemy.type},
                    {"position", {{"x", enemy.x}, {"y", enemy.y}}}
                });
            }

            j["playerSpawns"] = json::array();
            for (const auto& spawn : level.playerSpawns) {
                j["playerSpawns"].push_back({
                    {"x", spawn.x},
                    {"y", spawn.y}
                });
            }

            return j.dump(4);
        }

        void LevelLoader::SaveToFile(const LevelData& level, const std::string& path) {
            try {
                std::string jsonString = SerializeToString(level);

                std::ofstream file(path);
                if (!file.is_open()) {
                    throw std::runtime_error("Failed to open file for writing: " + path);
                }

                file << jsonString;
                file.close();

                Core::Logger::Info("Successfully saved level '{}' to {}",
                                   level.name.empty() ? "unnamed" : level.name,
                                   path);

            } catch (const std::exception& e) {
                Core::Logger::Error("Failed to save level: {}", e.what());
                throw;
            }
        }

    }

}
