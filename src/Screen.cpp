#include "Screen.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>

Screen::Screen() {
    window.create(sf::VideoMode(width, height), "tameBOY");
    texture.create(width, height);
    window.setSize(sf::Vector2u(scale*width, scale*height));
}

void Screen::update(const std::vector<uint8_t> & frameBuffer) {
    if(!window.isOpen()) {
        running = false;
    }

    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window.close();
    }

    texture.update(frameBuffer.data());
    sprite.setTexture(texture);

    window.clear();
    window.draw(sprite);
    window.display();
}

