#pragma once

#include "ISystem.hpp"
#include "../Audio/IAudio.hpp"

namespace RType {
    namespace ECS {

        class AudioSystem : public ISystem {
        public:
            explicit AudioSystem(Audio::IAudio* audio)
                : m_audio(audio) {}

            ~AudioSystem() override = default;

            const char* GetName() const override { return "AudioSystem"; }

            void Update(Registry& registry, float deltaTime) override;

            void SetAudioBackend(Audio::IAudio* audio) { m_audio = audio; }

        private:
            Audio::IAudio* m_audio; 
        };

    }
}

