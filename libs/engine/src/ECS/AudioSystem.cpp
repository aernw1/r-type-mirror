#include "../../include/ECS/AudioSystem.hpp"
#include "../../include/ECS/Component.hpp"

namespace RType {
    namespace ECS {

        void AudioSystem::Update(Registry& registry, float) {
            if (!m_audio) {
                return;
            }

            auto musicEntities = registry.GetEntitiesWithComponent<MusicEffect>();
            for (auto entity : musicEntities) {
                if (!registry.IsEntityAlive(entity) ||
                    !registry.HasComponent<MusicEffect>(entity)) {
                    continue;
                }

                auto& music = registry.GetComponent<MusicEffect>(entity);
                if (music.musicId == Audio::INVALID_MUSIC_ID) {
                    registry.RemoveComponent<MusicEffect>(entity);
                    continue;
                }

                if (music.stop) {
                    m_audio->StopMusic(music.musicId);
                } else if (music.play) {
                    Audio::PlaybackOptions opts;
                    opts.volume = music.volume;
                    opts.pitch = music.pitch;
                    opts.loop = music.loop;
                    m_audio->PlayMusic(music.musicId, opts);
                }

                registry.RemoveComponent<MusicEffect>(entity);
            }

            auto entities = registry.GetEntitiesWithComponent<SoundEffect>();

            for (auto entity : entities) {
                if (!registry.IsEntityAlive(entity) ||
                    !registry.HasComponent<SoundEffect>(entity)) {
                    continue;
                }

                auto& sound = registry.GetComponent<SoundEffect>(entity);
                if (sound.soundId == Audio::INVALID_SOUND_ID) {
                    registry.RemoveComponent<SoundEffect>(entity);
                    continue;
                }

                Audio::PlaybackOptions opts;
                opts.volume = sound.volume;
                opts.pitch = sound.pitch;
                opts.pan = sound.pan;
                opts.loop = sound.loop;


                m_audio->PlaySound(sound.soundId, opts);
                registry.RemoveComponent<SoundEffect>(entity);
            }
        }

    }
}

