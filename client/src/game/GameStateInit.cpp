/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameState - Initialization functions
*/

#include "../../include/GameState.hpp"

#include "ECS/Components/TextLabel.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include "ECS/PlayerFactory.hpp"
#include "Animation/AnimationTypes.hpp"

using namespace RType::ECS;
using namespace Animation;

namespace RType {
    namespace Client {

        InGameState::InGameState(GameStateMachine& machine, GameContext& context, uint32_t seed, const std::string& levelPath)
            : m_machine(machine), m_context(context), m_gameSeed(seed), m_currentLevelPath(levelPath) {
            m_renderer = context.renderer;
        }

        void InGameState::Init() {
            Core::Logger::Info("[GameState] Initializing game");

            if (m_context.audio) {
                m_audioSystem = std::make_unique<RType::ECS::AudioSystem>(m_context.audio.get());
                
                m_shootMusic = m_context.audio->LoadMusic("assets/sounds/players_shoot.flac");
                if (m_shootMusic == Audio::INVALID_MUSIC_ID) {
                     m_shootMusic = m_context.audio->LoadMusic("../assets/sounds/players_shoot.flac");
                }
                std::cout << "[DEBUG] Shoot Music ID: " << m_shootMusic << std::endl;
                std::cout << "[DEBUG] Shoot Sound ID: " << m_playerShootSound << std::endl;

                std::cout << "[DEBUG] Loading powerup sound..." << std::endl;
                m_powerUpSound = m_context.audio->LoadSound("assets/sounds/powerup.flac");
                if (m_powerUpSound == Audio::INVALID_SOUND_ID) {
                    m_powerUpSound = m_context.audio->LoadSound("../assets/sounds/powerup.flac");
                }
                m_powerUpMusic = m_context.audio->LoadMusic("assets/sounds/powerup.flac");
                if (m_powerUpMusic == Audio::INVALID_MUSIC_ID) {
                    m_powerUpMusic = m_context.audio->LoadMusic("../assets/sounds/powerup.flac");
                }

                std::cout << "[DEBUG] Loading stage1.flac..." << std::endl;
                m_gameMusic = m_context.audio->LoadMusic("assets/sounds/stage1.flac");
                if (m_gameMusic == Audio::INVALID_MUSIC_ID) {
                    m_gameMusic = m_context.audio->LoadMusic("../assets/sounds/stage1.flac");
                }

                std::cout << "[DEBUG] Loading BOSS.flac..." << std::endl;
                m_bossMusic = m_context.audio->LoadMusic("assets/sounds/BOSS.flac");
                if (m_bossMusic == Audio::INVALID_MUSIC_ID) {
                    m_bossMusic = m_context.audio->LoadMusic("../assets/sounds/BOSS.flac");
                }
                std::cout << "[DEBUG] Game Music ID: " << m_gameMusic << std::endl;

                std::cout << "[DEBUG] Loading gameover.flac..." << std::endl;
                m_gameOverMusic = m_context.audio->LoadMusic("assets/sounds/gameover.flac");
                if (m_gameOverMusic == Audio::INVALID_MUSIC_ID) {
                    m_gameOverMusic = m_context.audio->LoadMusic("../assets/sounds/gameover.flac");
                }
                std::cout << "[DEBUG] GameOver Music ID: " << m_gameOverMusic << std::endl;

                if (m_gameMusic != Audio::INVALID_MUSIC_ID) {
                    auto cmd = m_registry.CreateEntity();
                    auto& me = m_registry.AddComponent<MusicEffect>(cmd, MusicEffect(m_gameMusic));
                    me.play = true;
                    me.stop = false;
                    me.loop = true;
                    me.volume = 0.35f;
                    me.pitch = 1.0f;
                    m_gameMusicPlaying = true;
                }
            }

            if (m_context.networkClient) {
                m_context.networkClient->SetStateCallback([this](uint32_t tick, const std::vector<network::EntityState>& entities, const std::vector<network::InputAck>& inputAcks) { this->OnServerStateUpdate(tick, entities, inputAcks); });
                m_context.networkClient->SetLevelCompleteCallback([this](uint8_t completedLevel, uint8_t nextLevel) { this->OnLevelComplete(completedLevel, nextLevel); });

                if (!m_context.networkClient->IsConnected()) {
                    Core::Logger::Info("[GameState] Reconnecting to server after level transition...");
                    if (!m_context.networkClient->ConnectToServer()) {
                        Core::Logger::Error("[GameState] Failed to reconnect to server!");
                    } else {
                        Core::Logger::Info("[GameState] Reconnected to server successfully");
                    }
                }
            } else {
                Core::Logger::Warning("[GameState] No network client available");
            }

            m_isNetworkSession = (m_context.networkClient != nullptr);

            for (const auto& player : m_context.allPlayers) {
                if (player.hash != 0) {
                    m_playerNameMap[player.hash] = player.name;
                }
                m_playerNameMap[static_cast<uint64_t>(player.number)] = player.name;
            }

            loadLevel(m_currentLevelPath);
            createSystems();

            if (m_shootingSystem && m_playerShootSound != Audio::INVALID_SOUND_ID) {
                m_shootingSystem->SetShootSound(m_playerShootSound);
            }

            if (m_powerUpCollisionSystem && m_powerUpSound != Audio::INVALID_SOUND_ID) {
                m_powerUpCollisionSystem->SetPowerUpSound(m_powerUpSound);
            }

            initializeFromLevel();
            if (!m_isNetworkSession) {
                initializePlayers();
            }
            initializeUI();

            Core::Logger::Info("[GameState] Initialization complete");
        }

