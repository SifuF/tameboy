#include "Screen.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>

Screen::Screen() {
    mainWindow.create(sf::VideoMode(mainWidth, mainHeight), "tameBOY");
    mainTexture.create(mainWidth, mainHeight);
    mainWindow.setSize(sf::Vector2u(mainScale* mainWidth, mainScale* mainHeight));
    sf::Vector2i windowPosition(2000, 1000);
    mainWindow.setPosition(windowPosition);

    debugWindow.create(sf::VideoMode(debugWidth, debugHeight), "VRAM");
    debugTexture.create(debugWidth, debugHeight);
    debugWindow.setSize(sf::Vector2u(debugScale * debugWidth, debugScale * debugHeight));
    debugWindow.setPosition(windowPosition + sf::Vector2i{170 * mainScale, 0});
}

void Screen::update(const std::vector<uint8_t> & frameBuffer, const std::vector<uint8_t>& vramDisplayBuffer) {
    if(!mainWindow.isOpen()) {
        running = false;
    }

    sf::Event mainEvent;
    while (mainWindow.pollEvent(mainEvent))
    {
        if (mainEvent.type == sf::Event::Closed)
            mainWindow.close();
    }

    sf::Event debugEvent;
    while (debugWindow.pollEvent(debugEvent))
    {
        if (debugEvent.type == sf::Event::Closed)
            debugWindow.close();
    }

    mainTexture.update(frameBuffer.data());
    mainSprite.setTexture(mainTexture);
    mainWindow.clear();
    mainWindow.draw(mainSprite);
    mainWindow.display();

    debugTexture.update(vramDisplayBuffer.data());
    debugSprite.setTexture(debugTexture);
    debugWindow.clear();
    debugWindow.draw(debugSprite);
    debugWindow.display();
}

