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

        class GameState : public IState {
        public:
            GameState(GameStateMachine& machine, GameContext& context, uint32_t seed);
            ~GameState() override = default;

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

            Renderer::TextureId m_rock1Texture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_rock2Texture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_rock3Texture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_rock4Texture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_rock5Texture = Renderer::INVALID_TEXTURE_ID;

            Renderer::SpriteId m_rock1Sprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_rock2Sprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_rock3Sprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_rock4Sprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_rock5Sprite = Renderer::INVALID_SPRITE_ID;

            RType::ECS::Entity m_bgGameEntity = RType::ECS::NULL_ENTITY;

            // Background and obstacles entities
            std::vector<RType::ECS::Entity> m_backgroundEntities;
            std::vector<RType::ECS::Entity> m_obstacleEntities;
        };

    }
}
