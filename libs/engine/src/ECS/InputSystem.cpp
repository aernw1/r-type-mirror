#include "../include/ECS/InputSystem.hpp"
#include "../include/ECS/Registry.hpp"
#include "../include/ECS/Component.hpp"

namespace RType {
    namespace ECS {

    InputSystem::InputSystem(Renderer::IRenderer* renderer)
        : m_renderer(renderer) {}

    void InputSystem::Update(Registry& registry, float deltaTime) {
        auto entities = registry.GetEntitiesWithComponent<Controllable>();

            for (Entity entity : entities) {
                if (!registry.HasComponent<Velocity>(entity)) {
                    continue;
                }

                auto& controllable = registry.GetComponent<Controllable>(entity);
                auto& vel = registry.GetComponent<Velocity>(entity);

                vel.dx = 0.0f;
                vel.dy = 0.0f;

                // ARROW UP or Z
                if (m_renderer->IsKeyPressed(Renderer::Key::Up) ||
                    m_renderer->IsKeyPressed(Renderer::Key::Z)) {
                    vel.dy = -controllable.speed;
                }

                // ARROW DOWN or S
                if (m_renderer->IsKeyPressed(Renderer::Key::Down) ||
                    m_renderer->IsKeyPressed(Renderer::Key::S)) {
                    vel.dy = controllable.speed;
                }

                // ARROW LEFT or Q
                if (m_renderer->IsKeyPressed(Renderer::Key::Left) ||
                    m_renderer->IsKeyPressed(Renderer::Key::A)) {
                    vel.dx = -controllable.speed;
                }

                // ARROW RIGHT or D
                if (m_renderer->IsKeyPressed(Renderer::Key::Right) ||
                    m_renderer->IsKeyPressed(Renderer::Key::D)) {
                    vel.dx = controllable.speed;
                }

                if (registry.HasComponent<ShootCommand>(entity)) {
                    auto& shootCmd = registry.GetComponent<ShootCommand>(entity);
                    shootCmd.wantsToShoot = m_renderer->IsKeyPressed(Renderer::Key::E);
                }

            }
        }
    }
}
