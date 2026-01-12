/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BlackOrbSystem implementation
*/

#include "ECS/BlackOrbSystem.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include <cmath>

namespace RType {
    namespace ECS {

        void BlackOrbSystem::Update(Registry& registry, float deltaTime) {
            auto orbs = registry.GetEntitiesWithComponent<BlackOrb>();

            for (auto orb : orbs) {
                if (!registry.IsEntityAlive(orb)) {
                    continue;
                }

                auto& blackOrb = registry.GetComponent<BlackOrb>(orb);
                if (!blackOrb.isActive) {
                    continue;
                }

                const auto& orbPos = registry.GetComponent<Position>(orb);

                auto bullets = registry.GetEntitiesWithComponent<Bullet>();
                for (auto bullet : bullets) {
                    if (!registry.IsEntityAlive(bullet)) {
                        continue;
                    }

                    if (registry.HasComponent<BossBullet>(bullet) || registry.HasComponent<BlackOrb>(bullet)) {
                        continue;
                    }

                    const auto& bulletPos = registry.GetComponent<Position>(bullet);
                    float dx = orbPos.x - bulletPos.x;
                    float dy = orbPos.y - bulletPos.y;
                    float distance = std::sqrt(dx * dx + dy * dy);
                    
                    // Destroy bullet if within absorption radius
                    if (distance < blackOrb.absorptionRadius) {
                        registry.DestroyEntity(bullet);
                        continue;
                    }

                    // Apply attraction force if within attraction radius
                    if (distance < blackOrb.attractionRadius) {
                        if (registry.HasComponent<Velocity>(bullet)) {
                            auto& bulletVel = registry.GetComponent<Velocity>(bullet);

                            float dirX = dx / distance;
                            float dirY = dy / distance;

                            float attractionStrength = blackOrb.attractionForce * (1.0f - distance / blackOrb.attractionRadius);
                            bulletVel.dx += dirX * attractionStrength * deltaTime;
                            bulletVel.dy += dirY * attractionStrength * deltaTime;
                        }
                    }
                }

                if (registry.HasComponent<ProximityDamage>(orb)) {
                    auto& proxDamage = registry.GetComponent<ProximityDamage>(orb);
                    proxDamage.timeSinceDamage += deltaTime;

                    if (proxDamage.timeSinceDamage >= proxDamage.tickRate) {
                        proxDamage.timeSinceDamage = 0.0f;

                        auto players = registry.GetEntitiesWithComponent<Player>();
                        for (auto player : players) {
                            if (!registry.IsEntityAlive(player)) {
                                continue;
                            }

                            const auto& playerPos = registry.GetComponent<Position>(player);
                            float dx = playerPos.x - orbPos.x;
                            float dy = playerPos.y - orbPos.y;
                            float distance = std::sqrt(dx * dx + dy * dy);

                            if (distance < proxDamage.damageRadius) {
                                if (registry.HasComponent<Shield>(player)) {
                                    continue;
                                }

                                // Apply proximity damage
                                if (registry.HasComponent<Health>(player)) {
                                    auto& health = registry.GetComponent<Health>(player);
                                    health.current -= static_cast<int>(proxDamage.damageAmount);

                                    if (health.current < 0) {
                                        health.current = 0;
                                    }

                                    Core::Logger::Debug("[BlackOrbSystem] Proximity damage {} to player",
                                                       proxDamage.damageAmount);
                                }
                            }
                        }
                    }
                }
            }
        }

    }
}
