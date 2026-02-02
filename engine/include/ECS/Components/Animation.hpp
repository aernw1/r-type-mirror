#pragma once
/**
 * @file Animation.hpp
 * @brief Generic animation components for any game engine.
 */

#include <cstring>
#include <array>
#include "Animation/AnimationTypes.hpp"
#include "Math/Types.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Components/IComponent.hpp"

namespace RType {
namespace ECS {

    /**
     * @brief Sprite animation playback component.
     */
    struct SpriteAnimation : public IComponent {
        Animation::AnimationClipId clipId = Animation::INVALID_CLIP_ID;
        float currentTime = 0.0f;
        float playbackSpeed = 1.0f;
        bool playing = true;
        bool looping = false;
        bool destroyOnComplete = false;

        std::size_t currentFrameIndex = 0;
        Math::Rectangle currentRegion{};

        SpriteAnimation() = default;
        SpriteAnimation(Animation::AnimationClipId clip, bool loop = false, float speed = 1.0f)
            : clipId(clip), playbackSpeed(speed), looping(loop) {}
    };

    /**
     * @brief Animation state machine for complex animations.
     */
    struct AnimationStateMachine : public IComponent {
        Animation::AnimationGraphId graphId = Animation::INVALID_GRAPH_ID;
        Animation::AnimationStateId currentState = Animation::INVALID_STATE_ID;
        Animation::AnimationStateId previousState = Animation::INVALID_STATE_ID;
        float stateTime = 0.0f;
        float blendFactor = 0.0f;
        float blendDuration = 0.0f;
        bool isTransitioning = false;

        static constexpr std::size_t MAX_PARAMS = 8;
        std::array<float, MAX_PARAMS> parameters{};
        std::array<std::array<char, 32>, MAX_PARAMS> parameterNames{};
        std::size_t parameterCount = 0;

        AnimationStateMachine() = default;
        explicit AnimationStateMachine(Animation::AnimationGraphId graph)
            : graphId(graph) {}

        void SetParameter(const char* name, float value) {
            for (std::size_t i = 0; i < parameterCount; ++i) {
                if (std::strncmp(parameterNames[i].data(), name, 31) == 0) {
                    parameters[i] = value;
                    return;
                }
            }
            if (parameterCount < MAX_PARAMS) {
                std::strncpy(parameterNames[parameterCount].data(), name, 31);
                parameterNames[parameterCount][31] = '\0';
                parameters[parameterCount] = value;
                parameterCount++;
            }
        }

        float GetParameter(const char* name) const {
            for (std::size_t i = 0; i < parameterCount; ++i) {
                if (std::strncmp(parameterNames[i].data(), name, 31) == 0) {
                    return parameters[i];
                }
            }
            return 0.0f;
        }
    };

    /**
     * @brief Marker for animated sprites that need frame updates.
     */
    struct AnimatedSprite : public IComponent {
        bool needsUpdate = true;

        AnimatedSprite() = default;
    };

    /**
     * @brief Visual effect component (explosions, particles, etc).
     */
    struct VisualEffect : public IComponent {
        Animation::EffectType type = Animation::EffectType::EXPLOSION_SMALL;
        float lifetime = 0.0f;
        float maxLifetime = 1.0f;
        Entity owner = NULL_ENTITY;
        float offsetX = 0.0f;
        float offsetY = 0.0f;

        VisualEffect() = default;
        VisualEffect(Animation::EffectType t, float duration)
            : type(t), maxLifetime(duration) {}
        VisualEffect(Animation::EffectType t, float duration, Entity ownerEntity, float offX, float offY)
            : type(t), maxLifetime(duration), owner(ownerEntity), offsetX(offX), offsetY(offY) {}
    };

    /**
     * @brief Animation events for triggering actions at specific frames.
     */
    struct AnimationEvents : public IComponent {
        static constexpr std::size_t MAX_EVENTS = 4;
        std::array<std::array<char, 32>, MAX_EVENTS> eventNames{};
        std::size_t eventCount = 0;

        AnimationEvents() = default;

        void PushEvent(const char* name) {
            if (eventCount < MAX_EVENTS && name) {
                std::strncpy(eventNames[eventCount].data(), name, 31);
                eventNames[eventCount][31] = '\0';
                eventCount++;
            }
        }

        void Clear() { eventCount = 0; }

        bool HasEvent(const char* name) const {
            for (std::size_t i = 0; i < eventCount; ++i) {
                if (std::strncmp(eventNames[i].data(), name, 31) == 0) {
                    return true;
                }
            }
            return false;
        }
    };

    /**
     * @brief Animation layer for blending multiple animations.
     */
    struct AnimationLayer : public IComponent {
        Animation::AnimationClipId clipId = Animation::INVALID_CLIP_ID;
        float currentTime = 0.0f;
        float weight = 1.0f;
        float playbackSpeed = 1.0f;
        bool additive = false;
        int layerIndex = 0;

        AnimationLayer() = default;
        AnimationLayer(Animation::AnimationClipId clip, int layer, float w = 1.0f)
            : clipId(clip), weight(w), layerIndex(layer) {}
    };

} // namespace ECS
} // namespace RType
