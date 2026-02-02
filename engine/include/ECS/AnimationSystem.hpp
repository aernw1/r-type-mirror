#pragma once

#include "ISystem.hpp"
#include "Animation/IAnimation.hpp"
#include <vector>

namespace RType {
namespace ECS {

    class AnimationSystem : public ISystem {
    public:
        explicit AnimationSystem(Animation::IAnimation* animation);
        ~AnimationSystem() override = default;

        const char* GetName() const override { return "AnimationSystem"; }
        void Update(Registry& registry, float deltaTime) override;

        void SetAnimationBackend(Animation::IAnimation* animation) { m_animation = animation; }
        Animation::IAnimation* GetAnimationBackend() const { return m_animation; }

    private:
        void UpdateSpriteAnimations(Registry& registry, float deltaTime);
        void UpdateVisualEffects(Registry& registry, float deltaTime);
        void UpdateFloatingTexts(Registry& registry, float deltaTime);
        void UpdatePowerUpGlow(Registry& registry, float deltaTime);
        void CleanupCompletedAnimations(Registry& registry);

        Animation::IAnimation* m_animation;
        std::vector<Entity> m_entitiesToDestroy;
    };

}
}
