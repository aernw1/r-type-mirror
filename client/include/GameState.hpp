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
#include "ECS/ShootingSystem.hpp"
#include "ECS/MovementSystem.hpp"
#include "ECS/InputSystem.hpp"
#include "ECS/CollisionSystem.hpp"
#include "ECS/HealthSystem.hpp"
#include "ECS/Component.hpp"
#include "Renderer/IRenderer.hpp"

#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <unordered_set>
#include <array>

namespace RType {
    namespace Client {

        // Player HUD data for multiplayer scoreboard
        struct PlayerHUDData {
            bool active = false;
            uint32_t score = 0;
            int lives = 3;
            int health = 100;
            int maxHealth = 100;
            bool isDead = false;
            RType::ECS::Entity scoreEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity playerEntity = RType::ECS::NULL_ENTITY;
        };

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

            void initializeUI();
            void updateHUD();
            void renderChargeBar();
            void renderHealthBars();

            // ECS systems
            void createSystems();

            // Server state update handler
            void OnServerStateUpdate(uint32_t tick, const std::vector<network::EntityState>& entities);

            struct EnemySpriteConfig {
                Renderer::SpriteId sprite;
                Math::Color tint;
            };
            struct EnemyBulletSpriteConfig {
                Renderer::SpriteId sprite;
                Math::Color tint;
                float scale;
            };
            EnemySpriteConfig GetEnemySpriteConfig(uint8_t enemyType) const;
            EnemyBulletSpriteConfig GetEnemyBulletSpriteConfig(uint8_t enemyType) const;
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
            std::unique_ptr<RType::ECS::MovementSystem> m_movementSystem;
            std::unique_ptr<RType::ECS::InputSystem> m_inputSystem;
            std::unique_ptr<RType::ECS::CollisionSystem> m_collisionSystem;
            std::unique_ptr<RType::ECS::HealthSystem> m_healthSystem;
            std::unique_ptr<RType::ECS::ShootingSystem> m_shootingSystem;

            // Textures pour background et obstacles
            Renderer::FontId m_fontId = Renderer::INVALID_FONT_ID;
            Renderer::TextureId m_bulletTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId m_bulletSprite = Renderer::INVALID_SPRITE_ID;

            // Enemy bullet textures and sprites
            Renderer::TextureId m_enemyBulletGreenTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_enemyBulletYellowTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_enemyBulletPurpleTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId m_enemyBulletGreenSprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_enemyBulletYellowSprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_enemyBulletPurpleSprite = Renderer::INVALID_SPRITE_ID;

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

            // Network synchronization
            float m_localScrollOffset = 0.0f;
            float m_serverScrollOffset = 0.0f;

            std::chrono::steady_clock::time_point m_lastInputTime;
            uint8_t m_currentInputs = 0;
            uint8_t m_previousInputs = 0;

            // Player ships tracking (network entities → ECS entities)
            std::unordered_map<uint32_t, RType::ECS::Entity> m_networkEntityMap;
            RType::ECS::Entity m_localPlayerEntity = RType::ECS::NULL_ENTITY; // Local player for prediction

            // Individual player ship sprites
            Renderer::TextureId m_playerGreenTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_playerBlueTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_playerRedTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId m_playerGreenSprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_playerBlueSprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_playerRedSprite = Renderer::INVALID_SPRITE_ID;

            // Enemy ship sprites
            Renderer::TextureId m_enemyGreenTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_enemyRedTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_enemyBlueTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId m_enemyGreenSprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_enemyRedSprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_enemyBlueSprite = Renderer::INVALID_SPRITE_ID;

            // HUD fonts
            Renderer::FontId m_hudFont = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_hudFontSmall = Renderer::INVALID_FONT_ID;

            // HUD entities - local player info (left side)
            RType::ECS::Entity m_hudPlayerEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_hudScoreEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_hudLivesEntity = RType::ECS::NULL_ENTITY;

            // HUD entities - all players scoreboard (right side)
            RType::ECS::Entity m_hudScoreboardTitle = RType::ECS::NULL_ENTITY;
            std::array<PlayerHUDData, MAX_PLAYERS> m_playersHUD;

            // Local player state for HUD
            uint32_t m_playerScore = 0;
            int m_playerLives = 3;
            float m_scoreAccumulator = 0.0f; // For time-based score testing

            bool m_isCharging = false;
            float m_chargeTime = 0.0f;
            static constexpr float MAX_CHARGE_TIME = 2.0f; // 2 seconds for full charge
        };

    }
}
