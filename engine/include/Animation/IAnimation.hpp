#pragma once

#include <optional>
#include <string>
#include "Core/Module.hpp"
#include "Animation/AnimationTypes.hpp"

namespace Animation {

    class IAnimation : public RType::Core::IModule {
    public:
        ~IAnimation() override = default;

        const char* GetName() const override = 0;
        RType::Core::ModulePriority GetPriority() const override = 0;
        bool Initialize(RType::Core::Engine* engine) override = 0;
        void Shutdown() override = 0;
        void Update(float deltaTime) override = 0;

        // Create a clip from explicit configuration
        virtual AnimationClipId CreateClip(const AnimationClipConfig& config) = 0;

        // Create a clip from a grid layout (uniform sprite sheet)
        virtual AnimationClipId CreateClipFromGrid(const std::string& name,
                                                   const std::string& texturePath,
                                                   const GridLayout& layout,
                                                   bool looping = false) = 0;

        // Load a clip definition from a JSON file
        virtual AnimationClipId LoadClipFromJson(const std::string& path) = 0;

        // Destroy a clip and release its resources
        virtual void DestroyClip(AnimationClipId clipId) = 0;

        // Get the configuration of a clip (nullptr if not found)
        virtual const AnimationClipConfig* GetClipConfig(AnimationClipId clipId) const = 0;

        // Get total duration of a clip in seconds
        virtual float GetClipDuration(AnimationClipId clipId) const = 0;

        // Get number of frames in a clip
        virtual std::size_t GetClipFrameCount(AnimationClipId clipId) const = 0;

        // Check if a clip exists and is valid
        virtual bool IsClipValid(AnimationClipId clipId) const = 0;

        // Get the frame definition at a given time
        // Returns the frame data including texture region
        virtual FrameDef GetFrameAtTime(AnimationClipId clipId,
                                        float time,
                                        bool looping) const = 0;

        // Get the frame index at a given time
        virtual std::size_t GetFrameIndexAtTime(AnimationClipId clipId,
                                                float time,
                                                bool looping) const = 0;

        // Load multiple animations from a manifest JSON file
        virtual void LoadAnimationsFromManifest(const std::string& manifestPath) = 0;

        // Unload all loaded animations
        virtual void UnloadAll() = 0;

        // Get a clip ID by name (returns INVALID_CLIP_ID if not found)
        virtual AnimationClipId GetClipByName(const std::string& name) const = 0;
    };

}
