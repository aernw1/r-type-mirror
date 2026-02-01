#pragma once

#include "ISystem.hpp"
#include "Renderer/IRenderer.hpp"

namespace RType {
    namespace ECS {

        class TextRenderingSystem : public ISystem {
        public:
            explicit TextRenderingSystem(Renderer::IRenderer* renderer);
            ~TextRenderingSystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "TextRenderingSystem"; }
        private:
            Renderer::IRenderer* m_renderer;
        };

    }
}
