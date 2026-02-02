#pragma once

#include "ECS/ISystem.hpp"
#include "Renderer/IRenderer.hpp"

namespace Breakout {
    namespace ECS {

        class BreakoutRenderSystem : public RType::ECS::ISystem {
        public:
            explicit BreakoutRenderSystem(Renderer::IRenderer* renderer);
            ~BreakoutRenderSystem() override = default;

            const char* GetName() const override { return "BreakoutRenderSystem"; }
            void Update(RType::ECS::Registry& registry, float deltaTime) override;

        private:
            Renderer::IRenderer* m_renderer;
        };

    }
}
