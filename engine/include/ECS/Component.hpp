#pragma once
/**
 * @file Component.hpp
 * @brief Master include file for all ECS components.
 * 
 * This file includes all generic engine components from modular files,
 * and contains R-Type specific components inline for backwards compatibility.
 * 
 * For new games, use only the generic component headers from ECS/Components/.
 * R-Type specific components will be moved to games/rtype/components/ in a future phase.
 */

// =============================================================================
// Generic Engine Components (game-agnostic)
// =============================================================================
#include "ECS/Components/IComponent.hpp"     // Base component interface
#include "ECS/Components/Transform.hpp"       // Position, Velocity, Transform2D, Controllable, Scrollable, NetworkId
#include "ECS/Components/Rendering.hpp"       // Drawable, DamageFlash, FloatingText
#include "ECS/Components/Physics.hpp"         // BoxCollider, CircleCollider, CollisionLayer, Obstacle, Invincibility, Rigidbody2D, PhysicsMaterial
#include "ECS/Components/Audio.hpp"           // SoundEffect, MusicEffect
#include "ECS/Components/Animation.hpp"       // SpriteAnimation, AnimationStateMachine, VisualEffect, AnimationEvents, AnimationLayer
#include "ECS/Components/Gameplay.hpp"        // Health, Damage, ScoreValue, ScoreTimer, Bullet, Shooter, ShootCommand, ProximityDamage, Lifetime, Tag

// Standard library includes for R-Type components
#include <string>
#include <cstring>
#include <cstdint>
#include <vector>
#include <array>
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"
#include "Entity.hpp"
#include "Animation/AnimationTypes.hpp"

namespace RType {
namespace ECS {

// =============================================================================
// R-Type Specific Components (will be moved to games/rtype/components/ later)
// =============================================================================

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

    struct SecondAttack : public IComponent {
        SecondAttack() = default;
    };

    struct FireBullet : public IComponent {
        FireBullet() = default;
    };

    struct Mine : public IComponent {
        float proximityRadius = 80.0f;
        float explosionRadius = 100.0f;
        float lifeTime = 10.0f;
        float timer = 0.0f;
        bool isExploding = false;
        float explosionTimer = 0.0f;

        Mine() = default;
        Mine(float proximity, float explosion, float life)
            : proximityRadius(proximity), explosionRadius(explosion), lifeTime(life) {}
    };

    struct BossMovementPattern : public IComponent {
        float timer = 0.0f;
        float amplitudeY = 200.0f;
        float amplitudeX = 80.0f;
        float frequencyY = 0.5f;
        float frequencyX = 0.3f;
        float centerY = 0.0f;
        float centerX = 0.0f;

        BossMovementPattern() = default;
        BossMovementPattern(float ampY, float ampX, float freqY, float freqX, float centerYPos, float centerXPos)
            : amplitudeY(ampY), amplitudeX(ampX), frequencyY(freqY), frequencyX(freqX), centerY(centerYPos), centerX(centerXPos) {}
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

    struct ThirdBullet : public IComponent {
        float spawnInterval = 0.3f;
        float timeSinceSpawn = 0.0f;
        int damage = 50;
        bool isActive = true;

        ThirdBullet() = default;
        ThirdBullet(float interval, int dmg)
            : spawnInterval(interval), damage(dmg) {}
    };

    struct EnemyKilled : public IComponent {
        uint32_t enemyId = 0;
        Entity killedBy = NULL_ENTITY;

        EnemyKilled() = default;
        EnemyKilled(uint32_t id, Entity killer = NULL_ENTITY)
            : enemyId(id), killedBy(killer) {}
    };

    struct ObstacleVisual : public IComponent {
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

    struct PowerUpGlow : public IComponent {
        float time = 0.0f;
        float pulseSpeed = 2.0f;
        float minAlpha = 0.7f;
        float maxAlpha = 1.0f;
        float baseScale = 2.5f;
        float scalePulse = 0.08f;

        PowerUpGlow() = default;
    };

} // namespace ECS
} // namespace RType
