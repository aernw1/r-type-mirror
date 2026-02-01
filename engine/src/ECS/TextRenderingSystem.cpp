#include "ECS/TextRenderingSystem.hpp"
#include "ECS/Component.hpp"
#include "ECS/Components/TextLabel.hpp"
#include "Core/ColorFilter.hpp"

namespace RType {
    namespace ECS {

        TextRenderingSystem::TextRenderingSystem(Renderer::IRenderer* renderer)
            : m_renderer(renderer) {}

        void TextRenderingSystem::Update(Registry& registry, float deltaTime) {
            if (!m_renderer)
                return;

            auto entities = registry.GetEntitiesWithComponent<TextLabel>();

            for (Entity entity : entities) {
                if (!registry.IsEntityAlive(entity)) {
                    continue;
                }
                if (!registry.HasComponent<Position>(entity))
                    continue;

                const auto& pos = registry.GetComponent<Position>(entity);
                const auto& label = registry.GetComponent<TextLabel>(entity);

                Renderer::TextParams params;
                params.position = {pos.x + label.offsetX, pos.y + label.offsetY};
                Math::Color finalColor = label.color;
                if (RType::Core::ColorFilter::IsColourBlindModeEnabled()) {
                    finalColor = RType::Core::ColorFilter::ApplyColourBlindFilter(label.color);
                }
                params.color = finalColor;
                params.scale = 1.0f;
                params.rotation = 0.0f;
                params.centered = label.centered;

                m_renderer->DrawText(label.fontId, label.text, params);
            }
        }
    }
}
