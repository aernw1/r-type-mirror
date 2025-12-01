#pragma once

#include "ISystem.hpp"
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"
#include <string>

namespace RType {

    namespace ECS {

        class PlayerSystem : public ISystem {
        public:
            explicit PlayerSystem(Renderer::IRenderer* renderer = nullptr);
            ~PlayerSystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "PlayerSystem"; }

            static Entity CreatePlayer(Registry& registry, uint8_t playerNumber, uint64_t playerHash,
                                       float startX, float startY, Renderer::IRenderer* renderer);
            static void ClampPlayerToScreen(Registry& registry, Entity player, float screenWidth = 1280.0f,
                                            float screenHeight = 720.0f);
        private:
            Renderer::IRenderer* m_renderer;
            static Math::Color GetPlayerColor(uint8_t playerNumber);
            static std::string GetPlayerSpritePath(uint8_t playerNumber);
        };

    }

}
