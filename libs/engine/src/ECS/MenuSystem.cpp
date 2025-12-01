#include "ECS/MenuSystem.hpp"
#include "ECS/Component.hpp"
#include "ECS/Components/Clickable.hpp"
#include "ECS/Components/TextLabel.hpp"

namespace RType {
    namespace ECS {

        MenuSystem::MenuSystem(Renderer::IRenderer* renderer)
            : m_renderer(renderer) {}

        void MenuSystem::Update(Registry& registry, float deltaTime) {
            if (!m_renderer)
                return;

            Renderer::Vector2 mousePos = m_renderer->GetMousePosition();
            bool isMouseDown = m_renderer->IsMouseButtonPressed(Renderer::IRenderer::MouseButton::Left);

            auto entities = registry.GetEntitiesWithComponent<Clickable>();

            for (Entity entity : entities) {
                if (!registry.HasComponent<Position>(entity))
                    continue;

                const auto& pos = registry.GetComponent<Position>(entity);
                auto& clickable = registry.GetComponent<Clickable>(entity);

                Drawable* drawable = nullptr;
                if (registry.HasComponent<Drawable>(entity)) {
                    drawable = &registry.GetComponent<Drawable>(entity);
                }

                TextLabel* textLabel = nullptr;
                if (registry.HasComponent<TextLabel>(entity)) {
                    textLabel = &registry.GetComponent<TextLabel>(entity);
                }

                bool hovered = (mousePos.x >= pos.x && mousePos.x <= pos.x + clickable.width &&
                                mousePos.y >= pos.y && mousePos.y <= pos.y + clickable.height);

                if (hovered) {
                    if (isMouseDown) {
                        clickable.state = ButtonState::Active;
                        if (drawable)
                            drawable->tint = {0.5f, 0.5f, 0.5f, 1.0f};
                        if (textLabel)
                            textLabel->color = {0.5f, 0.5f, 0.5f, 1.0f};
                    } else {
                        if (m_wasMouseDown && clickable.state == ButtonState::Active) {
                            if (m_callback) {
                                m_callback(clickable.actionId);
                            }
                        }
                        clickable.state = ButtonState::Hover;
                        if (drawable)
                            drawable->tint = {0.8f, 0.8f, 0.8f, 1.0f};
                        if (textLabel)
                            textLabel->color = {0.8f, 0.8f, 0.8f, 1.0f};
                    }
                } else {
                    clickable.state = ButtonState::Idle;
                    if (drawable)
                        drawable->tint = {1.0f, 1.0f, 1.0f, 1.0f};
                    if (textLabel)
                        textLabel->color = {1.0f, 1.0f, 1.0f, 1.0f};
                }
            }

            m_wasMouseDown = isMouseDown;
        }

    }
}
