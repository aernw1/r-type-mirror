/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameState
*/

#include "../include/GameState.hpp"

#include <iostream>

using namespace RType::ECS;

namespace RType {
    namespace Client {

        InGameState::InGameState(GameStateMachine& machine, GameContext& context, uint32_t seed)
            : m_machine(machine),
              m_context(context),
              m_gameSeed(seed) {
            m_renderer = context.renderer;
        }

        void InGameState::Init() {
            std::cout << "[GameState] === Initialisation du jeu ===" << std::endl;

            // Network callback setup
            if (m_context.networkClient) {
                m_context.networkClient->SetStateCallback([this](uint32_t tick, const std::vector<network::EntityState>& entities) { this->OnServerStateUpdate(tick, entities); });
                std::cout << "[GameState] Network callback registered" << std::endl;
            } else {
                std::cout << "[GameState] WARNING: No network client available!" << std::endl;
            }

            std::cout << "[GameState] Étape 1/5: Textures Loading" << std::endl;
            loadTextures();

            std::cout << "[GameState] Étape 2/5: ECS Systems creation" << std::endl;
            createSystems();

            std::cout << "[GameState] Étape 3/5: Background Creation" << std::endl;
            initializeBackground();

            std::cout << "[GameState] Étape 4/5: Obstacles Creation" << std::endl;
            initializeObstacles();

            std::cout << "[GameState] Étape 5/5: Player Init and UI" << std::endl;
            // initializePlayers();  // Keane
            // initializeUI();       // Matthieu

            std::cout << "[GameState] === Initialisation terminée! ===" << std::endl;
        }

        void InGameState::loadTextures() {
            loadMapTextures();

            // Load individual player ship textures (green, blue, red)
            m_playerGreenTexture = m_renderer->LoadTexture("assets/spaceships/player_green.png");
            if (m_playerGreenTexture == Renderer::INVALID_TEXTURE_ID) {
                m_playerGreenTexture = m_renderer->LoadTexture("../assets/spaceships/player_green.png");
            }
            if (m_playerGreenTexture != Renderer::INVALID_TEXTURE_ID) {
                m_playerGreenSprite = m_renderer->CreateSprite(m_playerGreenTexture, {});
                std::cout << "[GameState] Green player sprite loaded" << std::endl;
            }

            m_playerBlueTexture = m_renderer->LoadTexture("assets/spaceships/player_blue.png");
            if (m_playerBlueTexture == Renderer::INVALID_TEXTURE_ID) {
                m_playerBlueTexture = m_renderer->LoadTexture("../assets/spaceships/player_blue.png");
            }
            if (m_playerBlueTexture != Renderer::INVALID_TEXTURE_ID) {
                m_playerBlueSprite = m_renderer->CreateSprite(m_playerBlueTexture, {});
                std::cout << "[GameState] Blue player sprite loaded" << std::endl;
            }

            m_playerRedTexture = m_renderer->LoadTexture("assets/spaceships/player_red.png");
            if (m_playerRedTexture == Renderer::INVALID_TEXTURE_ID) {
                m_playerRedTexture = m_renderer->LoadTexture("../assets/spaceships/player_red.png");
            }
            if (m_playerRedTexture != Renderer::INVALID_TEXTURE_ID) {
                m_playerRedSprite = m_renderer->CreateSprite(m_playerRedTexture, {});
                std::cout << "[GameState] Red player sprite loaded" << std::endl;
            }
        }

        void InGameState::loadMapTextures() {
            m_obstacle1Texture = m_renderer->LoadTexture("assets/backgrounds/obstacles/obstacle_first.png");
            if (m_obstacle1Texture == Renderer::INVALID_TEXTURE_ID) {
                m_obstacle1Texture = m_renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_first.png");
            }

            m_obstacle2Texture = m_renderer->LoadTexture("assets/backgrounds/obstacles/obstacle_two.png");
            if (m_obstacle2Texture == Renderer::INVALID_TEXTURE_ID) {
                m_obstacle2Texture = m_renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_two.png");
            }

            m_obstacle3Texture = m_renderer->LoadTexture("assets/backgrounds/obstacles/obstacle_three.png");
            if (m_obstacle3Texture == Renderer::INVALID_TEXTURE_ID) {
                m_obstacle3Texture = m_renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_three.png");
            }

            m_obstacle4Texture = m_renderer->LoadTexture("assets/backgrounds/obstacles/obstacle_four.png");
            if (m_obstacle4Texture == Renderer::INVALID_TEXTURE_ID) {
                m_obstacle4Texture = m_renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_four.png");
            }
            m_obstacle5Texture = m_renderer->LoadTexture("assets/backgrounds/obstacles/obstacle_five.png");
            if (m_obstacle5Texture == Renderer::INVALID_TEXTURE_ID) {
                m_obstacle5Texture = m_renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_five.png");
            }
            m_bgTexture = m_renderer->LoadTexture("assets/backgrounds/Cave_one.png");
            if (m_bgTexture == Renderer::INVALID_TEXTURE_ID) {
                m_bgTexture = m_renderer->LoadTexture("../assets/backgrounds/Cave_one.png");
            }
        }

