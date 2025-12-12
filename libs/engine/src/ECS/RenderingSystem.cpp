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
