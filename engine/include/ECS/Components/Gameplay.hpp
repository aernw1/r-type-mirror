#pragma once
/**
 * @file Gameplay.hpp
 * @brief Generic gameplay components for any game engine.
 * 
 * These include health, damage, scoring, and combat-related components.
 */

#include <cstdint>
#include <cstring>
#include "ECS/Entity.hpp"
#include "ECS/Components/IComponent.hpp"

namespace RType {
namespace ECS {

    /**
     * @brief Health component for damageable entities.
     */
    struct Health : public IComponent {
        int current = 100;
        int max = 100;

        Health() = default;
        Health(int maxHealth)
            : current(maxHealth), max(maxHealth) {}
        Health(int currentHealth, int maxHealth)
            : current(currentHealth), max(maxHealth) {}

        bool IsDead() const { return current <= 0; }
        float GetPercentage() const { return max > 0 ? static_cast<float>(current) / max : 0.0f; }
    };

    /**
     * @brief Damage component for entities that deal damage.
     */
    struct Damage : public IComponent {
        int amount = 10;

        Damage() = default;
        Damage(int damageAmount)
            : amount(damageAmount) {}
    };

    /**
     * @brief Score value for collectibles/enemies.
     */
    struct ScoreValue : public IComponent {
        uint32_t points = 100;

        ScoreValue() = default;
        ScoreValue(uint32_t scorePoints)
            : points(scorePoints) {}
    };

    /**
     * @brief Score timer for time-based scoring.
     */
    struct ScoreTimer : public IComponent {
        float elapsed = 0.0f;

        ScoreTimer() = default;
        ScoreTimer(float startElapsed)
            : elapsed(startElapsed) {}
    };

    /**
     * @brief Bullet/projectile component.
     */
    struct Bullet : public IComponent {
        Entity owner = NULL_ENTITY;

        Bullet() = default;
        Bullet(Entity shooter)
            : owner(shooter) {}
    };

    /**
     * @brief Component for entities that can shoot.
     */
    struct Shooter : public IComponent {
        float fireRate = 0.2f;
        float cooldown = 0.0f;
        float offsetX = 50.0f;
        float offsetY = 20.0f;

        Shooter() = default;
        Shooter(float rate, float oX = 50.0f, float oY = 20.0f)
            : fireRate(rate), offsetX(oX), offsetY(oY) {}
    };

    /**
     * @brief Shoot command input state.
     */
    struct ShootCommand : public IComponent {
        bool wantsToShoot = false;

        ShootCommand() = default;
        ShootCommand(bool shoot) : wantsToShoot(shoot) {}
    };

    /**
     * @brief Proximity damage for AOE effects.
     */
    struct ProximityDamage : public IComponent {
        float damageRadius = 120.0f;
        float damageAmount = 1.0f;
        float tickRate = 0.5f;
        float timeSinceDamage = 0.0f;

        ProximityDamage() = default;
        ProximityDamage(float radius, float damage, float rate)
            : damageRadius(radius), damageAmount(damage), tickRate(rate) {}
    };

    /**
     * @brief Lifetime component for auto-destruction.
     */
    struct Lifetime : public IComponent {
        float remaining = 1.0f;

        Lifetime() = default;
        Lifetime(float duration) : remaining(duration) {}
    };

    /**
     * @brief Tag component for entity categorization.
     */
    struct Tag : public IComponent {
        char name[32] = {};

        Tag() = default;
        Tag(const char* tagName) {
            if (tagName) {
                std::strncpy(name, tagName, 31);
                name[31] = '\0';
            }
        }
    };

} // namespace ECS
} // namespace RType
