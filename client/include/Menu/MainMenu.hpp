#pragma once

#include "../GameStateMachine.hpp"
#include "ECS/Registry.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/TextRenderingSystem.hpp"
#include "ECS/MenuSystem.hpp"
#include <memory>
#include <vector>

namespace RType {
    namespace Client {

        class MainMenu : public IState {
        public:
            MainMenu(GameStateMachine& machine, GameContext& context);
            ~MainMenu() override = default;

            void Init() override;
            void Cleanup() override;
            void HandleInput() override;
            void Update(float dt) override;
            void Draw() override;

        private:
            GameStateMachine& m_machine;
            GameContext& m_context;             
            std::unique_ptr<ECS::RenderingSystem> m_renderingSystem;
            std::unique_ptr<ECS::TextRenderingSystem> m_textRenderingSystem;
            std::unique_ptr<ECS::MenuSystem> m_menuSystem;
            Renderer::TextureId m_bgTextureId = 0;
            Renderer::SpriteId m_bgSpriteId = 0;
            Renderer::FontId m_fontId = 0;            
            std::vector<ECS::Entity> m_entities;
        };

    }
}
