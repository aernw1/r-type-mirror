#include "ECS/RenderingSystem.hpp"
#include "ECS/Component.hpp"
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

            std::vector<Entity> renderableEntities;
            for (Entity entity : entities) {
                if (registry.HasComponent<Position>(entity)) {
                    renderableEntities.push_back(entity);
                }
            }

            std::sort(renderableEntities.begin(), renderableEntities.end(),
                      [&registry](Entity a, Entity b) {
                          const auto& drawableA = registry.GetComponent<Drawable>(a);
                          const auto& drawableB = registry.GetComponent<Drawable>(b);
                          return drawableA.layer < drawableB.layer;
                      });

            for (Entity entity : renderableEntities) {
                const auto& position = registry.GetComponent<Position>(entity);
                const auto& drawable = registry.GetComponent<Drawable>(entity);

                if (drawable.spriteId == Renderer::INVALID_SPRITE_ID) {
                    continue;
                }

                Renderer::Transform2D transform;
                transform.position = Renderer::Vector2(position.x, position.y);
                transform.scale = drawable.scale;
                transform.rotation = drawable.rotation;
                transform.origin = drawable.origin;

                m_renderer->DrawSprite(drawable.spriteId, transform, drawable.tint);
            }
        }
    }

}
