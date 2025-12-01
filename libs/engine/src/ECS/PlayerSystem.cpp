#include "ECS/PlayerSystem.hpp"
#include "ECS/Component.hpp"
#include <algorithm>

namespace RType {

    namespace ECS {

        PlayerSystem::PlayerSystem(Renderer::IRenderer* renderer)
            : m_renderer(renderer) {}

        void PlayerSystem::Update(Registry& registry, float /* deltaTime */) {
            auto players = registry.GetEntitiesWithComponent<Player>();

            for (Entity player : players) {
                if (!registry.HasComponent<Position>(player)) {
                    continue;
                }

                ClampPlayerToScreen(registry, player, 1280.0f, 720.0f);
            }
        }

        void PlayerSystem::ClampPlayerToScreen(Registry& registry, Entity player, float screenWidth,
                                             float screenHeight) {
            if (!registry.HasComponent<Position>(player)) {
                return;
            }

            auto& pos = registry.GetComponent<Position>(player);

            float minX = 0.0f;
            float maxX = screenWidth;
            float minY = 0.0f;
            float maxY = screenHeight;

            if (registry.HasComponent<BoxCollider>(player)) {
                const auto& collider = registry.GetComponent<BoxCollider>(player);
                maxX -= collider.width;
                maxY -= collider.height;
            }

            pos.x = std::clamp(pos.x, minX, maxX);
            pos.y = std::clamp(pos.y, minY, maxY);
        }


    }

}