        void InGameState::createSystems() {
            m_scrollingSystem = std::make_unique<RType::ECS::ScrollingSystem>();
            m_renderingSystem = std::make_unique<RType::ECS::RenderingSystem>(m_renderer.get());
            m_textSystem = std::make_unique<RType::ECS::TextRenderingSystem>(m_renderer.get());
        }

        void InGameState::initializeBackground() {
            if (m_bgTexture == Renderer::INVALID_TEXTURE_ID) {
                std::cerr << "[GameState] Error: Background texture not loaded!" << std::endl;
                return;
            }

            m_bgSprite = m_renderer->CreateSprite(m_bgTexture, {});
            auto bgSize = m_renderer->GetTextureSize(m_bgTexture);

            // Need to see if we use Types.hpp class with Vectors
            float scaleX = 1280.0f / bgSize.x;
            float scaleY = 720.0f / bgSize.y;

            for (int i = 0; i < 3; i++) {
                m_bgGameEntity = m_registry.CreateEntity();

                m_registry.AddComponent<Position>(m_bgGameEntity, Position{i * 1280.0f, 0.0f});
                auto& bgDrawable = m_registry.AddComponent<Drawable>(m_bgGameEntity, Drawable(m_bgSprite, -100));
                bgDrawable.scale = {scaleX, scaleY};
                m_registry.AddComponent<Scrollable>(m_bgGameEntity, Scrollable(-150.0f));

                m_backgroundEntities.push_back(m_bgGameEntity);
            }
        }

        void InGameState::initializeObstacles() {
            if (m_obstacle1Texture == Renderer::INVALID_TEXTURE_ID ||
                m_obstacle2Texture == Renderer::INVALID_TEXTURE_ID ||
                m_obstacle3Texture == Renderer::INVALID_TEXTURE_ID ||
                m_obstacle4Texture == Renderer::INVALID_TEXTURE_ID ||
                m_obstacle5Texture == Renderer::INVALID_TEXTURE_ID) {
                std::cerr << "[GameState] Error: One or more obstacle textures not loaded!" << std::endl;
                return;
            }

            m_obstacle1Sprite = m_renderer->CreateSprite(m_obstacle1Texture, {});
            m_obstacle2Sprite = m_renderer->CreateSprite(m_obstacle2Texture, {});
            m_obstacle3Sprite = m_renderer->CreateSprite(m_obstacle3Texture, {});
            m_obstacle4Sprite = m_renderer->CreateSprite(m_obstacle4Texture, {});
            m_obstacle5Sprite = m_renderer->CreateSprite(m_obstacle5Texture, {});

            float width = 1200.0f;
            float height = 720.0f;

            ObstacleData obstacles[5]{
                {m_obstacle1Sprite, 1500.0f, 0.0f},
                {m_obstacle2Sprite, 2700.0f, 0.0f},
                {m_obstacle3Sprite, 3900.0f, 0.0f},
                {m_obstacle4Sprite, 5100.0f, 0.0f},
                {m_obstacle5Sprite, 6300.0f, 0.0f}};

            ObstacleTextureData obstacleTextures[5] = {
                {m_obstacle1Texture},
                {m_obstacle2Texture},
                {m_obstacle3Texture},
                {m_obstacle4Texture},
                {m_obstacle5Texture}};

            ColliderBoxData obstacleColliders[5] = {
                // Obstacle 1
                {0.0f, 80.0f, 180.0f, 280.0f, 220.0f, 120.0f, 210.0f, 240.0f, 480.0f, 60.0f, 240.0f, 300.0f},
                // Obstacle 2
                {0.0f, 60.0f, 200.0f, 260.0f, 260.0f, 30.0f, 190.0f, 290.0f, 500.0f, 90.0f, 180.0f, 230.0f},
                // Obstacle 3
                {0.0f, 45.0f, 220.0f, 310.0f, 280.0f, 0.0f, 250.0f, 340.0f, 570.0f, 75.0f, 240.0f, 290.0f},
                // Obstacle 4
                {0.0f, 110.0f, 210.0f, 270.0f, 260.0f, 65.0f, 240.0f, 315.0f, 550.0f, 125.0f, 210.0f, 255.0f},
                // Obstacle 5
                {0.0f, 95.0f, 220.0f, 260.0f, 270.0f, 50.0f, 230.0f, 305.0f, 540.0f, 110.0f, 230.0f, 245.0f}};

            for (int i = 0; i < 5; i++) {
                m_obstacleGameEntity = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(m_obstacleGameEntity, Position{obstacles[i].x, obstacles[i].y});

                auto obsSize = m_renderer->GetTextureSize(obstacleTextures[i].texture);
                auto& obstacleDrawable = m_registry.AddComponent<Drawable>(m_obstacleGameEntity, Drawable(obstacles[i].sprite, 1));
                obstacleDrawable.scale = {width / obsSize.x, height / obsSize.y};

                auto& collider = m_registry.AddComponent<MultiBoxCollider>(m_obstacleGameEntity);
                collider.AddBox(obstacleColliders[i].x1, obstacleColliders[i].y1,
                                obstacleColliders[i].w1, obstacleColliders[i].h1);
                collider.AddBox(obstacleColliders[i].x2, obstacleColliders[i].y2,
                                obstacleColliders[i].w2, obstacleColliders[i].h2);
                collider.AddBox(obstacleColliders[i].x3, obstacleColliders[i].y3,
                                obstacleColliders[i].w3, obstacleColliders[i].h3);

                m_registry.AddComponent<Scrollable>(m_obstacleGameEntity, Scrollable(-150.0f));
                m_registry.AddComponent<Obstacle>(m_obstacleGameEntity, Obstacle(true));

                m_obstacleEntities.push_back(m_obstacleGameEntity);
            }
        }

