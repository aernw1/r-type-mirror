#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

namespace RType {
    namespace Client {

        class Button {
        public:
            Button(const sf::Vector2f& position, const sf::Vector2f& size, const std::string& text, const sf::Font& font);

            void SetCallback(std::function<void()> callback);
            void Update(const sf::Vector2f& mousePos);
            void Draw(sf::RenderWindow& window);
            void HandleEvent(const sf::Event& event, const sf::Vector2f& mousePos);

        private:
            sf::RectangleShape m_shape;
            sf::Text m_text;
            std::function<void()> m_callback;           
            sf::Color m_idleColor;
            sf::Color m_hoverColor;
            sf::Color m_activeColor;

            enum class State { Idle, Hover, Active };
            State m_state;
        };

    }
}

