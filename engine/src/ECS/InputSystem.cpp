#include "../include/ECS/InputSystem.hpp"
#include "../include/ECS/Registry.hpp"
#include "../include/ECS/Component.hpp"
#include "../include/Core/InputMapping.hpp"

namespace RType {
    namespace ECS {

        InputSystem::InputSystem(Renderer::IRenderer* renderer)
            : m_renderer(renderer) {}

        void InputSystem::Update(Registry& registry, float deltaTime) {
            auto entities = registry.GetEntitiesWithComponent<Controllable>();

            Renderer::Key moveUpKey = Core::InputMapping::GetKey("MOVE_UP");
            Renderer::Key moveDownKey = Core::InputMapping::GetKey("MOVE_DOWN");
            Renderer::Key moveLeftKey = Core::InputMapping::GetKey("MOVE_LEFT");
            Renderer::Key moveRightKey = Core::InputMapping::GetKey("MOVE_RIGHT");
            Renderer::Key shootKey = Core::InputMapping::GetKey("SHOOT");

            if (moveUpKey == Renderer::Key::Unknown) moveUpKey = Renderer::Key::Up;
            if (moveDownKey == Renderer::Key::Unknown) moveDownKey = Renderer::Key::Down;
            if (moveLeftKey == Renderer::Key::Unknown) moveLeftKey = Renderer::Key::Left;
            if (moveRightKey == Renderer::Key::Unknown) moveRightKey = Renderer::Key::Right;
            if (shootKey == Renderer::Key::Unknown) shootKey = Renderer::Key::E;

            for (Entity entity : entities) {
                if (!registry.HasComponent<Velocity>(entity)) {
                    continue;
                }

                auto& controllable = registry.GetComponent<Controllable>(entity);
                auto& vel = registry.GetComponent<Velocity>(entity);

                vel.dx = 0.0f;
                vel.dy = 0.0f;

                if (m_renderer->IsKeyPressed(moveUpKey)) {
                    vel.dy = -controllable.speed;
                }

                if (m_renderer->IsKeyPressed(moveDownKey)) {
                    vel.dy = controllable.speed;
                }

                if (m_renderer->IsKeyPressed(moveLeftKey)) {
                    vel.dx = -controllable.speed;
                }

                if (m_renderer->IsKeyPressed(moveRightKey)) {
                    vel.dx = controllable.speed;
                }

                if (registry.HasComponent<ShootCommand>(entity)) {
                    auto& shootCmd = registry.GetComponent<ShootCommand>(entity);
                    shootCmd.wantsToShoot = m_renderer->IsKeyPressed(shootKey);
                }
            }
        }
    }
}
