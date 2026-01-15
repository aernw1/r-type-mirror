#include "Animation/AnimationModule.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <cmath>

namespace Animation {

    AnimationModule::AnimationModule() = default;

    AnimationModule::~AnimationModule() {
        Shutdown();
    }

    bool AnimationModule::Initialize(RType::Core::Engine* engine) {
        m_engine = engine;
        return true;
    }

    void AnimationModule::Shutdown() {
        UnloadAll();
        m_engine = nullptr;
    }

    void AnimationModule::Update(float /* deltaTime */) {
    }

    AnimationClipId AnimationModule::CreateClip(const AnimationClipConfig& config) {
        if (config.name.empty() || config.frames.empty()) {
            return INVALID_CLIP_ID;
        }

        AnimationClipId id = m_nextClipId++;
        m_clips[id] = config;
        m_clipNameToId[config.name] = id;

        return id;
    }

    AnimationClipId AnimationModule::CreateClipFromGrid(const std::string& name,
                                                        const std::string& texturePath,
                                                        const GridLayout& layout,
                                                        bool looping) {
        if (name.empty() || layout.columns == 0 || layout.rows == 0) {
            return INVALID_CLIP_ID;
        }

        AnimationClipConfig config;
        config.name = name;
        config.texturePath = texturePath;
        config.looping = looping;

        std::uint32_t totalFrames = layout.columns * layout.rows;
        std::uint32_t frameCount = (layout.frameCount > 0) ?
            std::min(layout.frameCount, totalFrames - layout.startFrame) :
            totalFrames - layout.startFrame;

        config.frames.reserve(frameCount);

        for (std::uint32_t i = 0; i < frameCount; ++i) {
            std::uint32_t frameIndex = layout.startFrame + i;
            std::uint32_t col = frameIndex % layout.columns;
            std::uint32_t row = frameIndex / layout.columns;

            FrameDef frame;
            frame.region.position.x = static_cast<float>(col) * layout.frameWidth;
            frame.region.position.y = static_cast<float>(row) * layout.frameHeight;
            frame.region.size.x = layout.frameWidth;
            frame.region.size.y = layout.frameHeight;
            frame.duration = layout.defaultDuration;

            config.frames.push_back(frame);
        }

        return CreateClip(config);
    }

    AnimationClipId AnimationModule::LoadClipFromJson(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return INVALID_CLIP_ID;
        }

