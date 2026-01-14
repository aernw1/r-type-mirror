/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PowerUpCollisionSystem
*/

#pragma once

#include "ISystem.hpp"
#include "CollisionDetectionSystem.hpp"
#include "Renderer/IRenderer.hpp"
#include "../Audio/IAudio.hpp"

namespace RType {
    namespace ECS {

        class PowerUpCollisionSystem : public ISystem {
        public:
            explicit PowerUpCollisionSystem(Renderer::IRenderer* renderer = nullptr);
            ~PowerUpCollisionSystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "PowerUpCollisionSystem"; }

            void SetPowerUpSound(Audio::SoundId id) { m_powerUpSound = id; }

        private:
            Renderer::IRenderer* m_renderer;
            Audio::SoundId m_powerUpSound = Audio::INVALID_SOUND_ID;
        };

    }
}
