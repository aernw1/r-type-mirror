#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include "Math/Types.hpp"

namespace Animation {

    using AnimationClipId = std::uint32_t;
    using AnimationGraphId = std::uint32_t;
    using AnimationStateId = std::uint32_t;

    constexpr AnimationClipId INVALID_CLIP_ID = 0;
    constexpr AnimationGraphId INVALID_GRAPH_ID = 0;
    constexpr AnimationStateId INVALID_STATE_ID = 0;

    struct FrameDef {
        Math::Rectangle region{};
        float duration = 0.1f;
        std::string eventName;

        FrameDef() = default;
        FrameDef(const Math::Rectangle& reg, float dur, const std::string& event = "")
            : region(reg), duration(dur), eventName(event) {}
    };

    struct AnimationClipConfig {
        std::string name;
        std::string texturePath;
        std::vector<FrameDef> frames;
        bool looping = false;
        float playbackSpeed = 1.0f;

        AnimationClipConfig() = default;
    };

    struct GridLayout {
        std::uint32_t columns = 1;
        std::uint32_t rows = 1;
        std::uint32_t startFrame = 0;
        std::uint32_t frameCount = 0;
        float frameWidth = 0.0f;
        float frameHeight = 0.0f;
        float defaultDuration = 0.1f;

        GridLayout() = default;
    };

    enum class CompareOp : std::uint8_t {
        GREATER = 0,
        GREATER_EQUAL = 1,
        LESS = 2,
        LESS_EQUAL = 3,
        EQUAL = 4,
        NOT_EQUAL = 5
    };

    struct TransitionDef {
        AnimationStateId targetState = INVALID_STATE_ID;
        std::string conditionParam;
        CompareOp compareOp = CompareOp::GREATER_EQUAL;
        float conditionValue = 1.0f;
        float blendDuration = 0.0f;
        bool hasExitTime = false;
        float exitTimeNormalized = 1.0f;
        int priority = 0;

        TransitionDef() = default;
    };

    struct AnimationStateDef {
        std::string name;
        AnimationClipId clipId = INVALID_CLIP_ID;
        std::vector<TransitionDef> transitions;
        float speed = 1.0f;
        bool looping = true;

        AnimationStateDef() = default;
    };

    struct AnimationGraphDef {
        std::string name;
        std::vector<AnimationStateDef> states;
        AnimationStateId defaultStateId = INVALID_STATE_ID;
        std::vector<std::string> parameterNames;

        AnimationGraphDef() = default;
    };

    struct PlaybackOptions {
        float speed = 1.0f;
        bool reversed = false;
        float startTimeNormalized = 0.0f;
        bool destroyOnComplete = false;

        PlaybackOptions() = default;
    };

    using AnimationEventCallback = std::function<void(const std::string& eventName)>;

    enum class EffectType : std::uint8_t {
        EXPLOSION_SMALL = 0,
        EXPLOSION_LARGE = 1,
        BULLET_IMPACT = 2,
        HIT_MARKER = 3,
        POWER_UP_COLLECT = 4,
        BOSS_PHASE_TRANSITION = 5,
        SPAWN_EFFECT = 6,
        DEATH_EFFECT = 7,
        CUSTOM = 255
    };

}
