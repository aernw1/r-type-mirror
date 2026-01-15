#include "ECS/EffectFactory.hpp"
#include "ECS/Component.hpp"
#include "ECS/Components/TextLabel.hpp"
#include "ECS/Registry.hpp"
#include "Core/Logger.hpp"
#include <cstdio>
#include <cstring>

namespace RType {
namespace ECS {

    EffectFactory::EffectFactory(const EffectConfig& config)
        : m_config(config) {}

    Entity EffectFactory::CreateBaseEffect(Registry& registry,
                                           float x, float y,
                                           Animation::EffectType type,
                                           float duration,
                                           Entity owner,
                                           float offsetX,
                                           float offsetY) {
        Entity entity = registry.CreateEntity();

        registry.AddComponent<Position>(entity, Position(x, y));
        registry.AddComponent<VisualEffect>(entity, VisualEffect(type, duration, owner, offsetX, offsetY));

        return entity;
    }

    Entity EffectFactory::CreateExplosionSmall(Registry& registry, float x, float y) {
        return CreateExplosion(registry, x, y, m_config.explosionSmall, 2.0f, 100);
    }

    Entity EffectFactory::CreateExplosionLarge(Registry& registry, float x, float y) {
        return CreateExplosion(registry, x, y, m_config.explosionLarge, 3.0f, 100);
    }

    Entity EffectFactory::CreateExplosion(Registry& registry,
                                          float x, float y,
                                          Animation::AnimationClipId clipId,
                                          float scale,
                                          int layer) {
        Entity entity = CreateBaseEffect(registry, x, y,
                                         Animation::EffectType::EXPLOSION_SMALL, 1.0f);

        if (clipId != Animation::INVALID_CLIP_ID) {
            auto& anim = registry.AddComponent<SpriteAnimation>(entity,
                SpriteAnimation(clipId, false, 1.5f));
            anim.destroyOnComplete = true;
            registry.AddComponent<AnimatedSprite>(entity);
        }

        auto& drawable = registry.AddComponent<Drawable>(entity, Drawable());
        drawable.layer = layer;
        drawable.scale = Math::Vector2(scale, scale);
        if (m_config.effectsSprite != Renderer::INVALID_SPRITE_ID) {
            drawable.spriteId = m_config.effectsSprite;
        }

        return entity;
    }

    Entity EffectFactory::CreateBulletImpact(Registry& registry, float x, float y) {
        Entity entity = CreateBaseEffect(registry, x, y,
                                         Animation::EffectType::BULLET_IMPACT, 0.3f);

        if (m_config.bulletImpact != Animation::INVALID_CLIP_ID) {
            auto& anim = registry.AddComponent<SpriteAnimation>(entity,
                SpriteAnimation(m_config.bulletImpact, false, 2.0f));
            anim.destroyOnComplete = true;
            registry.AddComponent<AnimatedSprite>(entity);
        }

        auto& drawable = registry.AddComponent<Drawable>(entity, Drawable());
        drawable.layer = 99;
        drawable.scale = Math::Vector2(0.5f, 0.5f);
        if (m_config.effectsSprite != Renderer::INVALID_SPRITE_ID) {
            drawable.spriteId = m_config.effectsSprite;
        }

        return entity;
    }

    Entity EffectFactory::CreateDamageNumber(Registry& registry,
                                             float x, float y,
                                             int damage,
                                             const Math::Color& color) {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "-%d", damage);
        return CreateFloatingText(registry, x, y - 20.0f, buffer, color, 1.5f);
    }

