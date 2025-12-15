#pragma once

#include <string>
#include <cstring>
#include <cstdint>
#include <typeindex>
#include <type_traits>
#include <vector>
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"
#include "Entity.hpp"

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

            Player() = default;
            Player(uint8_t number, uint64_t hash, bool local = false)
                : playerNumber(number), playerHash(hash), isLocalPlayer(local) {}
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
    }

}
