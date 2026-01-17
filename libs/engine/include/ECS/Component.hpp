#pragma once

#include <string>
#include <cstring>
#include <cstdint>
#include <typeindex>
#include <type_traits>
#include <vector>
#include <array>
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"
#include "Entity.hpp"
#include "Audio/IAudio.hpp"
#include "Animation/AnimationTypes.hpp"

namespace RType {

    namespace ECS {
        using ComponentID = std::type_index;

        struct IComponent {
            virtual ~IComponent() = default;
        };

        struct Position : public IComponent {
            float x = 0.0f;
            float y = 0.0f;

            Position() = default;
            Position(float x, float y)
                : x(x), y(y) {}
        };

        struct Velocity : public IComponent {
            float dx = 0.0f;
            float dy = 0.0f;

            Velocity() = default;
            Velocity(float dx, float dy)
                : dx(dx), dy(dy) {}
        };

        struct Drawable : public IComponent {
            Renderer::SpriteId spriteId = Renderer::INVALID_SPRITE_ID;
            Math::Vector2 scale{1.0f, 1.0f};
            float rotation = 0.0f;
            Math::Vector2 origin{0.0f, 0.0f};
            Math::Color tint{1.0f, 1.0f, 1.0f, 1.0f};
            int layer = 0;

            Drawable() = default;
            Drawable(Renderer::SpriteId sprite, int renderLayer = 0)
                : spriteId(sprite), layer(renderLayer) {}
        };

        struct NetworkPlayer : public IComponent {
            uint8_t playerNumber = 0;
            uint64_t playerHash = 0;
            char name[32] = {};
            bool ready = false;

            NetworkPlayer() = default;
            NetworkPlayer(uint8_t num, uint64_t hash, const char* playerName, bool isReady = false)
                : playerNumber(num), playerHash(hash), ready(isReady) {
                if (playerName) {
                    std::strncpy(name, playerName, 31);
                    name[31] = '\0';
                }
            }
        };

        struct BoxCollider : public IComponent {
            float width = 0.0f;
            float height = 0.0f;

            BoxCollider() = default;
            BoxCollider(float width, float height)
                : width(width), height(height) {}
        };

        struct Controllable : public IComponent {
            float speed = 200.0f;

            Controllable() = default;
            Controllable(float moveSpeed)
                : speed(moveSpeed) {}
        };

        struct Player : public IComponent {
            uint8_t playerNumber = 0;
            uint64_t playerHash = 0;
            bool isLocalPlayer = false;
            uint8_t lives = 3;

            Player() = default;
            Player(uint8_t number, uint64_t hash, bool local = false, uint8_t startLives = 3)
                : playerNumber(number), playerHash(hash), isLocalPlayer(local), lives(startLives) {}
        };

        enum class EnemyType : uint8_t {
            BASIC = 0,
            FAST = 1,
            TANK = 2,
            BOSS = 3,
            FORMATION = 4
        };

        struct Enemy : public IComponent {
            EnemyType type = EnemyType::BASIC;
            uint32_t id = 0;

            Enemy() = default;
            Enemy(EnemyType enemyType, uint32_t enemyId = 0)
                : type(enemyType), id(enemyId) {}
        };

        struct Boss : public IComponent {
            uint8_t bossId = 1;

            Boss() = default;
            Boss(uint8_t id) : bossId(id) {}
        };

        struct BossKilled : public IComponent {
            Entity bossEntity;
            int levelNumber;
            float timeSinceDeath = 0.0f;

            BossKilled() = default;
            BossKilled(Entity boss, int level)
                : bossEntity(boss), levelNumber(level) {}
        };

        enum class BossAttackPattern {
            IDLE = 0,
            // Boss 1 
            FAN_SPRAY = 1,
            DIRECT_SHOT = 2,
            CIRCLE = 3,
            BLACK_ORB = 4,
            THIRD_BULLET = 5,
            // Boss 2
            SPIRAL_WAVE = 6,
            ANIMATED_ORB = 7,
            LASER_BEAM = 8
        };

        struct BossAttack : public IComponent {
            float attackCooldown = 3.0f;
            float timeSinceLastAttack = 0.0f;
            BossAttackPattern currentPattern = BossAttackPattern::FAN_SPRAY;

            BossAttack() = default;
            BossAttack(float cooldown) : attackCooldown(cooldown) {}
        };

        struct BossBullet : public IComponent {
            BossBullet() = default;
        };

        struct WaveAttack : public IComponent {
            WaveAttack() = default;
        };

        struct BlackOrb : public IComponent {
            float attractionRadius = 200.0f;
            float absorptionRadius = 30.0f;
            float attractionForce = 500.0f;
            bool isActive = true;

