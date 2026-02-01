#include "Audio/SFMLAudio.hpp"
#include "Core/Logger.hpp"

#include <algorithm>

namespace Audio {

    bool SFMLAudio::ConfigureDevice(const AudioConfig& config) {
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
        m_musicOriginalVolumes.erase(musicId);
    }

    void SFMLAudio::PlaySound(SoundId soundId, const PlaybackOptions& options) {
        auto it = m_soundBuffers.find(soundId);
        if (it == m_soundBuffers.end()) {
            RType::Core::Logger::Warning("[SFMLAudio] PlaySound unknown id={}", soundId);
            return;
        }

        sf::Sound s;
        s.setBuffer(it->second);
        float finalVolume = std::clamp(options.volume * m_masterVolume, 0.0f, 1.0f) * 100.0f;
        s.setVolume(finalVolume);
        s.setPitch(std::max(0.01f, options.pitch));
        s.setLoop(options.loop);


        s.setRelativeToListener(true);
        s.setPosition(0.0f, 0.0f, 0.0f);
        s.setMinDistance(1.0f);
        s.setAttenuation(0.0f); // No attenuation



        s.play();
        m_activeSounds.push_back(std::move(s));
        m_soundOriginalVolumes[&it->second] = options.volume;
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
        float finalVolume = std::clamp(options.volume * m_masterVolume, 0.0f, 1.0f) * 100.0f;
        m.setVolume(finalVolume);
        m.setPitch(std::max(0.01f, options.pitch));
        m.setLoop(options.loop);
        m.play();
        m_musicOriginalVolumes[musicId] = options.volume;
    }

    void SFMLAudio::StopMusic(MusicId musicId) {
        auto it = m_music.find(musicId);
        if (it == m_music.end() || !it->second) {
            return;
        }
        it->second->stop();
        m_musicOriginalVolumes.erase(musicId);
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
        m_soundOriginalVolumes.clear();
        for (auto& [id, m] : m_music) {
            (void)id;
            if (m) {
                m->stop();
            }
        }
        m_musicOriginalVolumes.clear();
        CleanupStoppedSounds();
    }

    void SFMLAudio::SetMasterVolume(float volume) {
        m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
        UpdateAllVolumes();
    }

    void SFMLAudio::UpdateAllVolumes() {
        for (auto& s : m_activeSounds) {
            if (s.getStatus() == sf::Sound::Playing || s.getStatus() == sf::Sound::Paused) {
                const sf::SoundBuffer* buffer = s.getBuffer();
                auto volIt = m_soundOriginalVolumes.find(buffer);
                if (volIt != m_soundOriginalVolumes.end()) {
                    float finalVolume = std::clamp(volIt->second * m_masterVolume, 0.0f, 1.0f) * 100.0f;
                    s.setVolume(finalVolume);
                }
            }
        }

        for (auto& [id, m] : m_music) {
            if (m && (m->getStatus() == sf::Music::Playing || m->getStatus() == sf::Music::Paused)) {
                auto volIt = m_musicOriginalVolumes.find(id);
                if (volIt != m_musicOriginalVolumes.end()) {
                    float finalVolume = std::clamp(volIt->second * m_masterVolume, 0.0f, 1.0f) * 100.0f;
                    m->setVolume(finalVolume);
                }
            }
        }
    }

    float SFMLAudio::GetMasterVolume() const {
        return m_masterVolume;
    }

    void SFMLAudio::SetListener(const ListenerProperties& listener) {
        sf::Listener::setPosition(listener.position.x, listener.position.y, 0.0f);
        sf::Listener::setDirection(listener.forward.x, listener.forward.y, 0.0f);
    }

    void SFMLAudio::CleanupStoppedSounds() {
        m_activeSounds.remove_if([this](const sf::Sound& s) {
            if (s.getStatus() == sf::Sound::Stopped) {
                const sf::SoundBuffer* buffer = s.getBuffer();
                m_soundOriginalVolumes.erase(buffer);
                return true;
            }
            return false;
        });
    }

}

