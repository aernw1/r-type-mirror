#pragma once

#include <cstdint>
#include <string>
#include "Core/Module.hpp"

namespace Audio {

    using SoundId = std::uint32_t;
    using MusicId = std::uint32_t;

    constexpr SoundId INVALID_SOUND_ID = 0;
    constexpr MusicId INVALID_MUSIC_ID = 0;

    struct Vector2 {
        float x = 0.0f;
        float y = 0.0f;

        Vector2() = default;
        Vector2(float x, float y)
            : x(x), y(y) {}
    };

    struct AudioConfig {
        std::uint32_t sampleRate = 44100;
        std::uint32_t channelCount = 2;
        float masterVolume = 1.0f;
        std::uint32_t streamingBufferSize = 4096;
    };

    struct PlaybackOptions {
        float volume = 1.0f;
        float pitch = 1.0f;
        float pan = 0.0f;
        bool loop = false;
    };

    struct ListenerProperties {
        Vector2 position{0.0f, 0.0f};
        Vector2 forward{0.0f, -1.0f};
        Vector2 velocity{0.0f, 0.0f};
    };

    class IAudio : public RType::Core::IModule {
    public:
        ~IAudio() override = default;

        const char* GetName() const override = 0;
        RType::Core::ModulePriority GetPriority() const override = 0;
        bool Initialize(RType::Core::Engine* engine) override = 0;
        void Shutdown() override = 0;
        void Update(float deltaTime) override = 0;

        virtual bool ConfigureDevice(const AudioConfig& config) = 0;

        virtual SoundId LoadSound(const std::string& path) = 0;
        virtual void UnloadSound(SoundId soundId) = 0;

        virtual MusicId LoadMusic(const std::string& path) = 0;
        virtual void UnloadMusic(MusicId musicId) = 0;

        virtual void PlaySound(SoundId soundId,
                               const PlaybackOptions& options = PlaybackOptions{}) = 0;
        virtual void StopSound(SoundId soundId) = 0;

        virtual void PlayMusic(MusicId musicId,
                               const PlaybackOptions& options = PlaybackOptions{}) = 0;
        virtual void StopMusic(MusicId musicId) = 0;

        virtual void PauseAll() = 0;
        virtual void ResumeAll() = 0;
        virtual void StopAll() = 0;

        virtual void SetMasterVolume(float volume) = 0;
        virtual float GetMasterVolume() const = 0;

        virtual void SetListener(const ListenerProperties& listener) = 0;
    };

}