            BlackOrb() = default;
            BlackOrb(float attraction, float absorption, float force)
                : attractionRadius(attraction), absorptionRadius(absorption), attractionForce(force) {}
        };

        struct ProximityDamage : public IComponent {
            float damageRadius = 120.0f;
            float damageAmount = 1.0f;
            float tickRate = 0.5f;
            float timeSinceDamage = 0.0f;

            ProximityDamage() = default;
            ProximityDamage(float radius, float damage, float rate)
                : damageRadius(radius), damageAmount(damage), tickRate(rate) {}
        };

        struct DamageFlash : public IComponent {
            float duration = 0.1f;
            float timeRemaining = 0.0f;
            bool isActive = false;

            DamageFlash() = default;
            DamageFlash(float flashDuration) : duration(flashDuration) {}

            void Trigger() {
                isActive = true;
                timeRemaining = duration;
            }
        };

        struct ThirdBullet : public IComponent {
            float spawnInterval = 0.3f;
            float timeSinceSpawn = 0.0f;
            int damage = 50;
            bool isActive = true;

            ThirdBullet() = default;
            ThirdBullet(float interval, int dmg)
                : spawnInterval(interval), damage(dmg) {}
        };

        struct Health : public IComponent {
            int current = 100;
            int max = 100;

            Health() = default;
            Health(int maxHealth)
                : current(maxHealth), max(maxHealth) {}
            Health(int currentHealth, int maxHealth)
                : current(currentHealth), max(maxHealth) {}
        };

        struct ScoreValue : public IComponent {
            uint32_t points = 100;

            ScoreValue() = default;
            ScoreValue(uint32_t scorePoints)
                : points(scorePoints) {}
        };

        struct ScoreTimer : public IComponent {
            float elapsed = 0.0f;

            ScoreTimer() = default;
            ScoreTimer(float startElapsed)
                : elapsed(startElapsed) {}
        };

        struct Damage : public IComponent {
            int amount = 10;

            Damage() = default;
            Damage(int damageAmount)
                : amount(damageAmount) {}
        };

        struct EnemyKilled : public IComponent {
            uint32_t enemyId = 0;
            Entity killedBy = NULL_ENTITY;

            EnemyKilled() = default;
            EnemyKilled(uint32_t id, Entity killer = NULL_ENTITY)
                : enemyId(id), killedBy(killer) {}
        };

        struct Bullet : public IComponent {
            Entity owner = NULL_ENTITY;

            Bullet() = default;
            Bullet(Entity shooter)
                : owner(shooter) {}
        };

        struct Shooter : public IComponent {
            float fireRate = 0.2f;
            float cooldown = 0.0f;
            float offsetX = 50.0f;
            float offsetY = 20.0f;

            Shooter() = default;
            Shooter(float rate, float oX = 50.0f, float oY = 20.0f) : fireRate(rate), offsetX(oX), offsetY(oY) {}
        };

        struct ShootCommand : public IComponent {
            bool wantsToShoot = false;

            ShootCommand() = default;
            ShootCommand(bool shoot) : wantsToShoot(shoot) {}
        };

        struct Scrollable : public IComponent {
            float speed = -100.0f;

            Scrollable() = default;
            Scrollable(float scrollSpeed) : speed(scrollSpeed) {}
        };

        struct Obstacle : public IComponent {
            bool blocking = true;

            Obstacle() = default;
            Obstacle(bool isBlocking) : blocking(isBlocking) {}
        };

        struct ObstacleVisual : public IComponent {
        };

        // Stable network identifier for server->client entity mirroring.
        // IMPORTANT: This must live on the entity as a component, because raw ECS entity IDs are recycled.
        // If we instead map "ECS entity id -> network id" in a hash map, then destroying and reusing an
        // ECS id in the same tick can cause a new entity to inherit the old network id (type confusion).
        struct NetworkId : public IComponent {
            uint32_t id = 0;

            NetworkId() = default;
            explicit NetworkId(uint32_t networkId) : id(networkId) {}
        };

        struct ObstacleMetadata : public IComponent {
            uint32_t uniqueId = 0;
            Entity visualEntity = NULL_ENTITY;
            float offsetX = 0.0f;
            float offsetY = 0.0f;

            ObstacleMetadata() = default;
            ObstacleMetadata(uint32_t id,
                             Entity visual = NULL_ENTITY,
                             float offsetX = 0.0f,
                             float offsetY = 0.0f)
                : uniqueId(id),
                  visualEntity(visual),
                  offsetX(offsetX),
                  offsetY(offsetY) {}
        };

        struct Invincibility : public IComponent {
            float remainingTime = 0.0f;

            Invincibility() = default;
            Invincibility(float duration) : remainingTime(duration) {}
        };

