#include "Screen.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>

Screen::Screen() {
    constexpr int delta = 20;
    mainWindow.create(sf::VideoMode(mainWidth, mainHeight), "tameBOY");
    mainTexture.create(mainWidth, mainHeight);
    mainWindow.setSize(sf::Vector2u(mainScale* mainWidth, mainScale* mainHeight));
    sf::Vector2i windowPosition(700, 400);
    mainWindow.setPosition(windowPosition);

    tileDataWindow.create(sf::VideoMode(tileDataWidth, tileDataHeight), "VRAM Tile Data");
    tileDataTexture.create(tileDataWidth, tileDataHeight);
    tileDataWindow.setSize(sf::Vector2u(tileDataScale * tileDataWidth, tileDataScale * tileDataHeight));
    tileDataWindow.setPosition(windowPosition + sf::Vector2i{mainScale * mainWidth + delta, 0});

    tileMapWindow.create(sf::VideoMode(tileMapWidth, tileMapHeight), "VRAM Tile Maps");
    tileMapTexture.create(tileMapWidth, tileMapHeight);
    tileMapWindow.setSize(sf::Vector2u(tileMapScale * tileMapWidth, tileMapScale * tileMapHeight));
    tileMapWindow.setPosition(windowPosition + sf::Vector2i{mainScale * mainWidth + tileDataScale * tileDataWidth + 2 * delta, 0});
}

void Screen::update(const std::vector<uint8_t> & frameBuffer, const std::vector<uint8_t>& tileDataBuffer, const std::vector<uint8_t>& tileMapBuffer) {
    if(!mainWindow.isOpen()) {
        running = false;
    }

    sf::Event mainEvent;
    while (mainWindow.pollEvent(mainEvent))
    {
        if (mainEvent.type == sf::Event::Closed)
            mainWindow.close();
    }

    sf::Event tileDataEvent;
    while (tileDataWindow.pollEvent(tileDataEvent))
    {
        if (tileDataEvent.type == sf::Event::Closed)
            tileDataWindow.close();
    }

    sf::Event tileMapEvent;
    while (tileMapWindow.pollEvent(tileMapEvent))
    {
        if (tileMapEvent.type == sf::Event::Closed)
            tileMapWindow.close();
    }

    mainTexture.update(frameBuffer.data());
    mainSprite.setTexture(mainTexture);
    mainWindow.clear();
    mainWindow.draw(mainSprite);
    mainWindow.display();

    tileDataTexture.update(tileDataBuffer.data());
    tileDataSprite.setTexture(tileDataTexture);
    tileDataWindow.clear();
    tileDataWindow.draw(tileDataSprite);
    tileDataWindow.display();

    tileMapTexture.update(tileMapBuffer.data());
    tileMapSprite.setTexture(tileMapTexture);
    tileMapWindow.clear();
    tileMapWindow.draw(tileMapSprite);
    tileMapWindow.display();
}

