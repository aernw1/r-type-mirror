#include "BreakoutRenderSystem.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"

namespace Breakout {
    namespace ECS {

        BreakoutRenderSystem::BreakoutRenderSystem(Renderer::IRenderer* renderer)
            : m_renderer(renderer) {}

        void BreakoutRenderSystem::Update(RType::ECS::Registry& registry, float /*deltaTime*/) {
            if (!m_renderer) {
                return;
            }

            auto boxEntities = registry.GetEntitiesWithComponent<RType::ECS::BoxCollider>();
            for (auto entity : boxEntities) {
                if (!registry.IsEntityAlive(entity) || !registry.HasComponent<RType::ECS::Position>(entity)) {
                    continue;
                }

                const auto& pos = registry.GetComponent<RType::ECS::Position>(entity);
                const auto& collider = registry.GetComponent<RType::ECS::BoxCollider>(entity);

                if (registry.HasComponent<RType::ECS::Drawable>(entity)) {
                    const auto& drawable = registry.GetComponent<RType::ECS::Drawable>(entity);
                    if (drawable.spriteId != Renderer::INVALID_SPRITE_ID) {
                        continue;
                    }

                    Renderer::Rectangle rect;
                    rect.position = Renderer::Vector2(pos.x - collider.width / 2.0f, pos.y - collider.height / 2.0f);
                    rect.size = Renderer::Vector2(collider.width, collider.height);
                    m_renderer->DrawRectangle(rect, drawable.tint);
                }
            }

            auto circleEntities = registry.GetEntitiesWithComponent<RType::ECS::CircleCollider>();
            for (auto entity : circleEntities) {
                if (!registry.IsEntityAlive(entity) || !registry.HasComponent<RType::ECS::Position>(entity)) {
                    continue;
                }

                const auto& pos = registry.GetComponent<RType::ECS::Position>(entity);
                const auto& collider = registry.GetComponent<RType::ECS::CircleCollider>(entity);

                if (registry.HasComponent<RType::ECS::Drawable>(entity)) {
                    const auto& drawable = registry.GetComponent<RType::ECS::Drawable>(entity);
                    if (drawable.spriteId != Renderer::INVALID_SPRITE_ID) {
                        continue;
                    }

                    float radius = collider.radius;
                    Renderer::Rectangle rect;
                    rect.position = Renderer::Vector2(pos.x - radius, pos.y - radius);
                    rect.size = Renderer::Vector2(radius * 2.0f, radius * 2.0f);
                    m_renderer->DrawRectangle(rect, drawable.tint);
                }
            }
        }

    }
}
