#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

int main()
{
    std::cout << "=== SFML Proof of Concept ===" << std::endl;

    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML PoC - R-Type");
    window.setFramerateLimit(60);

    sf::Texture texture;
    if (!texture.loadFromFile("assets/ship.png")) {
        std::cout << "Warning: Could not load ship.png, using colored rectangle" << std::endl;
    }
    sf::Sprite sprite(texture);
    sprite.setPosition(400, 300);

    sf::RectangleShape shape(sf::Vector2f(50, 50));
    shape.setFillColor(sf::Color::Green);
    shape.setPosition(400, 300);

    sf::SoundBuffer buffer;
    sf::Sound sound;
    if (buffer.loadFromFile("assets/shoot.wav")) {
        sound.setBuffer(buffer);
    }

    std::cout << "Window created successfully" << std::endl;
    std::cout << "Controls: Arrow keys to move, Space to play sound, ESC to quit" << std::endl;

    sf::Clock clock;
    float speed = 200.f;

    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                if (event.key.code == sf::Keyboard::Space && buffer.getSampleCount() > 0)
                    sound.play();
            }
        }

        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            movement.x -= speed * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            movement.x += speed * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            movement.y -= speed * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            movement.y += speed * deltaTime;

        sprite.move(movement);
        shape.move(movement);

        window.clear(sf::Color::Black);

        if (texture.getSize().x > 0)
            window.draw(sprite);
        else
            window.draw(shape);

        window.display();
    }

    std::cout << "SFML PoC completed successfully" << std::endl;
    return 0;
}

