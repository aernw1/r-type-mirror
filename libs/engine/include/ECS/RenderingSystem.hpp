#pragma once

#include "ISystem.hpp"
#include "Renderer/IRenderer.hpp"

namespace RType {

    namespace ECS {

        class RenderingSystem : public ISystem {
        public:
            explicit RenderingSystem(Renderer::IRenderer* renderer);
            ~RenderingSystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "RenderingSystem"; }
        private:
            Renderer::IRenderer* m_renderer;
        };

    }

}
