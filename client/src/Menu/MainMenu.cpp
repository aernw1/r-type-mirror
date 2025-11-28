#include "Menu/MainMenu.hpp"
#include <iostream>

namespace RType {
    namespace Client {

        MainMenu::MainMenu(GameStateMachine& machine, GameContext& context)
            : m_machine(machine), m_context(context) {
        }

        void MainMenu::Init() {
            if (!m_font.loadFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
                std::cerr << "Failed to load assets/fonts/PressStart2P-Regular.ttf" << std::endl;
                if (!m_font.loadFromFile("../assets/fonts/PressStart2P-Regular.ttf")) {
                     std::cerr << "Failed to load font from fallback path too!" << std::endl;
                }
            }

            if (m_bgTexture.loadFromFile("assets/backgrounds/1.jpg")) {
                m_bgSprite.setTexture(m_bgTexture);
                
                sf::Vector2u texSize = m_bgTexture.getSize();
                sf::Vector2u winSize = m_context.window->getSize();
                float scaleX = (float)winSize.x / texSize.x;
                float scaleY = (float)winSize.y / texSize.y;
                m_bgSprite.setScale(scaleX, scaleY);
            } else {
                 std::cerr << "Failed to load background assets/backgrounds/1.jpg" << std::endl;
            }
//je vais les remplacer par des sprites plus tard tkt
            m_title.setFont(m_font);
            m_title.setString("R-TYPE");
            m_title.setCharacterSize(60);
            m_title.setFillColor(sf::Color(0, 255, 255));
            m_title.setOutlineColor(sf::Color::Blue);
            m_title.setOutlineThickness(2);
            
            sf::FloatRect textRect = m_title.getLocalBounds();
            m_title.setOrigin(textRect.left + textRect.width/2.0f,
                            textRect.top  + textRect.height/2.0f);
            m_title.setPosition(m_context.window->getSize().x / 2.0f, 100.0f);

            float centerX = m_context.window->getSize().x / 2.0f;
            
            auto playBtn = std::make_unique<Button>(
                sf::Vector2f(centerX - 125, 300),
                sf::Vector2f(250, 60),
                "PLAY",
                m_font
            );
            playBtn->SetCallback([this]() {
                std::cout << "Play button clicked!" << std::endl;
            });
            m_buttons.push_back(std::move(playBtn));

            auto exitBtn = std::make_unique<Button>(
                sf::Vector2f(centerX - 125, 400),
                sf::Vector2f(250, 60),
                "EXIT",
                m_font
            );
            exitBtn->SetCallback([this]() {
                m_context.window->close();
            });
            m_buttons.push_back(std::move(exitBtn));
        }

        void MainMenu::HandleInput() {
            sf::Event event;
            sf::Vector2f mousePos = m_context.window->mapPixelToCoords(sf::Mouse::getPosition(*m_context.window));

            while (m_context.window->pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    m_context.window->close();
                    return;
                }
                
                if (event.type == sf::Event::Resized) {
                    sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                    m_context.window->setView(sf::View(visibleArea));
                    
                    if (m_bgTexture.getSize().x > 0) {
                        float scaleX = (float)event.size.width / m_bgTexture.getSize().x;
                        float scaleY = (float)event.size.height / m_bgTexture.getSize().y;
                        m_bgSprite.setScale(scaleX, scaleY);
                    }
                }

                for (auto& btn : m_buttons) {
                    btn->HandleEvent(event, mousePos);
                }
            }
        }

        void MainMenu::Update(float dt) {
            sf::Vector2f mousePos = m_context.window->mapPixelToCoords(sf::Mouse::getPosition(*m_context.window));
            
            for (auto& btn : m_buttons) {
                btn->Update(mousePos);
            }
        }

        void MainMenu::Draw() {
            m_context.window->clear(sf::Color::Black);           
            m_context.window->draw(m_bgSprite);           
            m_context.window->draw(m_title);
            
            for (auto& btn : m_buttons) {
                btn->Draw(*m_context.window);
            }
            m_context.window->display();
        }

    }
}
