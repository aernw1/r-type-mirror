#include "ECS/AnimationSystem.hpp"
#include "ECS/Component.hpp"
#include "ECS/Components/TextLabel.hpp"
#include "ECS/Registry.hpp"
#include <cmath>
#include <algorithm>

namespace RType {
namespace ECS {

    AnimationSystem::AnimationSystem(Animation::IAnimation* animation)
        : m_animation(animation) {}

    void AnimationSystem::Update(Registry& registry, float deltaTime) {
        if (!m_animation) {
            return;
        }

        m_entitiesToDestroy.clear();

        UpdateSpriteAnimations(registry, deltaTime);
        UpdateVisualEffects(registry, deltaTime);
        UpdateFloatingTexts(registry, deltaTime);
        UpdatePowerUpGlow(registry, deltaTime);
        CleanupCompletedAnimations(registry);
    }

    void AnimationSystem::UpdateSpriteAnimations(Registry& registry, float deltaTime) {
        auto entities = registry.GetEntitiesWithComponent<SpriteAnimation>();

        for (Entity entity : entities) {
            if (!registry.IsEntityAlive(entity)) {
                continue;
            }

            auto& anim = registry.GetComponent<SpriteAnimation>(entity);
            if (!anim.playing || anim.clipId == Animation::INVALID_CLIP_ID) {
                continue;
            }

            float clipDuration = m_animation->GetClipDuration(anim.clipId);
            if (clipDuration <= 0.0f) {
                continue;
            }

            bool regionUninitialized = (anim.currentRegion.size.x <= 0.0f || anim.currentRegion.size.y <= 0.0f);
            if (regionUninitialized && anim.currentTime == 0.0f) {
                auto firstFrame = m_animation->GetFrameAtTime(anim.clipId, 0.0f, anim.looping);
                std::size_t firstFrameIndex = m_animation->GetFrameIndexAtTime(anim.clipId, 0.0f, anim.looping);
                anim.currentFrameIndex = firstFrameIndex;
                anim.currentRegion = firstFrame.region;
                if (registry.HasComponent<AnimatedSprite>(entity)) {
                    registry.GetComponent<AnimatedSprite>(entity).needsUpdate = true;
                }
            }

            anim.currentTime += deltaTime * anim.playbackSpeed;
            if (anim.currentTime >= clipDuration) {
                if (anim.looping) {
                    anim.currentTime = std::fmod(anim.currentTime, clipDuration);
                } else {
                    anim.currentTime = clipDuration;
                    anim.playing = false;

                    if (anim.destroyOnComplete) {
                        m_entitiesToDestroy.push_back(entity);
                        continue;
                    }
                }
            }

            auto frame = m_animation->GetFrameAtTime(anim.clipId, anim.currentTime, anim.looping);
            std::size_t newFrameIndex = m_animation->GetFrameIndexAtTime(
                anim.clipId, anim.currentTime, anim.looping);
            
            bool frameChanged = (newFrameIndex != anim.currentFrameIndex);
            if (frameChanged || frame.region.size.x > 0.0f) {
                anim.currentRegion = frame.region;
                anim.currentFrameIndex = newFrameIndex;

                if (!frame.eventName.empty() && registry.HasComponent<AnimationEvents>(entity)) {
                    auto& events = registry.GetComponent<AnimationEvents>(entity);
                    events.PushEvent(frame.eventName.c_str());
                }
                
                if (registry.HasComponent<AnimatedSprite>(entity)) {
                    registry.GetComponent<AnimatedSprite>(entity).needsUpdate = true;
                }
            }
        }
    }

    void AnimationSystem::UpdateVisualEffects(Registry& registry, float deltaTime) {
        auto entities = registry.GetEntitiesWithComponent<VisualEffect>();

        for (Entity entity : entities) {
            if (!registry.IsEntityAlive(entity)) {
                continue;
            }

            auto& effect = registry.GetComponent<VisualEffect>(entity);
            effect.lifetime += deltaTime;

            if (effect.owner != NULL_ENTITY && registry.IsEntityAlive(effect.owner) &&
                registry.HasComponent<Position>(effect.owner) &&
                registry.HasComponent<Position>(entity)) {
                const auto& ownerPos = registry.GetComponent<Position>(effect.owner);
                auto& effectPos = registry.GetComponent<Position>(entity);
                
                float offsetX = effect.offsetX;
                float offsetY = effect.offsetY;
                
                effectPos.x = ownerPos.x + offsetX;
                effectPos.y = ownerPos.y + offsetY;
            }

            if (effect.lifetime >= effect.maxLifetime) {
                m_entitiesToDestroy.push_back(entity);
            }
        }
    }

    void AnimationSystem::UpdateFloatingTexts(Registry& registry, float deltaTime) {
        auto entities = registry.GetEntitiesWithComponent<FloatingText>();

        for (Entity entity : entities) {
            if (!registry.IsEntityAlive(entity)) {
                continue;
            }

            auto& text = registry.GetComponent<FloatingText>(entity);
            text.lifetime += deltaTime;

            if (registry.HasComponent<Position>(entity)) {
                auto& pos = registry.GetComponent<Position>(entity);
                pos.y += text.velocityY * deltaTime;
            }
            if (text.lifetime >= text.fadeStartTime && text.maxLifetime > text.fadeStartTime) {
                float fadeProgress = (text.lifetime - text.fadeStartTime) /
                                    (text.maxLifetime - text.fadeStartTime);
                fadeProgress = std::clamp(fadeProgress, 0.0f, 1.0f);
                text.color.a = 1.0f - fadeProgress;

                if (registry.HasComponent<TextLabel>(entity)) {
                    auto& label = registry.GetComponent<TextLabel>(entity);
                    label.color.a = text.color.a;
                }
            }

            if (text.lifetime >= text.maxLifetime) {
                m_entitiesToDestroy.push_back(entity);
            }
        }
    }

    void AnimationSystem::UpdatePowerUpGlow(Registry& registry, float deltaTime) {
        auto entities = registry.GetEntitiesWithComponent<PowerUpGlow>();

        for (Entity entity : entities) {
            if (!registry.IsEntityAlive(entity)) {
                continue;
            }

            auto& glow = registry.GetComponent<PowerUpGlow>(entity);
            glow.time += deltaTime * glow.pulseSpeed;

            float pulse = (std::sin(glow.time) + 1.0f) * 0.5f;
            float alpha = glow.minAlpha + (glow.maxAlpha - glow.minAlpha) * pulse;
            float scaleMultiplier = 1.0f + glow.scalePulse * (pulse * 2.0f - 1.0f);
            float scale = glow.baseScale * scaleMultiplier;

            if (registry.HasComponent<Drawable>(entity)) {
                auto& drawable = registry.GetComponent<Drawable>(entity);
                drawable.tint.a = alpha;
                drawable.scale.x = scale;
                drawable.scale.y = scale;
            }
        }
    }

    void AnimationSystem::CleanupCompletedAnimations(Registry& registry) {
        std::sort(m_entitiesToDestroy.begin(), m_entitiesToDestroy.end());
        m_entitiesToDestroy.erase(
            std::unique(m_entitiesToDestroy.begin(), m_entitiesToDestroy.end()),
            m_entitiesToDestroy.end());

        for (Entity entity : m_entitiesToDestroy) {
            if (registry.IsEntityAlive(entity)) {
                registry.DestroyEntity(entity);
            }
        }
    }

}
}
