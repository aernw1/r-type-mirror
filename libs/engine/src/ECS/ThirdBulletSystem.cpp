#include "ECS/ThirdBulletSystem.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include <cmath>
#include <iostream>

namespace RType {
    namespace ECS {

        void ThirdBulletSystem::Update(Registry& registry, float deltaTime) {
            auto thirdBullets = registry.GetEntitiesWithComponent<ThirdBullet>();

            for (auto bulletEntity : thirdBullets) {
                if (!registry.IsEntityAlive(bulletEntity)) {
                    continue;
                }

                auto& thirdBullet = registry.GetComponent<ThirdBullet>(bulletEntity);
                if (!thirdBullet.isActive) {
                    continue;
                }

                const auto& pos = registry.GetComponent<Position>(bulletEntity);
                thirdBullet.timeSinceSpawn += deltaTime;

                if (thirdBullet.timeSinceSpawn >= thirdBullet.spawnInterval) {
                    thirdBullet.timeSinceSpawn = 0.0f;
                    SpawnSmallProjectile(registry, pos.x, pos.y);
                }
            }
        }

        void ThirdBulletSystem::SpawnSmallProjectile(Registry& registry, float x, float y) {
            const float speed = 300.0f;

            Direction directions[3] = {
                {0.0f, -speed},
                {0.0f, speed},
                {speed, 0.0f}
            };

            for (int i = 0; i < 3; i++) {
                Entity smallBullet = registry.CreateEntity();

                // CRITICAL FIX: Clean up obstacle components from entity ID reuse
                if (registry.HasComponent<Obstacle>(smallBullet)) {
                    std::cerr << "[THIRDBULLET CLEANUP] Removing Obstacle from bullet entity " << smallBullet << std::endl;
                    registry.RemoveComponent<Obstacle>(smallBullet);
                }
                if (registry.HasComponent<ObstacleMetadata>(smallBullet)) {
                    std::cerr << "[THIRDBULLET CLEANUP] Removing ObstacleMetadata from bullet entity " << smallBullet << std::endl;
                    registry.RemoveComponent<ObstacleMetadata>(smallBullet);
                }

                registry.AddComponent<Position>(smallBullet, Position{x, y});

                registry.AddComponent<Velocity>(smallBullet, Velocity{directions[i].vx, directions[i].vy});

                registry.AddComponent<BossBullet>(smallBullet, BossBullet{});
                registry.AddComponent<Bullet>(smallBullet, Bullet{});

                registry.AddComponent<CircleCollider>(smallBullet, CircleCollider{10.0f});
                registry.AddComponent<CollisionLayer>(smallBullet,
                    CollisionLayer(CollisionLayers::OBSTACLE, CollisionLayers::PLAYER));

                registry.AddComponent<Damage>(smallBullet, Damage{10});
            }

            Core::Logger::Debug("[ThirdBulletSystem] Spawned 3 cross projectiles at ({}, {})", x, y);
        }

    }
}
