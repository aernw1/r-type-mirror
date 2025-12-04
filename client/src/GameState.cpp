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

        GameState::GameState(GameStateMachine& machine, GameContext& context, uint32_t seed)
            : m_machine(machine),
              m_context(context),
              m_gameSeed(seed)
        {
            m_renderer = context.renderer;
        }

        void GameState::Init() {
            std::cout << "[GameState] === Initialisation du jeu ===" << std::endl;

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

        void GameState::loadTextures() {
            loadMapTextures();
        }

        void GameState::loadMapTextures() {
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

        void GameState::createSystems() {
            m_scrollingSystem = std::make_unique<RType::ECS::ScrollingSystem>();
            m_renderingSystem = std::make_unique<RType::ECS::RenderingSystem>(m_renderer.get());
            m_textSystem = std::make_unique<RType::ECS::TextRenderingSystem>(m_renderer.get());
        }

        void GameState::initializeBackground() {
            if (m_bgTexture == Renderer::INVALID_TEXTURE_ID) {
                std::cerr << "[GameState] Error: Background texture not loaded!" << std::endl;
                return;
            }

            m_bgSprite = m_renderer->CreateSprite(m_bgTexture, {});
            auto bgSize = m_renderer->GetTextureSize(m_bgTexture);
            
            //Need to see if we use Types.hpp class with Vectors
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

        void GameState::initializeObstacles() {
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

            ObstacleData obstacles[5] {
                {m_obstacle1Sprite, 1500.0f, 0.0f},
                {m_obstacle2Sprite, 2700.0f, 0.0f},
                {m_obstacle3Sprite, 3900.0f, 0.0f},
                {m_obstacle4Sprite, 5100.0f, 0.0f},
                {m_obstacle5Sprite, 6300.0f, 0.0f}
            };

            ColliderBoxData obstacleColliders[5] = {
                // Obstacle 1
                {0.0f, 80.0f, 180.0f, 280.0f,  220.0f, 120.0f, 210.0f, 240.0f,  480.0f, 60.0f, 240.0f, 300.0f},
                // Obstacle 2
                {0.0f, 60.0f, 200.0f, 260.0f,  260.0f, 30.0f, 190.0f, 290.0f,  500.0f, 90.0f, 180.0f, 230.0f},
                // Obstacle 3
                {0.0f, 45.0f, 220.0f, 310.0f,  280.0f, 0.0f, 250.0f, 340.0f,  570.0f, 75.0f, 240.0f, 290.0f},
                // Obstacle 4
                {0.0f, 110.0f, 210.0f, 270.0f,  260.0f, 65.0f, 240.0f, 315.0f,  550.0f, 125.0f, 210.0f, 255.0f},
                // Obstacle 5
                {0.0f, 95.0f, 220.0f, 260.0f,  270.0f, 50.0f, 230.0f, 305.0f,  540.0f, 110.0f, 230.0f, 245.0f}
            };

            for (int i = 0; i < 5; i++) {
                m_obstacleGameEntity = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(m_obstacleGameEntity, Position{obstacles[i].x, obstacles[i].y});
                
                auto obsSize = m_renderer->GetTextureSize(obstacles[i].sprite);
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
        
        void GameState::HandleInput() {
            // Handle player input (to be implemented)
        }
        
        void GameState::Draw() {
            m_renderingSystem->Update(m_registry, 0.0f);
            m_textSystem->Update(m_registry, 0.0f);
        }
        
        void GameState::Cleanup() {
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

        void GameState::Update(float dt) {
            m_scrollingSystem->Update(m_registry, dt);

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
                if (pos.x <= -1500.0f) {
                    pos.x = pos.x + 5 * 1500.0f;
                }
            }
        }
    }
}
