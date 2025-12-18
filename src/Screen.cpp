#include "Screen.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>

Screen::Screen() {
    constexpr int delta = 20;
    mainWindow.create(sf::VideoMode(sf::Vector2u(mainWidth, mainHeight)), "tameBOY");
    mainTexture.emplace(sf::Vector2u(mainWidth, mainHeight ));
    mainWindow.setSize(sf::Vector2u(mainScale* mainWidth, mainScale* mainHeight));
    const sf::Vector2i windowPosition{700, 0};
    mainWindow.setPosition(windowPosition);

    tileDataWindow.create(sf::VideoMode(sf::Vector2u(tileDataWidth, tileDataHeight)), "VRAM Tile Data");
    tileDataTexture.emplace(sf::Vector2u(tileDataWidth, tileDataHeight));
    tileDataWindow.setSize(sf::Vector2u(tileDataScale * tileDataWidth, tileDataScale * tileDataHeight));
    tileDataWindow.setPosition(windowPosition + sf::Vector2i{mainScale * mainWidth + delta, 0});

    tileMapWindow.create(sf::VideoMode(sf::Vector2u(tileMapWidth, tileMapHeight)), "VRAM Tile Maps");
    tileMapTexture.emplace(sf::Vector2u(tileMapWidth, tileMapHeight));
    tileMapWindow.setSize(sf::Vector2u(tileMapScale * tileMapWidth, tileMapScale * tileMapHeight));
    tileMapWindow.setPosition(windowPosition + sf::Vector2i{mainScale * mainWidth + tileDataScale * tileDataWidth + 2 * delta, 0});
}

void Screen::update(const std::vector<uint8_t>& frameBuffer) {
    if(!mainWindow.isOpen()) {
        running = false;
    }

    while (const std::optional event = mainWindow.pollEvent()) {
        if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() &&
            event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
            mainWindow.close();
        }

        if (event->is<sf::Event::KeyPressed>()) {
            const auto code = event->getIf<sf::Event::KeyPressed>()->code;
            if (code == sf::Keyboard::Key::Up) { m_joypad.at(0) = true; }
            if (code == sf::Keyboard::Key::Down) { m_joypad.at(1) = true; }
            if (code == sf::Keyboard::Key::Left) { m_joypad.at(2) = true; }
            if (code == sf::Keyboard::Key::Right) { m_joypad.at(3) = true; }
            if (code == sf::Keyboard::Key::Z) { m_joypad.at(4) = true; }
            if (code == sf::Keyboard::Key::X) { m_joypad.at(5) = true; }
            if (code == sf::Keyboard::Key::A) { m_joypad.at(6) = true; }
            if (code == sf::Keyboard::Key::S) { m_joypad.at(7) = true; }
        }

        if (event->is<sf::Event::KeyReleased>()) {
            const auto code = event->getIf<sf::Event::KeyPressed>()->code;
            if (code == sf::Keyboard::Key::Up) { m_joypad.at(0) = false; }
            if (code == sf::Keyboard::Key::Down) { m_joypad.at(1) = false; }
            if (code == sf::Keyboard::Key::Left) { m_joypad.at(2) = false; }
            if (code == sf::Keyboard::Key::Right) { m_joypad.at(3) = false; }
            if (code == sf::Keyboard::Key::Z) { m_joypad.at(4) = false; }
            if (code == sf::Keyboard::Key::X) { m_joypad.at(5) = false; }
            if (code == sf::Keyboard::Key::A) { m_joypad.at(6) = false; }
            if (code == sf::Keyboard::Key::S) { m_joypad.at(7) = false; }
        }
    }

    mainTexture->update(frameBuffer.data());
    mainSprite.emplace(mainTexture.value());
    mainWindow.clear();
    mainWindow.draw(mainSprite.value());
    mainWindow.display();
}

void Screen::updateDebug(const std::vector<uint8_t>& tileDataBuffer, const std::vector<uint8_t>& tileMapBuffer) {
    while (const std::optional event = tileDataWindow.pollEvent()) {
        if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() &&
            event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
            tileDataWindow.close();
        }
    }

    while (const std::optional event = tileMapWindow.pollEvent()) {
        if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() &&
            event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
            tileMapWindow.close();
        }
    }

    tileDataTexture->update(tileDataBuffer.data());
    tileDataSprite.emplace(tileDataTexture.value());
    tileDataWindow.clear();
    tileDataWindow.draw(tileDataSprite.value());
    tileDataWindow.display();

    tileMapTexture->update(tileMapBuffer.data());
    tileMapSprite.emplace(tileMapTexture.value());
    tileMapWindow.clear();
    tileMapWindow.draw(tileMapSprite.value());
    tileMapWindow.display();
}