        void InGameState::HandleInput() {
            // LOCKSTEP: Capture inputs and send to server (server will set Velocity)
            m_currentInputs = 0;

            if (m_renderer->IsKeyPressed(Renderer::Key::Up)) {
                m_currentInputs |= network::InputFlags::UP;
            }
            if (m_renderer->IsKeyPressed(Renderer::Key::Down)) {
                m_currentInputs |= network::InputFlags::DOWN;
            }
            if (m_renderer->IsKeyPressed(Renderer::Key::Left)) {
                m_currentInputs |= network::InputFlags::LEFT;
            }
            if (m_renderer->IsKeyPressed(Renderer::Key::Right)) {
                m_currentInputs |= network::InputFlags::RIGHT;
            }
            if (m_renderer->IsKeyPressed(Renderer::Key::Space)) {
                m_currentInputs |= network::InputFlags::SHOOT;
            }

            // Send inputs to server EVERY frame
            if (m_context.networkClient && m_currentInputs != 0) {
                auto now = std::chrono::steady_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
                static int inputLog = 0;
                if (inputLog++ % 10 == 0) {
                    std::cout << "[CLIENT SEND INPUT] t=" << ms << " inputs=" << (int)m_currentInputs << std::endl;
                }
                m_context.networkClient->SendInput(m_currentInputs);
            }

            // Escape to return to lobby
            if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escapeKeyPressed) {
                m_escapeKeyPressed = true;
                std::cout << "[GameState] Returning to lobby..." << std::endl;
                m_machine.PopState();
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                m_escapeKeyPressed = false;
            }
        }

        void InGameState::Draw() {
            m_renderingSystem->Update(m_registry, 0.0f);
            m_textSystem->Update(m_registry, 0.0f);
        }

        void InGameState::Cleanup() {
            std::cout << "[GameState] Cleaning up game state..." << std::endl;

            for (auto& bg : m_backgroundEntities) {
                if (m_registry.IsEntityAlive(bg)) {
                    m_registry.DestroyEntity(bg);
                }
            }
            m_backgroundEntities.clear();

            for (auto& obstacle : m_obstacleEntities) {
                if (m_registry.IsEntityAlive(obstacle)) {
                    m_registry.DestroyEntity(obstacle);
                }
            }
            m_obstacleEntities.clear();
        }

