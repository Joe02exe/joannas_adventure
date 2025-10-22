#include <SFML/Graphics.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <iostream>
#include <spdlog/spdlog.h>

int main()
{
    spdlog::info("Game started!");
    sf::RenderWindow window(sf::VideoMode({1920u, 1080u}), "SFML 3.0 Discrete Movement");
    window.setFramerateLimit(144);

    // Load texture
    sf::Texture texture;
    if (!texture.loadFromFile("assets/player.png"))
    {
        std::cerr << "Error: Could not load texture 'assets/player.png'\n";
        return 1;
    }

    // Sprite setup
    sf::Sprite player(texture);
    player.setPosition({960.f, 540.f});
    player.setScale({0.5f, 0.5f});

    const float step = 32.f; // pixels per move

    while (window.isOpen())
    {
        // --- Event loop ---
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            // Handle key presses (discrete — reacts once per keypress)
            if (auto keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                auto key = keyPressed->code;
                sf::Vector2f moveDelta{0.f, 0.f};

                if (key == sf::Keyboard::Key::W) moveDelta.y -= step;
                if (key == sf::Keyboard::Key::S) moveDelta.y += step;
                if (key == sf::Keyboard::Key::A) moveDelta.x -= step;
                if (key == sf::Keyboard::Key::D) moveDelta.x += step;

                player.move(moveDelta);
            }
        }

        // --- Render ---
        window.clear(sf::Color::Black);
        window.draw(player);
        window.display();
    }

    return 0;
}
