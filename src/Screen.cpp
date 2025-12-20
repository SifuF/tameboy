#include "Screen.hpp"

#include "Bus.hpp"
#include "Utils.hpp"

#include <SFML/Graphics.hpp>

#include <iostream>

Screen::Screen(Bus* bus) : m_bus(bus)
{
    constexpr int delta = 20;
    m_mainWindow.create(sf::VideoMode(sf::Vector2u(m_mainWidth, m_mainHeight)), "tameBOY");
    m_mainTexture.emplace(sf::Vector2u(m_mainWidth, m_mainHeight ));
    m_mainWindow.setSize(sf::Vector2u(m_mainScale * m_mainWidth, m_mainScale * m_mainHeight));
    const sf::Vector2i windowPosition{700, 0};
    m_mainWindow.setPosition(windowPosition);

    m_tileDataWindow.create(sf::VideoMode(sf::Vector2u(m_tileDataWidth, m_tileDataHeight)), "VRAM Tile Data");
    m_tileDataTexture.emplace(sf::Vector2u(m_tileDataWidth, m_tileDataHeight));
    m_tileDataWindow.setSize(sf::Vector2u(m_tileDataScale * m_tileDataWidth, m_tileDataScale * m_tileDataHeight));
    m_tileDataWindow.setPosition(windowPosition + sf::Vector2i{m_mainScale * m_mainWidth + delta, 0});

    m_tileMapWindow.create(sf::VideoMode(sf::Vector2u(m_tileMapWidth, m_tileMapHeight)), "VRAM Tile Maps");
    m_tileMapTexture.emplace(sf::Vector2u(m_tileMapWidth, m_tileMapHeight));
    m_tileMapWindow.setSize(sf::Vector2u(m_tileMapScale * m_tileMapWidth, m_tileMapScale * m_tileMapHeight));
    m_tileMapWindow.setPosition(windowPosition + sf::Vector2i{m_mainScale * m_mainWidth + m_tileDataScale * m_tileDataWidth + 2 * delta, 0});

    m_objectWindow.create(sf::VideoMode(sf::Vector2u(m_objectWidth, m_objectHeight)), "Objects");
    m_objectTexture.emplace(sf::Vector2u(m_objectWidth, m_objectHeight));
    m_objectWindow.setSize(sf::Vector2u(m_objectScale * m_objectWidth, m_objectScale * m_objectHeight));
    m_objectWindow.setPosition(windowPosition + sf::Vector2i{m_mainScale * m_mainWidth + m_tileDataScale * m_tileDataWidth + m_tileMapScale * m_tileMapWidth + 3 * delta, 0});
}

void Screen::update(const std::vector<uint8_t>& frameBuffer)
{
    if(!m_mainWindow.isOpen()) {
        m_running = false;
    }

    while (const std::optional event = m_mainWindow.pollEvent()) {
        if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() &&
            event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
            m_mainWindow.close();
        }

        const uint8_t currentJoypad = m_joypad;
        if (event->is<sf::Event::KeyPressed>()) {  // active low
            const auto code = event->getIf<sf::Event::KeyPressed>()->code;
            if (code == sf::Keyboard::Key::Down) { m_joypad = Utils::clearBit(m_joypad, 7); }
            if (code == sf::Keyboard::Key::Up) { m_joypad = Utils::clearBit(m_joypad, 6); }
            if (code == sf::Keyboard::Key::Left) { m_joypad = Utils::clearBit(m_joypad, 5); }
            if (code == sf::Keyboard::Key::Right) { m_joypad = Utils::clearBit(m_joypad, 4); }
            if ((code == sf::Keyboard::Key::S) || (code == sf::Keyboard::Key::Enter)) { m_joypad = Utils::clearBit(m_joypad, 3); }
            if (code == sf::Keyboard::Key::A) { m_joypad = Utils::clearBit(m_joypad, 2); }
            if (code == sf::Keyboard::Key::Z) { m_joypad = Utils::clearBit(m_joypad, 1); }
            if (code == sf::Keyboard::Key::X) { m_joypad = Utils::clearBit(m_joypad, 0); }
        }

        if (event->is<sf::Event::KeyReleased>()) {
            const auto code = event->getIf<sf::Event::KeyReleased>()->code;
            if (code == sf::Keyboard::Key::Down) { m_joypad = Utils::setBit(m_joypad, 7); }
            if (code == sf::Keyboard::Key::Up) { m_joypad = Utils::setBit(m_joypad, 6); }
            if (code == sf::Keyboard::Key::Left) { m_joypad = Utils::setBit(m_joypad, 5); }
            if (code == sf::Keyboard::Key::Right) { m_joypad = Utils::setBit(m_joypad, 4); }
            if ((code == sf::Keyboard::Key::S) || (code == sf::Keyboard::Key::Enter)) { m_joypad = Utils::setBit(m_joypad, 3); }
            if (code == sf::Keyboard::Key::A) { m_joypad = Utils::setBit(m_joypad, 2); }
            if (code == sf::Keyboard::Key::Z) { m_joypad = Utils::setBit(m_joypad, 1); }
            if (code == sf::Keyboard::Key::X) { m_joypad = Utils::setBit(m_joypad, 0); }
        }

        if (currentJoypad != m_joypad) {
            const auto newInterruptFlag = Utils::setBit(m_bus->read(0xFF0F), static_cast<int>(Interrupt::Joypad));
            m_bus->write(0xFF0F, newInterruptFlag);
        }
    }

    m_mainTexture->update(frameBuffer.data());
    m_mainSprite.emplace(m_mainTexture.value());
    m_mainWindow.clear();
    m_mainWindow.draw(m_mainSprite.value());
    m_mainWindow.display();
}

void Screen::updateDebug(const std::vector<uint8_t>& tileDataBuffer,
    const std::vector<uint8_t>& tileMapBuffer, const std::vector<uint8_t>& objectBuffer)
{
    while (const std::optional event = m_tileDataWindow.pollEvent()) {
        if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() &&
            event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
            m_tileDataWindow.close();
        }
    }

    while (const std::optional event = m_tileMapWindow.pollEvent()) {
        if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() &&
            event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
            m_tileMapWindow.close();
        }
    }

    while (const std::optional event = m_objectWindow.pollEvent()) {
        if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() &&
            event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
            m_objectWindow.close();
        }
    }

    m_tileDataTexture->update(tileDataBuffer.data());
    m_tileDataSprite.emplace(m_tileDataTexture.value());
    m_tileDataWindow.clear();
    m_tileDataWindow.draw(m_tileDataSprite.value());
    m_tileDataWindow.display();

    m_tileMapTexture->update(tileMapBuffer.data());
    m_tileMapSprite.emplace(m_tileMapTexture.value());
    m_tileMapWindow.clear();
    m_tileMapWindow.draw(m_tileMapSprite.value());
    m_tileMapWindow.display();

    m_objectTexture->update(objectBuffer.data());
    m_objectSprite.emplace(m_objectTexture.value());
    m_objectWindow.clear();
    m_objectWindow.draw(m_objectSprite.value());
    m_objectWindow.display();
}
