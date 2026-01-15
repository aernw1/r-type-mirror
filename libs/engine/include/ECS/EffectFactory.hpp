#pragma once

#include "Entity.hpp"
#include "Animation/AnimationTypes.hpp"
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"

namespace RType {
namespace ECS {

    class Registry;

    struct EffectConfig {
        Animation::AnimationClipId explosionSmall = Animation::INVALID_CLIP_ID;
        Animation::AnimationClipId explosionLarge = Animation::INVALID_CLIP_ID;
        Animation::AnimationClipId bulletImpact = Animation::INVALID_CLIP_ID;
        Animation::AnimationClipId powerUpCollect = Animation::INVALID_CLIP_ID;
        Animation::AnimationClipId bossPhaseTransition = Animation::INVALID_CLIP_ID;
        Animation::AnimationClipId spawnEffect = Animation::INVALID_CLIP_ID;
        Animation::AnimationClipId deathEffect = Animation::INVALID_CLIP_ID;

        Renderer::FontId damageFont = Renderer::INVALID_FONT_ID;
        Renderer::TextureId effectsTexture = Renderer::INVALID_TEXTURE_ID;
        Renderer::SpriteId effectsSprite = Renderer::INVALID_SPRITE_ID;

        EffectConfig() = default;
    };

    class EffectFactory {
    public:
        explicit EffectFactory(const EffectConfig& config);
        ~EffectFactory() = default;

        void SetConfig(const EffectConfig& config) { m_config = config; }
        const EffectConfig& GetConfig() const { return m_config; }

        Entity CreateExplosionSmall(Registry& registry, float x, float y);

        Entity CreateExplosionLarge(Registry& registry, float x, float y);

        Entity CreateExplosion(Registry& registry,
                              float x, float y,
                              Animation::AnimationClipId clipId,
                              float scale = 1.0f,
                              int layer = 100);

        Entity CreateBulletImpact(Registry& registry, float x, float y);

        Entity CreateDamageNumber(Registry& registry,
                                  float x, float y,
                                  int damage,
                                  const Math::Color& color = {1.0f, 0.3f, 0.3f, 1.0f});

        Entity CreateScorePopup(Registry& registry,
                               float x, float y,
                               int score,
                               const Math::Color& color = {1.0f, 1.0f, 0.0f, 1.0f});

        Entity CreateFloatingText(Registry& registry,
                                 float x, float y,
                                 const char* text,
                                 const Math::Color& color,
                                 float duration = 1.5f);

        Entity CreatePowerUpEffect(Registry& registry, float x, float y);

        Entity CreateBossTransitionEffect(Registry& registry, float x, float y);

        Entity CreateSpawnEffect(Registry& registry, float x, float y);

        Entity CreateDeathEffect(Registry& registry, float x, float y);

        void CreateHitMarker(Registry& registry, Entity target, int damage);

        void CreateEnemyDeathEffect(Registry& registry,
                                   float x, float y,
                                   int scoreValue);

    private:
        EffectConfig m_config;

        Entity CreateBaseEffect(Registry& registry,
                               float x, float y,
                               Animation::EffectType type,
                               float duration);
    };

}
}
