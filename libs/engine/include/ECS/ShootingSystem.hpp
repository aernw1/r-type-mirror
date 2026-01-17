#pragma once

#include "ISystem.hpp"
#include "../Renderer/IRenderer.hpp"
#include "../Audio/IAudio.hpp"

namespace RType {
    namespace ECS {

        class EffectFactory;

        class ShootingSystem : public ISystem {
        public:
            ShootingSystem(Renderer::SpriteId bulletSprite);
            ~ShootingSystem() override = default;

            const char* GetName() const override { return "ShootingSystem"; }

            void Update(Registry& registry, float deltaTime) override;

            void SetShootSound(Audio::SoundId soundId) { m_shootSound = soundId; }
            void SetEffectFactory(EffectFactory* effectFactory) { m_effectFactory = effectFactory; }
        private:
            void CreateSpreadShot(Registry& registry, Entity shooter, const Position& pos, int damage);
            void CreateLaserShot(Registry& registry, Entity shooter, const Position& pos, int damage);

            Renderer::SpriteId m_bulletSprite;
            Audio::SoundId m_shootSound = Audio::INVALID_SOUND_ID;
            EffectFactory* m_effectFactory = nullptr;
        };
    }
}