        struct CollisionLayer : public IComponent {
            uint16_t layer = 0;     // What layer this entity is on
            uint16_t mask = 0xFFFF; // Which layers this entity collides with

            CollisionLayer() = default;
            CollisionLayer(uint16_t l, uint16_t m) : layer(l), mask(m) {}
        };

        // masks for collision layers
        namespace CollisionLayers {
            constexpr uint16_t NONE = 0;
            constexpr uint16_t PLAYER = 1 << 0;        // 0x0001
            constexpr uint16_t ENEMY = 1 << 1;         // 0x0002
            constexpr uint16_t PLAYER_BULLET = 1 << 2; // 0x0004
            constexpr uint16_t ENEMY_BULLET = 1 << 3;  // 0x0008
            constexpr uint16_t OBSTACLE = 1 << 4;      // 0x0010
            constexpr uint16_t POWERUP = 1 << 5;       // 0x0020
            constexpr uint16_t ALL = 0xFFFF;
        }

        struct CircleCollider : public IComponent {
            float radius = 0.0f;

            CircleCollider() = default;
            CircleCollider(float r) : radius(r) {}
        };

        struct CollisionEvent : public IComponent {
            Entity other = NULL_ENTITY;

            CollisionEvent() = default;
            CollisionEvent(Entity e) : other(e) {}
        };

        // Powerup system components
        enum class PowerUpType : uint8_t {
            FIRE_RATE_BOOST = 0,
            SPREAD_SHOT = 1,
            LASER_BEAM = 2,
            FORCE_POD = 3,
            SPEED_BOOST = 4,
            SHIELD = 5
        };

        struct PowerUp : public IComponent {
            PowerUpType type = PowerUpType::FIRE_RATE_BOOST;
            uint32_t id = 0;

            PowerUp() = default;
            PowerUp(PowerUpType powerupType, uint32_t powerupId = 0)
                : type(powerupType), id(powerupId) {}
        };

        struct ActivePowerUps : public IComponent {
            bool hasFireRateBoost = false;
            bool hasSpreadShot = false;
            bool hasLaserBeam = false;
            bool hasShield = false;
            float speedMultiplier = 1.0f;

            ActivePowerUps() = default;
        };

        enum class WeaponType : uint8_t {
            STANDARD = 0,
            SPREAD = 1,
            LASER = 2
        };

        struct WeaponSlot : public IComponent {
            WeaponType type = WeaponType::STANDARD;
            float fireRate = 0.2f;
            float cooldown = 0.0f;
            int damage = 25;
            bool enabled = true;

            WeaponSlot() = default;
            WeaponSlot(WeaponType weaponType, float rate, int dmg)
                : type(weaponType), fireRate(rate), damage(dmg) {}
        };

        struct ForcePod : public IComponent {
            Entity owner = NULL_ENTITY;
            float offsetX = -60.0f;
            float offsetY = 0.0f;
            bool isAttached = true;

            ForcePod() = default;
            ForcePod(Entity ownerEntity, float oX = -60.0f, float oY = 0.0f)
                : owner(ownerEntity), offsetX(oX), offsetY(oY) {}
        };

        struct Shield : public IComponent {
            float duration = 0.0f; // 0 = permanent (until death)
            float timeRemaining = 0.0f;

            Shield() = default;
            Shield(float dur = 0.0f) : duration(dur), timeRemaining(dur) {}
        };

        struct SoundEffect : public IComponent {
            Audio::SoundId soundId = Audio::INVALID_SOUND_ID;
            float volume = 1.0f;
            float pitch = 1.0f;
            float pan = 0.0f;
            bool loop = false;
            bool positional = false;

            SoundEffect() = default;
            SoundEffect(Audio::SoundId id, float vol = 1.0f)
                : soundId(id), volume(vol) {}
        };

        struct MusicEffect : public IComponent {
            Audio::MusicId musicId = Audio::INVALID_MUSIC_ID;
            bool play = true;
            bool stop = false;
            float volume = 1.0f;
            float pitch = 1.0f;
            bool loop = true;

            MusicEffect() = default;
            explicit MusicEffect(Audio::MusicId id)
                : musicId(id) {}
        };

        struct SpriteAnimation : public IComponent {
            Animation::AnimationClipId clipId = Animation::INVALID_CLIP_ID;
            float currentTime = 0.0f;
            float playbackSpeed = 1.0f;
            bool playing = true;
            bool looping = false;
            bool destroyOnComplete = false;

            std::size_t currentFrameIndex = 0;
            Math::Rectangle currentRegion{};

            SpriteAnimation() = default;
            SpriteAnimation(Animation::AnimationClipId clip, bool loop = false, float speed = 1.0f)
                : clipId(clip), playbackSpeed(speed), looping(loop) {}
        };

