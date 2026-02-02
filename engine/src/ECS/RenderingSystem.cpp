#include "ECS/RenderingSystem.hpp"
#include "ECS/Component.hpp"
#include "Core/ColorFilter.hpp"
#include <algorithm>

namespace RType {

    namespace ECS {

        RenderingSystem::RenderingSystem(Renderer::IRenderer* renderer)
            : m_renderer(renderer) {}

        void RenderingSystem::Update(Registry& registry, float deltaTime) {
            if (!m_renderer) {
                return;
            }

            auto entities = registry.GetEntitiesWithComponent<Drawable>();

            std::vector<std::pair<Entity, int>> renderableEntities;
            for (Entity entity : entities) {
                if (!registry.IsEntityAlive(entity)) {
                    continue;
                }
                if (registry.HasComponent<Position>(entity)) {
                    const auto& drawable = registry.GetComponent<Drawable>(entity);
                    renderableEntities.emplace_back(entity, drawable.layer);
                }
            }

            std::sort(renderableEntities.begin(), renderableEntities.end(),
                      [](const std::pair<Entity, int>& a, const std::pair<Entity, int>& b) {
                          return a.second < b.second;
                      });

            for (const auto& pair : renderableEntities) {
                Entity entity = pair.first;
                const auto& position = registry.GetComponent<Position>(entity);
                const auto& drawable = registry.GetComponent<Drawable>(entity);

                if (drawable.spriteId == Renderer::INVALID_SPRITE_ID) {
                    continue;
                }

                if (registry.HasComponent<AnimatedSprite>(entity) && registry.HasComponent<SpriteAnimation>(entity)) {
                    auto& animatedSprite = registry.GetComponent<AnimatedSprite>(entity);
                    const auto& anim = registry.GetComponent<SpriteAnimation>(entity);

                    if (anim.currentRegion.size.x > 0 && anim.currentRegion.size.y > 0) {
                        if (animatedSprite.needsUpdate) {
                            m_renderer->SetSpriteRegion(drawable.spriteId, anim.currentRegion);
                            animatedSprite.needsUpdate = false;
                        }
                    }
                }

                Renderer::Transform2D transform;
                transform.position = Renderer::Vector2(position.x, position.y);
                transform.scale = drawable.scale;
                transform.rotation = drawable.rotation;
                transform.origin = drawable.origin;

                Math::Color finalColor = drawable.tint;
                if (RType::Core::ColorFilter::IsColourBlindModeEnabled()) {
                    finalColor = RType::Core::ColorFilter::ApplyColourBlindFilter(drawable.tint);
                }

                m_renderer->DrawSprite(drawable.spriteId, transform, finalColor);
            }
        }
    }

}
