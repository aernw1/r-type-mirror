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
            m_rock1Texture = m_renderer->LoadTexture("assets/backgrounds/obstacles/obstacle_first.png");
            if (m_rock1Texture == Renderer::INVALID_TEXTURE_ID) {
                m_rock1Texture = m_renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_first.png");
            }

            m_rock2Texture = m_renderer->LoadTexture("assets/backgrounds/obstacles/obstacle_two.png");
            if (m_rock2Texture == Renderer::INVALID_TEXTURE_ID) {
                m_rock2Texture = m_renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_two.png");
            }

            m_rock3Texture = m_renderer->LoadTexture("assets/backgrounds/obstacles/obstacle_three.png");
            if (m_rock3Texture == Renderer::INVALID_TEXTURE_ID) {
                m_rock3Texture = m_renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_three.png");
            }

            m_rock4Texture = m_renderer->LoadTexture("assets/backgrounds/obstacles/obstacle_four.png");
            if (m_rock4Texture == Renderer::INVALID_TEXTURE_ID) {
                m_rock4Texture = m_renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_four.png");
            }
            m_rock5Texture = m_renderer->LoadTexture("assets/backgrounds/obstacles/obstacle_five.png");
            if (m_rock5Texture == Renderer::INVALID_TEXTURE_ID) {
                m_rock5Texture = m_renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_five.png");
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
    }
}