    Entity EffectFactory::CreateScorePopup(Registry& registry,
                                           float x, float y,
                                           int score,
                                           const Math::Color& color) {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "+%d", score);
        return CreateFloatingText(registry, x, y - 30.0f, buffer, color, 2.0f);
    }

    Entity EffectFactory::CreateFloatingText(Registry& registry,
                                             float x, float y,
                                             const char* text,
                                             const Math::Color& color,
                                             float duration) {
        Entity entity = registry.CreateEntity();

        registry.AddComponent<Position>(entity, Position(x, y));
        registry.AddComponent<FloatingText>(entity, FloatingText(text, duration, color));

        auto& label = registry.AddComponent<TextLabel>(entity);
        label.text = text ? text : "";
        label.fontId = m_config.damageFont;
        label.color = color;
        label.centered = true;

        return entity;
    }

    Entity EffectFactory::CreatePowerUpEffect(Registry& registry, float x, float y) {
        Entity entity = CreateBaseEffect(registry, x, y,
                                         Animation::EffectType::POWER_UP_COLLECT, 0.5f);

        if (m_config.powerUpCollect != Animation::INVALID_CLIP_ID) {
            auto& anim = registry.AddComponent<SpriteAnimation>(entity,
                SpriteAnimation(m_config.powerUpCollect, false, 1.5f));
            anim.destroyOnComplete = true;
            registry.AddComponent<AnimatedSprite>(entity);
        }

        auto& drawable = registry.AddComponent<Drawable>(entity, Drawable());
        drawable.layer = 101;
        if (m_config.effectsSprite != Renderer::INVALID_SPRITE_ID) {
            drawable.spriteId = m_config.effectsSprite;
        }

        return entity;
    }

    Entity EffectFactory::CreateBossTransitionEffect(Registry& registry, float x, float y) {
        Entity entity = CreateBaseEffect(registry, x, y,
                                         Animation::EffectType::BOSS_PHASE_TRANSITION, 2.0f);

        if (m_config.bossPhaseTransition != Animation::INVALID_CLIP_ID) {
            auto& anim = registry.AddComponent<SpriteAnimation>(entity,
                SpriteAnimation(m_config.bossPhaseTransition, false, 1.0f));
            anim.destroyOnComplete = true;
            registry.AddComponent<AnimatedSprite>(entity);
        }

        auto& drawable = registry.AddComponent<Drawable>(entity, Drawable());
        drawable.layer = 102;
        if (m_config.effectsSprite != Renderer::INVALID_SPRITE_ID) {
            drawable.spriteId = m_config.effectsSprite;
        }

        return entity;
    }

    Entity EffectFactory::CreateSpawnEffect(Registry& registry, float x, float y) {
        Entity entity = CreateBaseEffect(registry, x, y,
                                         Animation::EffectType::SPAWN_EFFECT, 0.5f);

        if (m_config.spawnEffect != Animation::INVALID_CLIP_ID) {
            auto& anim = registry.AddComponent<SpriteAnimation>(entity,
                SpriteAnimation(m_config.spawnEffect, false, 1.0f));
            anim.destroyOnComplete = true;
            registry.AddComponent<AnimatedSprite>(entity);
        }

        auto& drawable = registry.AddComponent<Drawable>(entity, Drawable());
        drawable.layer = 98;
        if (m_config.effectsSprite != Renderer::INVALID_SPRITE_ID) {
            drawable.spriteId = m_config.effectsSprite;
        }

        return entity;
    }

    Entity EffectFactory::CreateDeathEffect(Registry& registry, float x, float y) {
        Entity entity = CreateBaseEffect(registry, x, y,
                                         Animation::EffectType::DEATH_EFFECT, 0.8f);

        if (m_config.deathEffect != Animation::INVALID_CLIP_ID) {
            auto& anim = registry.AddComponent<SpriteAnimation>(entity,
                SpriteAnimation(m_config.deathEffect, false, 1.0f));
            anim.destroyOnComplete = true;
            registry.AddComponent<AnimatedSprite>(entity);
        }

        auto& drawable = registry.AddComponent<Drawable>(entity, Drawable());
        drawable.layer = 100;
        if (m_config.effectsSprite != Renderer::INVALID_SPRITE_ID) {
            drawable.spriteId = m_config.effectsSprite;
        }

        return entity;
    }

    void EffectFactory::CreateHitMarker(Registry& registry, Entity target, int damage) {
        if (!registry.IsEntityAlive(target)) {
            return;
        }

        if (registry.HasComponent<DamageFlash>(target)) {
            registry.GetComponent<DamageFlash>(target).Trigger();
        }

        if (registry.HasComponent<Position>(target)) {
            const auto& pos = registry.GetComponent<Position>(target);
            CreateDamageNumber(registry, pos.x, pos.y, damage);
        }
    }

    void EffectFactory::CreateEnemyDeathEffect(Registry& registry,
                                               float x, float y,
                                               int scoreValue) {
        CreateExplosionSmall(registry, x, y);

        if (scoreValue > 0) {
            CreateScorePopup(registry, x, y, scoreValue);
        }
    }

    Entity EffectFactory::CreateShootingEffect(Registry& registry, float x, float y, Entity owner) {
        if (m_config.shootingAnimation == Animation::INVALID_CLIP_ID) {
            return registry.CreateEntity();
        }

        constexpr float SHOOTING_EFFECT_OFFSET_X = 27.0f;
        constexpr float SHOOTING_EFFECT_OFFSET_Y = -10.0f;

        float offsetX = SHOOTING_EFFECT_OFFSET_X;
        float offsetY = SHOOTING_EFFECT_OFFSET_Y;
        
        if (owner != NULL_ENTITY && registry.IsEntityAlive(owner) &&
            registry.HasComponent<Shooter>(owner)) {
            const auto& shooterComp = registry.GetComponent<Shooter>(owner);
            offsetX = shooterComp.offsetX + SHOOTING_EFFECT_OFFSET_X;
            offsetY = shooterComp.offsetY + SHOOTING_EFFECT_OFFSET_Y;
        }
        
        Entity entity = CreateBaseEffect(registry, x + offsetX, y + offsetY,
                                         Animation::EffectType::CUSTOM, 0.4f, owner, offsetX, offsetY);

        Renderer::SpriteId spriteId = Renderer::INVALID_SPRITE_ID;
        if (m_config.shootingSprite != Renderer::INVALID_SPRITE_ID) {
            spriteId = m_config.shootingSprite;
        } else if (m_config.effectsSprite != Renderer::INVALID_SPRITE_ID) {
            spriteId = m_config.effectsSprite;
        }

        if (spriteId == Renderer::INVALID_SPRITE_ID) {
            return entity;
        }

        auto& drawable = registry.AddComponent<Drawable>(entity, Drawable());
        drawable.layer = 11;
        drawable.scale = Math::Vector2(2.0f, 2.0f);
        drawable.origin = Math::Vector2(12.5f, 12.5f);
        drawable.spriteId = spriteId;

        if (m_config.shootingAnimation != Animation::INVALID_CLIP_ID) {
            auto& anim = registry.AddComponent<SpriteAnimation>(entity,
                SpriteAnimation(m_config.shootingAnimation, false, 1.5f));
            anim.destroyOnComplete = true;
            anim.playbackSpeed = 1.5f;
            registry.AddComponent<AnimatedSprite>(entity);
        }

        return entity;
    }

}
}
