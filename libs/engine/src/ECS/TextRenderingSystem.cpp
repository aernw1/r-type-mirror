#include "ECS/TextRenderingSystem.hpp"
#include "ECS/Component.hpp"

namespace RType {

    namespace ECS {

        TextRenderingSystem::TextRenderingSystem(Renderer::IRenderer* renderer)
            : m_renderer(renderer) {}

        void TextRenderingSystem::Update(Registry& registry, float /* deltaTime */) {
            if (!m_renderer) {
                return;
            }

            auto entities = registry.GetEntitiesWithComponent<TextLabel>();

            for (Entity entity : entities) {
                if (!registry.HasComponent<Position>(entity)) {
                    continue;
                }

                const auto& position = registry.GetComponent<Position>(entity);
                const auto& label = registry.GetComponent<TextLabel>(entity);

                if (label.fontId == Renderer::INVALID_FONT_ID) {
                    continue;
                }

                Renderer::TextParams params;
                params.position = Math::Vector2{position.x, position.y};
                params.color = label.color;
                params.scale = label.scale;

                m_renderer->DrawText(label.fontId, label.text, params);
            }
        }
    }

}
