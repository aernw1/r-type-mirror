#include "Menu/Button.hpp"

namespace RType {
    namespace Client {

        Button::Button(const sf::Vector2f& position, const sf::Vector2f& size, const std::string& text, const sf::Font& font)
            : m_state(State::Idle) {
            
            m_shape.setPosition(position);
            m_shape.setSize(size);
            
            m_idleColor = sf::Color(70, 70, 70);
            m_hoverColor = sf::Color(100, 100, 100);
            m_activeColor = sf::Color(50, 50, 50);

            m_shape.setFillColor(m_idleColor);
            m_shape.setOutlineThickness(2);
            m_shape.setOutlineColor(sf::Color::White);

            m_text.setFont(font);
            m_text.setString(text);
            m_text.setCharacterSize(24);
            m_text.setFillColor(sf::Color::White);
            
            sf::FloatRect textRect = m_text.getLocalBounds();
            m_text.setOrigin(textRect.left + textRect.width/2.0f,
                           textRect.top  + textRect.height/2.0f);
            m_text.setPosition(position.x + size.x/2.0f, position.y + size.y/2.0f);
        }

        void Button::SetCallback(std::function<void()> callback) {
            m_callback = callback;
        }

        void Button::Update(const sf::Vector2f& mousePos) {
            if (m_state == State::Active) return;

            if (m_shape.getGlobalBounds().contains(mousePos)) {
                m_state = State::Hover;
                m_shape.setFillColor(m_hoverColor);
            } else {
                m_state = State::Idle;
                m_shape.setFillColor(m_idleColor);
            }
        }

        void Button::HandleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
            if (m_shape.getGlobalBounds().contains(mousePos)) {
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    m_state = State::Active;
                    m_shape.setFillColor(m_activeColor);
                }
                else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                    if (m_state == State::Active) {
                        if (m_callback)
                            m_callback();
                        m_state = State::Hover;
                        m_shape.setFillColor(m_hoverColor);
                    }
                }
            } else {
                if (event.type == sf::Event::MouseButtonReleased) {
                     m_state = State::Idle;
                     m_shape.setFillColor(m_idleColor);
                }
            }
        }

        void Button::Draw(sf::RenderWindow& window) {
            window.draw(m_shape);
            window.draw(m_text);
        }

    }
}

