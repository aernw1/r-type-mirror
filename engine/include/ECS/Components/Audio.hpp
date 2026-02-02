#pragma once
/**
 * @file Audio.hpp
 * @brief Generic audio components for any game engine.
 */

#include "Audio/IAudio.hpp"
#include "ECS/Components/IComponent.hpp"

namespace RType {
namespace ECS {

    /**
     * @brief Sound effect component for one-shot or looping sounds.
     */
    struct SoundEffect : public IComponent {
        Audio::SoundId soundId = Audio::INVALID_SOUND_ID;
        float volume = 1.0f;
        float pitch = 1.0f;
        float pan = 0.0f;
        bool loop = false;
        bool positional = false;

        SoundEffect() = default;
        SoundEffect(Audio::SoundId id, float vol = 1.0f)
            : soundId(id), volume(vol) {}
    };

    /**
     * @brief Music/stream component for background music.
     */
    struct MusicEffect : public IComponent {
        Audio::MusicId musicId = Audio::INVALID_MUSIC_ID;
        bool play = true;
        bool stop = false;
        float volume = 1.0f;
        float pitch = 1.0f;
        bool loop = true;

        MusicEffect() = default;
        explicit MusicEffect(Audio::MusicId id)
            : musicId(id) {}
    };

} // namespace ECS
} // namespace RType
