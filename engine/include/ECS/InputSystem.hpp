#pragma once

#include "ISystem.hpp"
#include "Renderer/IRenderer.hpp"

namespace RType {
    namespace ECS {

        class InputSystem : public ISystem {
        public:
            InputSystem(Renderer::IRenderer* renderer);

            const char* GetName() const override { return "InputSystem"; }
            void Update(Registry& registry, float deltaTime) override;
        private:
            Renderer::IRenderer* m_renderer;
        };

    }
}