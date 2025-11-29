#include "../../include/ECS/BulletCollisionSystem.hpp"
#include "../../include/ECS/ScoreSystem.hpp"
#include "../../include/ECS/Component.hpp"
#include "../../include/ECS/ShootingSystem.hpp"
#include "../../include/ECS/RenderingSystem.hpp"

namespace RType {
    namespace ECS {

        ShootingSystem::ShootingSystem(Renderer::IRenderer* renderer, Renderer::SpriteId bulletSprite)
            : m_renderer(renderer), m_bulletSprite(bulletSprite) {}

        void ShootingSystem::Update(Registry& registry, float deltaTime) {
            (void)deltaTime;

            auto shooters = registry.GetEntitiesWithComponent<Shooter>();
            auto bullets = registry.GetEntitiesWithComponent<Bullet>();

            for (auto shooterEntity : shooters) {
                if (!registry.IsEntityAlive(shooterEntity) || !registry.HasComponent<Shooter>(shooterEntity)) {
                    continue;
                }
                auto &shooterComp = registry.GetComponent<Shooter>(shooterEntity);

                shooterComp.cooldown = shooterComp.cooldown - deltaTime;

                if (m_renderer->IsKeyPressed(Renderer::Key::E) && shooterComp.cooldown <= 0.0f) {
                    if (!registry.HasComponent<Position>(shooterEntity)) {
                        continue;
                    }
                    const auto& positionComp = registry.GetComponent<Position>(shooterEntity);

                    auto bulletEntity = registry.CreateEntity();
                    registry.AddComponent<Position>(bulletEntity, Position(positionComp.x + 50.0f, positionComp.y + 20.0f));
                    registry.AddComponent<Velocity>(bulletEntity, Velocity(600.0f, 0.0f));
                    registry.AddComponent<Bullet>(bulletEntity, Bullet(shooterEntity));
                    registry.AddComponent<Drawable>(bulletEntity, Drawable(m_bulletSprite, 2));
                    registry.AddComponent<Damage>(bulletEntity, Damage(25));
                    registry.AddComponent<BoxCollider>(bulletEntity, BoxCollider(10.0f, 5.0f));

                    shooterComp.cooldown = shooterComp.fireRate;
                }
            }
        }
    }
}

