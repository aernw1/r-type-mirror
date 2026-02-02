#pragma once

#include "ISystem.hpp"
#include "Renderer/IRenderer.hpp"
#include <functional>

namespace RType {
    namespace ECS {

        class MenuSystem : public ISystem {
        public:
            using ActionCallback = std::function<void(int)>;

            explicit MenuSystem(Renderer::IRenderer* renderer);
            ~MenuSystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override {
                return "MenuSystem";
            }

            void SetActionCallback(ActionCallback callback) {
                m_callback = callback;
            }
        private:
            Renderer::IRenderer* m_renderer;
            ActionCallback m_callback;
            bool m_wasMouseDown = false;
        };

    }
}
