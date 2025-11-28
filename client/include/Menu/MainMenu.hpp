#pragma once

#include "../GameStateMachine.hpp"
#include "Button.hpp"
#include <vector>

namespace RType {
    namespace Client {

        class MainMenu : public IState {
        public:
            MainMenu(GameStateMachine& machine, GameContext& context);
            ~MainMenu() override = default;

            void Init() override;
            void HandleInput() override;
            void Update(float dt) override;
            void Draw() override;

        private:
            GameStateMachine& m_machine;
            GameContext& m_context;           
            sf::Font m_font;
            sf::Text m_title;           
            sf::Texture m_bgTexture;
            sf::Sprite m_bgSprite;
            std::vector<std::unique_ptr<Button>> m_buttons;
        };

    }
}