        void InGameState::OnServerStateUpdate(uint32_t tick, const std::vector<network::EntityState>& entities) {
            auto now = std::chrono::steady_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

            static int stateLog = 0;
            if (stateLog++ % 60 == 0) {
                std::cout << "[CLIENT RECV STATE] t=" << ms << " tick=" << tick << " entities=" << entities.size() << std::endl;
            }

            // get scroll offset from server
            if (m_context.networkClient) {
                m_serverScrollOffset = m_context.networkClient->GetLastScrollOffset();
            }

            // Update/Create player ships from server entities
            for (const auto& entityState : entities) {
                // Only process PLAYER entities
                if (static_cast<network::EntityType>(entityState.entityType) != network::EntityType::PLAYER) {
                    continue;
                }

                // Check if we already have this entity in our registry
                auto it = m_networkEntityMap.find(entityState.entityId);

                if (it == m_networkEntityMap.end()) {
                    // CREATE new player entity - choose sprite based on player index (round-robin)
                    Renderer::SpriteId playerSprite = Renderer::INVALID_SPRITE_ID;
                    size_t playerIndex = m_networkEntityMap.size() % 3;

                    if (playerIndex == 0 && m_playerGreenSprite != Renderer::INVALID_SPRITE_ID) {
                        playerSprite = m_playerGreenSprite;
                    } else if (playerIndex == 1 && m_playerBlueSprite != Renderer::INVALID_SPRITE_ID) {
                        playerSprite = m_playerBlueSprite;
                    } else if (playerIndex == 2 && m_playerRedSprite != Renderer::INVALID_SPRITE_ID) {
                        playerSprite = m_playerRedSprite;
                    }

                    if (playerSprite == Renderer::INVALID_SPRITE_ID) {
                        continue; // Can't create without sprite
                    }

                    auto newEntity = m_registry.CreateEntity();
                    m_registry.AddComponent<Position>(newEntity, Position{entityState.x, entityState.y});
                    m_registry.AddComponent<Velocity>(newEntity, Velocity{entityState.vx, entityState.vy});

                    auto& drawable = m_registry.AddComponent<Drawable>(newEntity, Drawable(playerSprite, 10)); // Layer 10 = on top
                    drawable.scale = {0.25f, 0.25f}; // Divide sprite size by 4

                    m_networkEntityMap[entityState.entityId] = newEntity;

                    // Track if this is the local player (for client-side prediction)
                    if (entityState.ownerHash == m_context.playerHash) {
                        m_localPlayerEntity = newEntity;
                        std::cout << "[GameState] ✓ Local player ready - client-side prediction enabled" << std::endl;
                    }

                    std::cout << "[GameState] Created player entity " << entityState.entityId << " with color index " << playerIndex << std::endl;
                } else {
                    // UPDATE existing entity - LOCKSTEP: server is authoritative, apply ALL updates
                    auto ecsEntity = it->second;

                    if (m_registry.HasComponent<Position>(ecsEntity)) {
                        auto& pos = m_registry.GetComponent<Position>(ecsEntity);
                        pos.x = entityState.x;
                        pos.y = entityState.y;
                    }

                    if (m_registry.HasComponent<Velocity>(ecsEntity)) {
                        auto& vel = m_registry.GetComponent<Velocity>(ecsEntity);
                        vel.dx = entityState.vx;
                        vel.dy = entityState.vy;
                    }
                }
            }
        }

        void InGameState::Update(float dt) {
            // LOCKSTEP: Receive server updates FIRST
            if (m_context.networkClient) {
                m_context.networkClient->ReceivePackets();
            }

            // Then move based on server-authoritative Velocity
            auto entities = m_registry.GetEntitiesWithComponent<Position>();
            for (auto entity : entities) {
                if (m_registry.HasComponent<Velocity>(entity)) {
                    auto& pos = m_registry.GetComponent<Position>(entity);
                    auto& vel = m_registry.GetComponent<Velocity>(entity);

                    // Apply velocity to position
                    pos.x += vel.dx * dt;
                    pos.y += vel.dy * dt;

                    // Clamp to screen bounds (only for players)
                    if (entity == m_localPlayerEntity) {
                        pos.x = std::max(0.0f, std::min(pos.x, 1280.0f - 66.0f));
                        pos.y = std::max(0.0f, std::min(pos.y, 720.0f - 32.0f));
                    }
                }
            }

            m_scrollingSystem->Update(m_registry, dt);
            m_localScrollOffset += -150.0f * dt;

            const float obstacleWidth = 1200.0f;
            const float obstacleSpacing = 1200.0f;
            const float firstObstacleX = 1500.0f;
            const float lastObstacleX = 6300.0f;
            const float totalObstacleDistance = lastObstacleX - firstObstacleX + obstacleSpacing;

            for (auto& bg : m_backgroundEntities) {
                if (!m_registry.HasComponent<Position>(bg))
                    continue;
                // Loop of background
                auto& pos = m_registry.GetComponent<Position>(bg);
                if (pos.x <= -1280.0f) {
                    pos.x = pos.x + 3 * 1280.0f;
                }
            }

            for (auto& obstacle : m_obstacleEntities) {
                if (!m_registry.HasComponent<Position>(obstacle))
                    continue;
                // Loop of obstacles
                auto& pos = m_registry.GetComponent<Position>(obstacle);
                if (pos.x <= -obstacleWidth) {
                    pos.x += totalObstacleDistance;
                }
            }
        }
    }
}
