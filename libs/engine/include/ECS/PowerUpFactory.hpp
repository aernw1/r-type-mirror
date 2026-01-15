/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PowerUpFactory
*/

#pragma once

#include "Registry.hpp"
#include "Component.hpp"
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"
#include <string>

namespace RType {
    namespace ECS {

        class PowerUpFactory {
        public:
            // Create a powerup entity in the world
            static Entity CreatePowerUp(
                Registry& registry,
                PowerUpType type,
                float startX,
                float startY,
                Renderer::IRenderer* renderer
            );

            // Apply powerup effect to player
            static void ApplyPowerUpToPlayer(
                Registry& registry,
                Entity player,
                PowerUpType type,
                Renderer::IRenderer* renderer
            );

            // Create force pod entity (separate from powerup pickup)
            static Entity CreateForcePod(
                Registry& registry,
                Entity owner,
                Renderer::IRenderer* renderer
            );

            // Get powerup properties
            static Math::Color GetPowerUpColor(PowerUpType type);
            static const char* GetPowerUpSpritePath(PowerUpType type);
            static const char* GetPowerUpName(PowerUpType type);
            static float GetPowerUpScale(PowerUpType type);
        };

    }
}
