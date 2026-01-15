#pragma once

#include <unordered_map>
#include <memory>
#include "Animation/IAnimation.hpp"

namespace Animation {

    class AnimationModule : public IAnimation {
    public:
        AnimationModule();
        ~AnimationModule() override;

        const char* GetName() const override { return "AnimationModule"; }
        RType::Core::ModulePriority GetPriority() const override {
            return RType::Core::ModulePriority::Normal;
        }
        bool Initialize(RType::Core::Engine* engine) override;
        void Shutdown() override;
        void Update(float deltaTime) override;

        // Clip Management
        AnimationClipId CreateClip(const AnimationClipConfig& config) override;
        AnimationClipId CreateClipFromGrid(const std::string& name,
                                           const std::string& texturePath,
                                           const GridLayout& layout,
                                           bool looping) override;
        AnimationClipId LoadClipFromJson(const std::string& path) override;
        void DestroyClip(AnimationClipId clipId) override;
        const AnimationClipConfig* GetClipConfig(AnimationClipId clipId) const override;

        // Clip Introspection
        float GetClipDuration(AnimationClipId clipId) const override;
        std::size_t GetClipFrameCount(AnimationClipId clipId) const override;
        bool IsClipValid(AnimationClipId clipId) const override;

        // Frame Calculation
        FrameDef GetFrameAtTime(AnimationClipId clipId,
                                float time,
                                bool looping) const override;
        std::size_t GetFrameIndexAtTime(AnimationClipId clipId,
                                        float time,
                                        bool looping) const override;

        // Bulk Loading
        void LoadAnimationsFromManifest(const std::string& manifestPath) override;
        void UnloadAll() override;

        // Utility
        AnimationClipId GetClipByName(const std::string& name) const override;

    private:
        // Internal storage
        std::unordered_map<AnimationClipId, AnimationClipConfig> m_clips;
        std::unordered_map<std::string, AnimationClipId> m_clipNameToId;

        // ID generation
        AnimationClipId m_nextClipId = 1;

        // Engine reference
        RType::Core::Engine* m_engine = nullptr;

        // Helper functions
        float CalculateClipDuration(const AnimationClipConfig& config) const;
    };

}