        try {
            nlohmann::json json;
            file >> json;

            AnimationClipConfig config;
            config.name = json.value("name", "");
            config.texturePath = json.value("texture", "");
            config.looping = json.value("looping", false);
            config.playbackSpeed = json.value("playbackSpeed", 1.0f);

            if (json.contains("gridLayout")) {
                const auto& grid = json["gridLayout"];
                GridLayout layout;
                layout.columns = grid.value("columns", 1u);
                layout.rows = grid.value("rows", 1u);
                layout.startFrame = grid.value("startFrame", 0u);
                layout.frameCount = grid.value("frameCount", 0u);
                layout.frameWidth = grid.value("frameWidth", 0.0f);
                layout.frameHeight = grid.value("frameHeight", 0.0f);
                layout.defaultDuration = grid.value("defaultDuration", 0.1f);

                if (layout.frameWidth <= 0.0f && json.contains("frameWidth")) {
                    layout.frameWidth = json["frameWidth"].get<float>();
                }
                if (layout.frameHeight <= 0.0f && json.contains("frameHeight")) {
                    layout.frameHeight = json["frameHeight"].get<float>();
                }

                return CreateClipFromGrid(config.name, config.texturePath, layout, config.looping);
            }

            if (json.contains("frames")) {
                for (const auto& frameJson : json["frames"]) {
                    FrameDef frame;
                    frame.region.position.x = frameJson.value("x", 0.0f);
                    frame.region.position.y = frameJson.value("y", 0.0f);
                    frame.region.size.x = frameJson.value("width", 0.0f);
                    frame.region.size.y = frameJson.value("height", 0.0f);
                    frame.duration = frameJson.value("duration", 0.1f);
                    frame.eventName = frameJson.value("event", "");
                    config.frames.push_back(frame);
                }
            }

            return CreateClip(config);

        } catch (const std::exception&) {
            return INVALID_CLIP_ID;
        }
    }

    void AnimationModule::DestroyClip(AnimationClipId clipId) {
        auto it = m_clips.find(clipId);
        if (it != m_clips.end()) {
            m_clipNameToId.erase(it->second.name);
            m_clips.erase(it);
        }
    }

    const AnimationClipConfig* AnimationModule::GetClipConfig(AnimationClipId clipId) const {
        auto it = m_clips.find(clipId);
        return (it != m_clips.end()) ? &it->second : nullptr;
    }

    float AnimationModule::GetClipDuration(AnimationClipId clipId) const {
        const auto* config = GetClipConfig(clipId);
        if (!config) {
            return 0.0f;
        }
        return CalculateClipDuration(*config);
    }

    std::size_t AnimationModule::GetClipFrameCount(AnimationClipId clipId) const {
        const auto* config = GetClipConfig(clipId);
        return config ? config->frames.size() : 0;
    }

    bool AnimationModule::IsClipValid(AnimationClipId clipId) const {
        return m_clips.find(clipId) != m_clips.end();
    }

    FrameDef AnimationModule::GetFrameAtTime(AnimationClipId clipId,
                                             float time,
                                             bool looping) const {
        const auto* config = GetClipConfig(clipId);
        if (!config || config->frames.empty()) {
            return FrameDef{};
        }

        std::size_t frameIndex = GetFrameIndexAtTime(clipId, time, looping);
        if (frameIndex < config->frames.size()) {
            return config->frames[frameIndex];
        }

        return config->frames.back();
    }

    std::size_t AnimationModule::GetFrameIndexAtTime(AnimationClipId clipId,
                                                     float time,
                                                     bool looping) const {
        const auto* config = GetClipConfig(clipId);
        if (!config || config->frames.empty()) {
            return 0;
        }

        float duration = CalculateClipDuration(*config);
        if (duration <= 0.0f) {
            return 0;
        }

        float normalizedTime = time;
        if (looping) {
            normalizedTime = std::fmod(time, duration);
            if (normalizedTime < 0.0f) {
                normalizedTime += duration;
            }
        } else {
            normalizedTime = std::clamp(time, 0.0f, duration);
        }

        float accumulated = 0.0f;
        for (std::size_t i = 0; i < config->frames.size(); ++i) {
            accumulated += config->frames[i].duration;
            if (normalizedTime < accumulated) {
                return i;
            }
        }

        return config->frames.size() - 1;
    }

    void AnimationModule::LoadAnimationsFromManifest(const std::string& manifestPath) {
        std::ifstream file(manifestPath);
        if (!file.is_open()) {
            return;
        }

        try {
            nlohmann::json json;
            file >> json;

            if (json.contains("clips")) {
                for (const auto& clipPath : json["clips"]) {
                    LoadClipFromJson(clipPath.get<std::string>());
                }
            }

        } catch (const std::exception&) {
        }
    }

    void AnimationModule::UnloadAll() {
        m_clips.clear();
        m_clipNameToId.clear();
        m_nextClipId = 1;
    }

    AnimationClipId AnimationModule::GetClipByName(const std::string& name) const {
        auto it = m_clipNameToId.find(name);
        return (it != m_clipNameToId.end()) ? it->second : INVALID_CLIP_ID;
    }

    float AnimationModule::CalculateClipDuration(const AnimationClipConfig& config) const {
        float duration = 0.0f;
        for (const auto& frame : config.frames) {
            duration += frame.duration;
        }
        return duration;
    }

}
