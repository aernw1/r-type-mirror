/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameState
*/

#pragma once

#include "GameStateMachine.hpp"
#include "ECS/Registry.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/TextRenderingSystem.hpp"
#include "ECS/ScrollingSystem.hpp"
#include "ECS/Component.hpp"
#include "Renderer/IRenderer.hpp"

#include <memory>
#include <string>
#include <vector>

namespace RType {
    namespace Client {

        struct ObstacleData {
            Renderer::SpriteId sprite;
            float x;
            float y;
        };

        struct ObstacleTextureData {
            Renderer::TextureId texture;
        };

        struct ColliderBoxData {
            float x1, y1, w1, h1;
            float x2, y2, w2, h2;
            float x3, y3, w3, h3;
        };

        class InGameState : public IState {
        public:
            InGameState(GameStateMachine& machine, GameContext& context, uint32_t seed);
            ~InGameState() override = default;

            void Init() override;
            void Cleanup() override;
            void HandleInput() override;
            void Update(float dt) override;
            void Draw() override;

        private:
            void loadTextures();
            void loadMapTextures();
            void initializeBackground();
            void initializeObstacles();

            // Player System (Keane)
            void initializePlayers();

            // Ennemy System (Dryss)
             void spawnEnemies();
            
            // Score UI (Matthieu)
            void initializeUI();

            // ECS systems
            void createSystems();

        private:
            GameStateMachine& m_machine;
            GameContext& m_context;
            uint32_t m_gameSeed;

            RType::ECS::Registry m_registry;
            std::shared_ptr<Renderer::IRenderer> m_renderer;

            // Systems
            std::unique_ptr<RType::ECS::ScrollingSystem> m_scrollingSystem;
            std::unique_ptr<RType::ECS::RenderingSystem> m_renderingSystem;
            std::unique_ptr<RType::ECS::TextRenderingSystem> m_textSystem;

            // Textures pour background et obstacles
            Renderer::TextureId m_bgTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId m_bgSprite = Renderer::INVALID_SPRITE_ID;

            Renderer::TextureId m_obstacle1Texture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_obstacle2Texture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_obstacle3Texture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_obstacle4Texture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_obstacle5Texture = Renderer::INVALID_TEXTURE_ID;

            Renderer::SpriteId m_obstacle1Sprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_obstacle2Sprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_obstacle3Sprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_obstacle4Sprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_obstacle5Sprite = Renderer::INVALID_SPRITE_ID;

            RType::ECS::Entity m_bgGameEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_obstacleGameEntity = RType::ECS::NULL_ENTITY;

            // Background and obstacles entities
            std::vector<RType::ECS::Entity> m_backgroundEntities;
            std::vector<RType::ECS::Entity> m_obstacleEntities;

            bool m_escapeKeyPressed = false;
        };

    }
}