        void InGameState::loadLevel(const std::string& levelPath) {
            m_levelData = ECS::LevelLoader::LoadFromFile(levelPath);
            m_levelAssets = ECS::LevelLoader::LoadAssets(m_levelData, m_renderer.get());
            Core::Logger::Info("[GameState] Loaded level '{}' with {} textures, {} obstacle definitions", m_levelData.name, m_levelAssets.textures.size(), m_levelData.obstacles.size());

            // Load explosion spritesheet for death animations
            m_explosionTexture = m_renderer->LoadTexture("assets/SFX/explosion.png");
            if (m_explosionTexture == Renderer::INVALID_TEXTURE_ID) {
                m_explosionTexture = m_renderer->LoadTexture("../assets/SFX/explosion.png");
            }
            if (m_explosionTexture != Renderer::INVALID_TEXTURE_ID) {
                m_explosionSprite = m_renderer->CreateSprite(m_explosionTexture, {});
                Core::Logger::Info("[GameState] Explosion spritesheet loaded");
            } else {
                Core::Logger::Warning("[GameState] Failed to load explosion spritesheet");
            }

            m_enemyBulletGreenTexture = m_renderer->LoadTexture("assets/projectiles/bullet-green.png");
            if (m_enemyBulletGreenTexture == Renderer::INVALID_TEXTURE_ID) {
                m_enemyBulletGreenTexture = m_renderer->LoadTexture("../assets/projectiles/bullet-green.png");
            }
            if (m_enemyBulletGreenTexture != Renderer::INVALID_TEXTURE_ID) {
                m_enemyBulletGreenSprite = m_renderer->CreateSprite(m_enemyBulletGreenTexture, {});
                Core::Logger::Info("[GameState] Enemy bullet green sprite loaded");
            }

            m_enemyBulletYellowTexture = m_renderer->LoadTexture("assets/projectiles/bullet-yellow.png");
            if (m_enemyBulletYellowTexture == Renderer::INVALID_TEXTURE_ID) {
                m_enemyBulletYellowTexture = m_renderer->LoadTexture("../assets/projectiles/bullet-yellow.png");
            }
            if (m_enemyBulletYellowTexture != Renderer::INVALID_TEXTURE_ID) {
                m_enemyBulletYellowSprite = m_renderer->CreateSprite(m_enemyBulletYellowTexture, {});
                Core::Logger::Info("[GameState] Enemy bullet yellow sprite loaded");
            }

            m_enemyBulletPurpleTexture = m_renderer->LoadTexture("assets/projectiles/bullet-purple.png");
            if (m_enemyBulletPurpleTexture == Renderer::INVALID_TEXTURE_ID) {
                m_enemyBulletPurpleTexture = m_renderer->LoadTexture("../assets/projectiles/bullet-purple.png");
            }
            if (m_enemyBulletPurpleTexture != Renderer::INVALID_TEXTURE_ID) {
                m_enemyBulletPurpleSprite = m_renderer->CreateSprite(m_enemyBulletPurpleTexture, {});
                Core::Logger::Info("[GameState] Enemy bullet purple sprite loaded");
            }

            auto enemyGreenIt = m_levelAssets.sprites.find("enemy-green");
            if (enemyGreenIt != m_levelAssets.sprites.end()) {
                m_enemyGreenSprite = enemyGreenIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("enemy-green");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_enemyGreenSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto enemyRedIt = m_levelAssets.sprites.find("enemy-red");
            if (enemyRedIt != m_levelAssets.sprites.end()) {
                m_enemyRedSprite = enemyRedIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("enemy-red");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_enemyRedSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto enemyBlueIt = m_levelAssets.sprites.find("enemy-blue");
            if (enemyBlueIt != m_levelAssets.sprites.end()) {
                m_enemyBlueSprite = enemyBlueIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("enemy-blue");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_enemyBlueSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto ensureEnemySprite = [this](Renderer::SpriteId& target, Renderer::SpriteId fallbackPlayerSprite, const std::string& path) {
                if (target != Renderer::INVALID_SPRITE_ID) {
                    return;
                }
                Renderer::TextureId textureId = m_renderer->LoadTexture(path);
                if (textureId == Renderer::INVALID_TEXTURE_ID) {
                    textureId = m_renderer->LoadTexture("../" + path);
                }
                if (textureId != Renderer::INVALID_TEXTURE_ID) {
                    target = m_renderer->CreateSprite(textureId, {});
                } else if (fallbackPlayerSprite != Renderer::INVALID_SPRITE_ID) {
                    target = fallbackPlayerSprite;
                }
            };

            ensureEnemySprite(m_enemyGreenSprite, m_playerGreenSprite, "assets/spaceships/enemy-green.png");
            ensureEnemySprite(m_enemyRedSprite, m_playerRedSprite, "assets/spaceships/enemy-red.png");
            ensureEnemySprite(m_enemyBlueSprite, m_playerBlueSprite, "assets/spaceships/enemy-blue.png");

            auto powerupSpreadIt = m_levelAssets.sprites.find("powerup-spread");
            if (powerupSpreadIt != m_levelAssets.sprites.end()) {
                m_powerupSpreadSprite = powerupSpreadIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("powerup-spread");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_powerupSpreadSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto powerupLaserIt = m_levelAssets.sprites.find("powerup-laser");
            if (powerupLaserIt != m_levelAssets.sprites.end()) {
                m_powerupLaserSprite = powerupLaserIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("powerup-laser");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_powerupLaserSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto powerupForcePodIt = m_levelAssets.sprites.find("powerup-force-pod");
            if (powerupForcePodIt != m_levelAssets.sprites.end()) {
                m_powerupForcePodSprite = powerupForcePodIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("powerup-force-pod");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_powerupForcePodSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto powerupSpeedIt = m_levelAssets.sprites.find("powerup-speed");
            if (powerupSpeedIt != m_levelAssets.sprites.end()) {
                m_powerupSpeedSprite = powerupSpeedIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("powerup-speed");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_powerupSpeedSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }

            auto powerupShieldIt = m_levelAssets.sprites.find("powerup-shield");
            if (powerupShieldIt != m_levelAssets.sprites.end()) {
                m_powerupShieldSprite = powerupShieldIt->second;
            } else {
                auto textureIt = m_levelAssets.textures.find("powerup-shield");
                if (textureIt != m_levelAssets.textures.end()) {
                    m_powerupShieldSprite = m_renderer->CreateSprite(textureIt->second, {});
                }
            }
        }

        void InGameState::initializeFromLevel() {
            m_levelEntities = ECS::LevelLoader::CreateEntities(
                m_registry,
                m_levelData,
                m_levelAssets,
                m_renderer.get());

            m_backgroundEntities = m_levelEntities.backgrounds;
            m_obstacleSpriteEntities = m_levelEntities.obstacleVisuals;
            m_obstacleColliderEntities = m_levelEntities.obstacleColliders;
            m_obstacleIdToCollider.clear();

            // Initialize collider positions based on their visual entities
            for (auto collider : m_obstacleColliderEntities) {
                if (!m_registry.IsEntityAlive(collider) ||
                    !m_registry.HasComponent<ECS::ObstacleMetadata>(collider)) {
                    continue;
                }
                const auto& metadata = m_registry.GetComponent<ECS::ObstacleMetadata>(collider);
                m_obstacleIdToCollider[metadata.uniqueId] = collider;

                // Sync collider to visual entity position on initialization
                if (metadata.visualEntity != ECS::NULL_ENTITY &&
                    m_registry.IsEntityAlive(metadata.visualEntity) &&
                    m_registry.HasComponent<ECS::Position>(metadata.visualEntity) &&
                    m_registry.HasComponent<ECS::Position>(collider)) {

                    const auto& visualPos = m_registry.GetComponent<ECS::Position>(metadata.visualEntity);
                    auto& colliderPos = m_registry.GetComponent<ECS::Position>(collider);

                    // Set absolute position = visual position + offset
                    colliderPos.x = visualPos.x + metadata.offsetX;
                    colliderPos.y = visualPos.y + metadata.offsetY;

                }
            }

            std::cout << "[INIT] Built m_obstacleIdToCollider map with " << m_obstacleIdToCollider.size()
                      << " entries from " << m_obstacleColliderEntities.size() << " collider entities" << std::endl;
        }

        void InGameState::createSystems() {
            auto bulletSpriteIt = m_levelAssets.sprites.find("bullet");
            Renderer::SpriteId bulletSprite = (bulletSpriteIt != m_levelAssets.sprites.end()) ? bulletSpriteIt->second : Renderer::INVALID_SPRITE_ID;

            if (bulletSprite == Renderer::INVALID_SPRITE_ID) {
                Core::Logger::Error("[GameState] Bullet sprite not found in level assets");
            }

            m_scrollingSystem = std::make_unique<RType::ECS::ScrollingSystem>();
            m_renderingSystem = std::make_unique<RType::ECS::RenderingSystem>(m_renderer.get());
            m_textSystem = std::make_unique<RType::ECS::TextRenderingSystem>(m_renderer.get());

            // Initialize animation module and system
            m_animationModule = std::make_unique<Animation::AnimationModule>();

            // Create explosion animation clip (11 frames, 32x32 each, horizontal strip)
            if (m_explosionTexture != Renderer::INVALID_TEXTURE_ID) {
                Animation::GridLayout layout;
                layout.columns = 11;
                layout.rows = 1;
                layout.frameCount = 11;
                layout.frameWidth = 32.0f;
                layout.frameHeight = 32.0f;
                layout.defaultDuration = 0.05f;  // 50ms per frame

                m_explosionClipId = m_animationModule->CreateClipFromGrid(
                    "explosion_small",
                    "assets/SFX/explosion.png",
                    layout,
                    false);
                Core::Logger::Info("[GameState] Created explosion animation clip with {} frames", 11);
            }

            m_animationSystem = std::make_unique<RType::ECS::AnimationSystem>(m_animationModule.get());

            // Initialize effect factory with explosion clip
            ECS::EffectConfig effectConfig;
            effectConfig.explosionSmall = m_explosionClipId;
            effectConfig.effectsTexture = m_explosionTexture;
            effectConfig.effectsSprite = m_explosionSprite;
            m_effectFactory = std::make_unique<RType::ECS::EffectFactory>(effectConfig);

            m_forcePodSystem = std::make_unique<RType::ECS::ForcePodSystem>();
            m_shieldSystem = std::make_unique<RType::ECS::ShieldSystem>();

            if (!m_isNetworkSession) {
                m_powerUpSpawnSystem = std::make_unique<RType::ECS::PowerUpSpawnSystem>(
                    m_renderer.get(),
                    m_levelData.config.screenWidth,
                    m_levelData.config.screenHeight);
                m_powerUpSpawnSystem->SetSpawnInterval(m_levelData.config.powerUpSpawnInterval);
                m_powerUpCollisionSystem = std::make_unique<RType::ECS::PowerUpCollisionSystem>(m_renderer.get());
                m_collisionDetectionSystem = std::make_unique<RType::ECS::CollisionDetectionSystem>();
                m_playerResponseSystem = std::make_unique<RType::ECS::PlayerCollisionResponseSystem>();
                m_shootingSystem = std::make_unique<RType::ECS::ShootingSystem>(bulletSprite);
                m_movementSystem = std::make_unique<RType::ECS::MovementSystem>();
                m_inputSystem = std::make_unique<RType::ECS::InputSystem>(m_renderer.get());
                m_bulletResponseSystem = std::make_unique<RType::ECS::BulletCollisionResponseSystem>();
                m_obstacleResponseSystem = std::make_unique<RType::ECS::ObstacleCollisionResponseSystem>();
                m_healthSystem = std::make_unique<RType::ECS::HealthSystem>();
                m_scoreSystem = std::make_unique<RType::ECS::ScoreSystem>();
            } else {
                m_powerUpSpawnSystem.reset();
                m_powerUpCollisionSystem.reset();
                m_collisionDetectionSystem.reset();
                m_playerResponseSystem.reset();
                m_shootingSystem.reset();
                m_movementSystem.reset();
                m_inputSystem.reset();
                m_bulletResponseSystem.reset();
                m_obstacleResponseSystem.reset();
                m_healthSystem.reset();
                m_scoreSystem.reset();
            }
        }

        void InGameState::initializePlayers() {
            if (m_isNetworkSession) {
                return;
            }
            std::cout << "[GameState] Initializing local player (ECS)..." << std::endl;

            const uint8_t playerNumber = m_context.playerNumber == 0 ? 1 : m_context.playerNumber;
            const uint64_t playerHash = m_context.playerHash;

            const auto& spawns = ECS::LevelLoader::GetPlayerSpawns(m_levelData);
            uint8_t playerIndex = playerNumber - 1;

            Position spawnPos{100.0f, 360.0f};
            if (!spawns.empty()) {
                if (playerIndex < spawns.size()) {
                    spawnPos = Position{spawns[playerIndex].x, spawns[playerIndex].y};
                    Core::Logger::Info("[GameState] Player {} spawning at ({}, {}) from level data", playerNumber, spawnPos.x, spawnPos.y);
                } else {
                    spawnPos = Position{spawns[0].x, spawns[0].y};
                    Core::Logger::Warning("[GameState] Player {} index out of range, using first spawn", playerNumber);
                }
            } else {
                Core::Logger::Warning("[GameState] No player spawns in level data, using default position");
            }

            const auto& playerConfig = m_levelData.config.playerDefaults;

            m_localPlayerEntity = ECS::PlayerFactory::CreatePlayer(m_registry, playerNumber, playerHash, spawnPos.x, spawnPos.y, m_renderer.get());

            if (m_localPlayerEntity == ECS::NULL_ENTITY) {
                Core::Logger::Error("[GameState] Failed to create local player entity");
                return;
            }

            if (m_registry.HasComponent<ECS::Player>(m_localPlayerEntity)) {
                auto& playerComp = m_registry.GetComponent<ECS::Player>(m_localPlayerEntity);
                playerComp.isLocalPlayer = true;
            } else {
                m_registry.AddComponent<Player>(m_localPlayerEntity, Player{playerNumber, playerHash, true});
            }

            if (m_registry.HasComponent<ECS::Position>(m_localPlayerEntity)) {
                auto& position = m_registry.GetComponent<ECS::Position>(m_localPlayerEntity);
                position = spawnPos;
            } else {
                m_registry.AddComponent<Position>(m_localPlayerEntity, Position{spawnPos.x, spawnPos.y});
            }

            if (!m_registry.HasComponent<Velocity>(m_localPlayerEntity)) {
                m_registry.AddComponent<Velocity>(m_localPlayerEntity, Velocity{0.0f, 0.0f});
            }

            if (m_registry.HasComponent<ECS::Controllable>(m_localPlayerEntity)) {
                auto& controllable = m_registry.GetComponent<ECS::Controllable>(m_localPlayerEntity);
                controllable.speed = playerConfig.movementSpeed;
            } else {
                m_registry.AddComponent<Controllable>(m_localPlayerEntity, Controllable{playerConfig.movementSpeed});
            }

            Shooter shooterConfig{
                playerConfig.fireRate,
                playerConfig.bulletOffsetX,
                playerConfig.bulletOffsetY};
            if (m_registry.HasComponent<Shooter>(m_localPlayerEntity)) {
                auto& shooter = m_registry.GetComponent<Shooter>(m_localPlayerEntity);
                shooter = shooterConfig;
            } else {
                m_registry.AddComponent<Shooter>(m_localPlayerEntity, Shooter{shooterConfig.fireRate, shooterConfig.offsetX, shooterConfig.offsetY});
            }

            if (m_registry.HasComponent<ShootCommand>(m_localPlayerEntity)) {
                auto& shootCommand = m_registry.GetComponent<ShootCommand>(m_localPlayerEntity);
                shootCommand = ShootCommand{};
            } else {
                m_registry.AddComponent<ShootCommand>(m_localPlayerEntity, ShootCommand{});
            }

            if (m_registry.HasComponent<Health>(m_localPlayerEntity)) {
                auto& health = m_registry.GetComponent<Health>(m_localPlayerEntity);
                health.max = playerConfig.maxHealth;
                if (health.current > playerConfig.maxHealth) {
                    health.current = playerConfig.maxHealth;
                }
            } else {
                m_registry.AddComponent<Health>(m_localPlayerEntity, Health{playerConfig.maxHealth});
            }

            auto spriteIt = m_levelAssets.sprites.find("player_blue");
            if (spriteIt != m_levelAssets.sprites.end()) {
                if (m_registry.HasComponent<Drawable>(m_localPlayerEntity)) {
                    auto& drawable = m_registry.GetComponent<Drawable>(m_localPlayerEntity);
                    drawable.spriteId = spriteIt->second;
                    drawable.layer = 10;
                    drawable.scale = {0.5f, 0.5f};
                } else {
                    auto& drawable = m_registry.AddComponent<Drawable>(m_localPlayerEntity, Drawable(spriteIt->second, 10));
                    drawable.scale = {0.5f, 0.5f};
                }
            }
        }

    }
}
