#include "Audio/SFMLAudio.hpp"
#include "Core/Logger.hpp"

#include <algorithm>

namespace Audio {

    bool SFMLAudio::ConfigureDevice(const AudioConfig& config) {
        // SFML handles device configuration internally; we mainly honor master volume.
        SetMasterVolume(config.masterVolume);
        return true;
    }

    void SFMLAudio::Shutdown() {
        StopAll();
        m_activeSounds.clear();
        m_soundBuffers.clear();
        m_music.clear();
    }

    void SFMLAudio::Update(float /*deltaTime*/) {
        CleanupStoppedSounds();
    }

    Audio::SoundId SFMLAudio::LoadSound(const std::string& path) {
        sf::SoundBuffer buffer;
        if (!buffer.loadFromFile(path)) {
            RType::Core::Logger::Warning("[SFMLAudio] Failed to load sound '{}'", path);
            return INVALID_SOUND_ID;
        }

        SoundId id = m_nextSoundId++;
        m_soundBuffers.emplace(id, std::move(buffer));
        RType::Core::Logger::Info("[SFMLAudio] Loaded sound '{}' (id={})", path, id);
        return id;
    }

    void SFMLAudio::UnloadSound(SoundId soundId) {
        StopSound(soundId);
        m_soundBuffers.erase(soundId);
    }

    Audio::MusicId SFMLAudio::LoadMusic(const std::string& path) {
        auto music = std::make_unique<sf::Music>();
        if (!music->openFromFile(path)) {
            RType::Core::Logger::Warning("[SFMLAudio] Failed to load music '{}'", path);
            return INVALID_MUSIC_ID;
        }

        MusicId id = m_nextMusicId++;
        m_music.emplace(id, std::move(music));
        return id;
    }

    void SFMLAudio::UnloadMusic(MusicId musicId) {
        StopMusic(musicId);
        m_music.erase(musicId);
    }

    void SFMLAudio::PlaySound(SoundId soundId, const PlaybackOptions& options) {
        auto it = m_soundBuffers.find(soundId);
        if (it == m_soundBuffers.end()) {
            RType::Core::Logger::Warning("[SFMLAudio] PlaySound unknown id={}", soundId);
            return;
        }

        sf::Sound s;
        s.setBuffer(it->second);
        s.setVolume(std::clamp(options.volume * m_masterVolume, 0.0f, 1.0f) * 100.0f);
        s.setPitch(std::max(0.01f, options.pitch));
        s.setLoop(options.loop);

        // SIMPLIFIED AUDIO LOGIC: Force 2D "flat" sound to ensure playback
        // We disable the specific mono/stereo branching that tried to use 3D positioning for pan.
        // This ensures sound is played directly without OpenAL spatialization quirks.
        s.setRelativeToListener(true);
        s.setPosition(0.0f, 0.0f, 0.0f);
        s.setMinDistance(1.0f);
        s.setAttenuation(0.0f); // No attenuation
        
        // If we really need pan later, we can re-enable it carefully, but let's fix playback first.
        // float pan = std::clamp(options.pan, -1.0f, 1.0f);
        // if (pan != 0.0f) s.setPosition(pan, 0.0f, 0.0f); // Simple pan if needed

        s.play();
        m_activeSounds.push_back(std::move(s));
        RType::Core::Logger::Debug("[SFMLAudio] PlaySound id={} vol={} pitch={} pan={} loop={}",
                                  soundId, options.volume, options.pitch, options.pan, options.loop);
    }

    void SFMLAudio::StopSound(SoundId soundId) {
        auto it = m_soundBuffers.find(soundId);
        if (it == m_soundBuffers.end()) {
            return;
        }
        const sf::SoundBuffer* buffer = &it->second;

        for (auto& s : m_activeSounds) {
            if (s.getBuffer() == buffer) {
                s.stop();
            }
        }
        CleanupStoppedSounds();
    }

    void SFMLAudio::PlayMusic(MusicId musicId, const PlaybackOptions& options) {
        auto it = m_music.find(musicId);
        if (it == m_music.end() || !it->second) {
            return;
        }
        auto& m = *it->second;
        m.setVolume(std::clamp(options.volume * m_masterVolume, 0.0f, 1.0f) * 100.0f);
        m.setPitch(std::max(0.01f, options.pitch));
        m.setLoop(options.loop);
        m.play();
    }

    void SFMLAudio::StopMusic(MusicId musicId) {
        auto it = m_music.find(musicId);
        if (it == m_music.end() || !it->second) {
            return;
        }
        it->second->stop();
    }

    void SFMLAudio::PauseAll() {
        for (auto& s : m_activeSounds) {
            if (s.getStatus() == sf::Sound::Playing) {
                s.pause();
            }
        }
        for (auto& [id, m] : m_music) {
            (void)id;
            if (m && m->getStatus() == sf::Music::Playing) {
                m->pause();
            }
        }
    }

    void SFMLAudio::ResumeAll() {
        for (auto& s : m_activeSounds) {
            if (s.getStatus() == sf::Sound::Paused) {
                s.play();
            }
        }
        for (auto& [id, m] : m_music) {
            (void)id;
            if (m && m->getStatus() == sf::Music::Paused) {
                m->play();
            }
        }
    }

    void SFMLAudio::StopAll() {
        for (auto& s : m_activeSounds) {
            s.stop();
        }
        for (auto& [id, m] : m_music) {
            (void)id;
            if (m) {
                m->stop();
            }
        }
        CleanupStoppedSounds();
    }

    void SFMLAudio::SetMasterVolume(float volume) {
        m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
    }

    float SFMLAudio::GetMasterVolume() const {
        return m_masterVolume;
    }

    void SFMLAudio::SetListener(const ListenerProperties& listener) {
        sf::Listener::setPosition(listener.position.x, listener.position.y, 0.0f);
        sf::Listener::setDirection(listener.forward.x, listener.forward.y, 0.0f);
        // SFML 2.6 no longer exposes listener velocity in the public API.
    }

    void SFMLAudio::CleanupStoppedSounds() {
        m_activeSounds.remove_if([](const sf::Sound& s) {
            return s.getStatus() == sf::Sound::Stopped;
        });
    }

}

