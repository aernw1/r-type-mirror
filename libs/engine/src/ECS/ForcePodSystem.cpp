/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ForcePodSystem
*/

#include "ECS/ForcePodSystem.hpp"
#include "ECS/Component.hpp"

namespace RType {
    namespace ECS {

        void ForcePodSystem::Update(Registry& registry, float deltaTime) {
            (void)deltaTime;

            auto forcePods = registry.GetEntitiesWithComponent<ForcePod>();

            for (Entity pod : forcePods) {
                if (!registry.IsEntityAlive(pod)) {
                    continue;
                }

                auto& forcePodComp = registry.GetComponent<ForcePod>(pod);

                // Check if owner still exists
                if (!registry.IsEntityAlive(forcePodComp.owner)) {
                    // Owner died, destroy force pod
                    registry.DestroyEntity(pod);
                    continue;
                }

                // Update force pod position to follow owner
                if (registry.HasComponent<Position>(forcePodComp.owner) &&
                    registry.HasComponent<Position>(pod)) {

                    const auto& ownerPos = registry.GetComponent<Position>(forcePodComp.owner);
                    auto& podPos = registry.GetComponent<Position>(pod);

                    // Smooth follow (lerp for smooth movement)
                    float targetX = ownerPos.x + forcePodComp.offsetX;
                    float targetY = ownerPos.y + forcePodComp.offsetY;

                    float lerpFactor = 0.15f; // Adjust for smoothness
                    podPos.x += (targetX - podPos.x) * lerpFactor;
                    podPos.y += (targetY - podPos.y) * lerpFactor;
                }

                // Sync shoot command with owner
                if (registry.HasComponent<ShootCommand>(forcePodComp.owner) &&
                    registry.HasComponent<ShootCommand>(pod)) {

                    const auto& ownerShoot = registry.GetComponent<ShootCommand>(forcePodComp.owner);
                    auto& podShoot = registry.GetComponent<ShootCommand>(pod);

                    podShoot.wantsToShoot = ownerShoot.wantsToShoot;
                }
            }
        }

    }
}
