#pragma once

#include "Audio/IAudio.hpp"

#include <SFML/Audio.hpp>
#include <unordered_map>
#include <list>
#include <memory>

namespace Audio {

    class SFMLAudio : public IAudio {
    public:
        SFMLAudio() = default;
        ~SFMLAudio() override = default;

        const char* GetName() const override { return "SFMLAudio"; }
        RType::Core::ModulePriority GetPriority() const override { return RType::Core::ModulePriority::Normal; }
        bool Initialize(RType::Core::Engine* /*engine*/) override { return true; }
        void Shutdown() override;
        void Update(float deltaTime) override;

        bool ConfigureDevice(const AudioConfig& config) override;

        SoundId LoadSound(const std::string& path) override;
        void UnloadSound(SoundId soundId) override;

        MusicId LoadMusic(const std::string& path) override;
        void UnloadMusic(MusicId musicId) override;

        void PlaySound(SoundId soundId, const PlaybackOptions& options = PlaybackOptions{}) override;
        void StopSound(SoundId soundId) override;

        void PlayMusic(MusicId musicId, const PlaybackOptions& options = PlaybackOptions{}) override;
        void StopMusic(MusicId musicId) override;

        void PauseAll() override;
        void ResumeAll() override;
        void StopAll() override;

        void SetMasterVolume(float volume) override;
        float GetMasterVolume() const override;

        void SetListener(const ListenerProperties& listener) override;

    private:
        void CleanupStoppedSounds();
        void UpdateAllVolumes();

        float m_masterVolume = 1.0f;
        SoundId m_nextSoundId = 1;
        MusicId m_nextMusicId = 1;

        std::unordered_map<SoundId, sf::SoundBuffer> m_soundBuffers;
        std::list<sf::Sound> m_activeSounds;
        std::unordered_map<const sf::SoundBuffer*, float> m_soundOriginalVolumes;

        std::unordered_map<MusicId, std::unique_ptr<sf::Music>> m_music;
        std::unordered_map<MusicId, float> m_musicOriginalVolumes;
    };

}

