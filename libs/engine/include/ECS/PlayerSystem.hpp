#pragma once

#include "ISystem.hpp"
#include "Renderer/IRenderer.hpp"

namespace RType {

    namespace ECS {

        class PlayerSystem : public ISystem {
        public:
            explicit PlayerSystem(Renderer::IRenderer* renderer = nullptr);
            ~PlayerSystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "PlayerSystem"; }

            static void ClampPlayerToScreen(Registry& registry, Entity player, float screenWidth = 1280.0f,
                                          float screenHeight = 720.0f);

        private:
            Renderer::IRenderer* m_renderer;
        };

    }

}