        struct AnimationStateMachine : public IComponent {
            Animation::AnimationGraphId graphId = Animation::INVALID_GRAPH_ID;
            Animation::AnimationStateId currentState = Animation::INVALID_STATE_ID;
            Animation::AnimationStateId previousState = Animation::INVALID_STATE_ID;
            float stateTime = 0.0f;
            float blendFactor = 0.0f;
            float blendDuration = 0.0f;
            bool isTransitioning = false;

            static constexpr std::size_t MAX_PARAMS = 8;
            std::array<float, MAX_PARAMS> parameters{};
            std::array<std::array<char, 32>, MAX_PARAMS> parameterNames{};
            std::size_t parameterCount = 0;

            AnimationStateMachine() = default;
            explicit AnimationStateMachine(Animation::AnimationGraphId graph)
                : graphId(graph) {}

            void SetParameter(const char* name, float value) {
                for (std::size_t i = 0; i < parameterCount; ++i) {
                    if (std::strncmp(parameterNames[i].data(), name, 31) == 0) {
                        parameters[i] = value;
                        return;
                    }
                }
                if (parameterCount < MAX_PARAMS) {
                    std::strncpy(parameterNames[parameterCount].data(), name, 31);
                    parameterNames[parameterCount][31] = '\0';
                    parameters[parameterCount] = value;
                    parameterCount++;
                }
            }

            float GetParameter(const char* name) const {
                for (std::size_t i = 0; i < parameterCount; ++i) {
                    if (std::strncmp(parameterNames[i].data(), name, 31) == 0) {
                        return parameters[i];
                    }
                }
                return 0.0f;
            }
        };

        struct AnimatedSprite : public IComponent {
            bool needsUpdate = true;

            AnimatedSprite() = default;
        };

        struct VisualEffect : public IComponent {
            Animation::EffectType type = Animation::EffectType::EXPLOSION_SMALL;
            float lifetime = 0.0f;
            float maxLifetime = 1.0f;
            Entity owner = NULL_ENTITY;
            float offsetX = 0.0f;
            float offsetY = 0.0f;

            VisualEffect() = default;
            VisualEffect(Animation::EffectType t, float duration)
                : type(t), maxLifetime(duration) {}
            VisualEffect(Animation::EffectType t, float duration, Entity ownerEntity, float offX, float offY)
                : type(t), maxLifetime(duration), owner(ownerEntity), offsetX(offX), offsetY(offY) {}
        };

        struct FloatingText : public IComponent {
            char text[32] = {};
            float lifetime = 0.0f;
            float maxLifetime = 1.5f;
            float velocityY = -50.0f;
            float fadeStartTime = 0.5f;
            Math::Color color{1.0f, 1.0f, 1.0f, 1.0f};

            FloatingText() = default;
            FloatingText(const char* txt, float duration, const Math::Color& col)
                : maxLifetime(duration), color(col) {
                if (txt) {
                    std::strncpy(text, txt, 31);
                    text[31] = '\0';
                }
            }
        };

        struct AnimationEvents : public IComponent {
            static constexpr std::size_t MAX_EVENTS = 4;
            std::array<std::array<char, 32>, MAX_EVENTS> eventNames{};
            std::size_t eventCount = 0;

            AnimationEvents() = default;

            void PushEvent(const char* name) {
                if (eventCount < MAX_EVENTS && name) {
                    std::strncpy(eventNames[eventCount].data(), name, 31);
                    eventNames[eventCount][31] = '\0';
                    eventCount++;
                }
            }

            void Clear() { eventCount = 0; }

            bool HasEvent(const char* name) const {
                for (std::size_t i = 0; i < eventCount; ++i) {
                    if (std::strncmp(eventNames[i].data(), name, 31) == 0) {
                        return true;
                    }
                }
                return false;
            }
        };

        struct AnimationLayer : public IComponent {
            Animation::AnimationClipId clipId = Animation::INVALID_CLIP_ID;
            float currentTime = 0.0f;
            float weight = 1.0f;
            float playbackSpeed = 1.0f;
            bool additive = false;
            int layerIndex = 0;

            AnimationLayer() = default;
            AnimationLayer(Animation::AnimationClipId clip, int layer, float w = 1.0f)
                : clipId(clip), weight(w), layerIndex(layer) {}
        };

        struct PowerUpGlow : public IComponent {
            float time = 0.0f;
            float pulseSpeed = 2.0f;
            float minAlpha = 0.7f;
            float maxAlpha = 1.0f;
            float baseScale = 2.5f;
            float scalePulse = 0.08f;

            PowerUpGlow() = default;
        };
    }

}
