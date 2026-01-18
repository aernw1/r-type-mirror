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
#include "ECS/CollisionDetectionSystem.hpp"
#include "ECS/BulletCollisionResponseSystem.hpp"
#include "ECS/PlayerCollisionResponseSystem.hpp"
#include "ECS/ObstacleCollisionResponseSystem.hpp"
#include "ECS/HealthSystem.hpp"
#include "ECS/ScoreSystem.hpp"
#include "ECS/ShieldSystem.hpp"
#include "ECS/ForcePodSystem.hpp"
#include "ECS/PowerUpSpawnSystem.hpp"
#include "ECS/PowerUpCollisionSystem.hpp"
#include "ECS/AudioSystem.hpp"
#include "ECS/AnimationSystem.hpp"
#include "ECS/EffectFactory.hpp"
#include "ECS/Component.hpp"
#include "ECS/PowerUpFactory.hpp"
#include "ECS/LevelLoader.hpp"
#include "Animation/AnimationModule.hpp"
#include "Renderer/IRenderer.hpp"

#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <unordered_set>
#include <unordered_map>
#include <array>
#include <deque>

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
            RType::ECS::Entity powerupSpreadEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity powerupLaserEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity powerupSpeedEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity powerupShieldEntity = RType::ECS::NULL_ENTITY;
        };

        struct PredictedInput {
            uint32_t sequence = 0;
            uint8_t inputs = 0;
            float predictedX = 0.0f;
            float predictedY = 0.0f;
            float deltaTime = 0.0f;
        };

        // Entity interpolation state for smooth rendering of remote entities
        struct InterpolationState {
            float prevX = 0.0f;
            float prevY = 0.0f;
            float targetX = 0.0f;
            float targetY = 0.0f;
            float interpTime = 0.0f;
            float interpDuration = 1.0f / 60.0f;
        };

        class InGameState : public IState {
        public:
            InGameState(GameStateMachine& machine, GameContext& context, uint32_t seed, const std::string& levelPath = "assets/levels/level1.json");
            ~InGameState() override = default;

            void Init() override;
            void Cleanup() override;
            void HandleInput() override;
            void Update(float dt) override;
            void Draw() override;
        private:
            // Level loading
            void loadLevel(const std::string& levelPath);
            void initializeFromLevel();

            // Player System (Keane)
            void initializePlayers();

            // Ennemy System (Dryss)
            void spawnEnemies();

            void initializeUI();
            void updateHUD();
            void updatePowerUpIcons();
            void updatePowerUpText(RType::ECS::Entity& textEntity, const std::string& text,
                                   bool isActive, float x, float y);
            void renderChargeBar();
            void renderHealthBars();
            void renderBossHealthBar();
            void updateBossHealthBar();
            void initializeBossHealthBar();
            void destroyBossHealthBar();
            void renderGameOverOverlay();
            void renderVictoryOverlay();
            void renderLevelTransition();
            void triggerGameOverIfNeeded();
            void triggerVictoryIfNeeded();
            void enterResultsScreen();
            void createBeamEntity();
            void updateBeam(float dt);

            // Level progression
            void checkBossDefeated();

            // ECS systems
            void createSystems();

            // Server state update handler
            void OnServerStateUpdate(uint32_t tick, const std::vector<network::EntityState>& entities, const std::vector<network::InputAck>& inputAcks);
            void ReconcileWithServer(const network::InputAck& ack);
            void OnLevelComplete(uint8_t completedLevel, uint8_t nextLevel);
            void ApplyPowerUpStateToPlayer(ECS::Entity playerEntity, const network::EntityState& entityState);

            // Component cleanup helper for entity type validation
            void CleanupInvalidComponents(ECS::Entity entity, network::EntityType expectedType);

            void cleanupForLevelTransition();

            struct EnemySpriteConfig {
                Renderer::SpriteId sprite = Renderer::INVALID_SPRITE_ID;
                Math::Color tint{1.0f, 1.0f, 1.0f, 1.0f};
                float rotation = 0.0f;
            };

            struct EnemyBulletSpriteConfig {
                Renderer::SpriteId sprite;
                Math::Color tint;
                float scale;
            };
            EnemySpriteConfig GetEnemySpriteConfig(uint8_t enemyType) const;
            EnemyBulletSpriteConfig GetEnemyBulletSpriteConfig(uint8_t enemyType) const;
            Renderer::SpriteId GetPowerUpSprite(ECS::PowerUpType type) const;

            std::pair<std::string, uint8_t> FindPlayerNameAndNumber(uint64_t ownerHash, const std::unordered_set<uint8_t>& assignedNumbers) const;
            void CreatePlayerNameLabel(RType::ECS::Entity playerEntity, const std::string& playerName, float x, float y);
            void UpdatePlayerNameLabelPosition(RType::ECS::Entity playerEntity, float x, float y);
            void DestroyPlayerNameLabel(RType::ECS::Entity playerEntity);

            // Level transition
            void UpdateLevelTransition(float dt);
            void LoadNextLevel();
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
            std::unique_ptr<RType::ECS::CollisionDetectionSystem> m_collisionDetectionSystem;
            std::unique_ptr<RType::ECS::BulletCollisionResponseSystem> m_bulletResponseSystem;
            std::unique_ptr<RType::ECS::PlayerCollisionResponseSystem> m_playerResponseSystem;
            std::unique_ptr<RType::ECS::ObstacleCollisionResponseSystem> m_obstacleResponseSystem;
            std::unique_ptr<RType::ECS::HealthSystem> m_healthSystem;
            std::unique_ptr<RType::ECS::ScoreSystem> m_scoreSystem;
            std::unique_ptr<RType::ECS::ShootingSystem> m_shootingSystem;
            std::unique_ptr<RType::ECS::AudioSystem> m_audioSystem;
            std::unique_ptr<RType::ECS::ShieldSystem> m_shieldSystem;
            std::unique_ptr<RType::ECS::ForcePodSystem> m_forcePodSystem;
            std::unique_ptr<RType::ECS::PowerUpSpawnSystem> m_powerUpSpawnSystem;
            std::unique_ptr<RType::ECS::PowerUpCollisionSystem> m_powerUpCollisionSystem;
            std::unique_ptr<RType::ECS::AnimationSystem> m_animationSystem;
            std::unique_ptr<Animation::AnimationModule> m_animationModule;
            std::unique_ptr<RType::ECS::EffectFactory> m_effectFactory;

            // Bullet textures and sprites
            Renderer::TextureId m_bulletTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId m_bulletSprite = Renderer::INVALID_SPRITE_ID;

            Audio::SoundId m_playerShootSound = Audio::INVALID_SOUND_ID;
            Audio::MusicId m_shootMusic = Audio::INVALID_MUSIC_ID;
            Audio::SoundId m_powerUpSound = Audio::INVALID_SOUND_ID;
            Audio::MusicId m_powerUpMusic = Audio::INVALID_MUSIC_ID;
            float m_shootSfxCooldown = 0.0f;
            Audio::MusicId m_gameMusic = Audio::INVALID_MUSIC_ID;
            bool m_gameMusicPlaying = false;
            Audio::MusicId m_bossMusic = Audio::INVALID_MUSIC_ID;
            bool m_bossMusicPlaying = false;
            Audio::MusicId m_gameOverMusic = Audio::INVALID_MUSIC_ID;
            bool m_gameOverMusicPlaying = false;
            Audio::MusicId m_victoryMusic = Audio::INVALID_MUSIC_ID;
            bool m_victoryMusicPlaying = false;

            // Enemy bullet textures and sprites
            Renderer::TextureId m_enemyBulletGreenTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_enemyBulletYellowTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::TextureId m_enemyBulletPurpleTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId m_enemyBulletGreenSprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_enemyBulletYellowSprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_enemyBulletPurpleSprite = Renderer::INVALID_SPRITE_ID;

            // Background and obstacles entities
            std::vector<RType::ECS::Entity> m_backgroundEntities;
            std::vector<RType::ECS::Entity> m_obstacleSpriteEntities;
            std::vector<RType::ECS::Entity> m_obstacleColliderEntities;

            bool m_escapeKeyPressed = false;

            // Network synchronization
            float m_localScrollOffset = 0.0f;
            float m_serverScrollOffset = 0.0f;

            std::chrono::steady_clock::time_point m_lastInputTime;
            uint8_t m_currentInputs = 0;
            uint8_t m_previousInputs = 0;

            std::deque<PredictedInput> m_inputHistory;
            uint32_t m_inputSequence = 0;
            uint32_t m_lastAckedSequence = 0;
            float m_predictedX = 0.0f;
            float m_predictedY = 0.0f;
            static constexpr size_t MAX_INPUT_HISTORY = 120;
            static constexpr float PREDICTION_SPEED = 200.0f;

            // Entity interpolation for remote entities
            std::unordered_map<uint32_t, InterpolationState> m_interpolationStates;

            // Player ships tracking (network entities → ECS entities)
            std::unordered_map<uint32_t, RType::ECS::Entity> m_networkEntityMap;
            std::unordered_map<uint32_t, uint8_t> m_bulletFlagsMap; // Track bullet flags to detect type changes
            RType::ECS::Entity m_localPlayerEntity = RType::ECS::NULL_ENTITY; // Local player entity mirrored from server

            // Level progression tracking with visual transition
            enum class TransitionPhase {
                NONE,
                FADE_OUT,
                LOADING,
                FADE_IN
            };

            struct LevelProgressionState {
                bool bossDefeated = false;
                bool levelComplete = false;
                bool allLevelsComplete = false;
                float transitionTimer = 0.0f;
                float victoryElapsed = 0.0f;
                int currentLevelNumber = 1;
                int nextLevelNumber = 2;
                int totalLevels = 3;
                TransitionPhase transitionPhase = TransitionPhase::NONE;
                float fadeAlpha = 0.0f;
            };
            LevelProgressionState m_levelProgress;

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

            // Power-up sprites
            Renderer::SpriteId m_powerupSpreadSprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_powerupLaserSprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_powerupForcePodSprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_powerupSpeedSprite = Renderer::INVALID_SPRITE_ID;
            Renderer::SpriteId m_powerupShieldSprite = Renderer::INVALID_SPRITE_ID;

            // Explosion animation
            Renderer::TextureId m_explosionTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId m_explosionSprite = Renderer::INVALID_SPRITE_ID;
            Animation::AnimationClipId m_explosionClipId = Animation::INVALID_CLIP_ID;
            Renderer::TextureId m_shootingTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId m_shootingSprite = Renderer::INVALID_SPRITE_ID;
            Animation::AnimationClipId m_shootingClipId = Animation::INVALID_CLIP_ID;

            Renderer::TextureId m_forcePodTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId m_forcePodSprite = Renderer::INVALID_SPRITE_ID;
            Animation::AnimationClipId m_forcePodClipId = Animation::INVALID_CLIP_ID;
            Renderer::TextureId m_beamTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId m_beamSprite = Renderer::INVALID_SPRITE_ID;
            Animation::AnimationClipId m_beamClipId = Animation::INVALID_CLIP_ID;
            Renderer::TextureId m_hitTexture = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId m_hitSprite = Renderer::INVALID_SPRITE_ID;
            Animation::AnimationClipId m_hitClipId = Animation::INVALID_CLIP_ID;
            Animation::AnimationClipId m_waveAttackClipId = Animation::INVALID_CLIP_ID;
            Animation::AnimationClipId m_secondAttackClipId = Animation::INVALID_CLIP_ID;
            Animation::AnimationClipId m_fireBulletClipId = Animation::INVALID_CLIP_ID;

            // HUD fonts
            Renderer::FontId m_hudFont = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_hudFontSmall = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_gameOverFontLarge = Renderer::INVALID_FONT_ID;
            Renderer::FontId m_gameOverFontMedium = Renderer::INVALID_FONT_ID;

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

            // Game Over overlay
            bool m_isGameOver = false;
            float m_gameOverElapsed = 0.0f;
            bool m_gameOverEnterPressed = false;
            bool m_gameOverEscapePressed = false;
            RType::ECS::Entity m_gameOverTitleEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_gameOverScoreEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_gameOverHintEntity = RType::ECS::NULL_ENTITY;

            RType::ECS::Entity m_victoryTitleEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_victoryScoreEntity = RType::ECS::NULL_ENTITY;
            RType::ECS::Entity m_victoryHintEntity = RType::ECS::NULL_ENTITY;

            bool m_isCharging = false;
            float m_chargeTime = 0.0f;
            RType::ECS::Entity m_beamEntity = RType::ECS::NULL_ENTITY;
            float m_beamDuration = 0.0f;
            static constexpr float MAX_CHARGE_TIME = 2.0f; // 2 seconds for full charge

            bool m_isNetworkSession = false;
            std::unordered_map<uint64_t, RType::ECS::Entity> m_obstacleIdToCollider;

            // Player name tracking
            std::unordered_map<uint64_t, std::string> m_playerNameMap;
            std::unordered_map<RType::ECS::Entity, RType::ECS::Entity> m_playerNameLabels;
            std::unordered_set<uint8_t> m_assignedPlayerNumbers;

            // Level loader data
            RType::ECS::LevelData m_levelData;
            RType::ECS::LoadedAssets m_levelAssets;
            RType::ECS::CreatedEntities m_levelEntities;
            std::string m_currentLevelPath = "assets/levels/level1.json";

            bool m_bossWarningActive = false;
            bool m_bossWarningTriggered = false;
            float m_bossWarningTimer = 0.0f;
            static constexpr float BOSS_WARNING_DURATION = 4.0f;
            bool m_bossWarningFlashState = false;
            void renderBossWarning();

            // Boss health bar
            struct {
                bool active = false;
                int currentHealth = 0;
                int maxHealth = 1000;
                uint32_t bossNetworkId = 0;
                RType::ECS::Entity titleEntity = RType::ECS::NULL_ENTITY;
                RType::ECS::Entity barBackgroundEntity = RType::ECS::NULL_ENTITY;
                RType::ECS::Entity barForegroundEntity = RType::ECS::NULL_ENTITY;
            } m_bossHealthBar;
        };

    }
}
