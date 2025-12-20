#pragma once

#include <SFML/Graphics.hpp>

#include <optional>

class Bus;

class Screen {
public:
    Screen(Bus* bus);
    void update(const std::vector<uint8_t>& frameBuffer);
    void updateDebug(const std::vector<uint8_t>& tileDataBuffer,
        const std::vector<uint8_t>& tileMapBuffer, const std::vector<uint8_t>& objectBuffer);

    uint8_t getJoypad() { return m_joypad; }

private:
    sf::RenderWindow m_mainWindow;
    std::optional<sf::Texture> m_mainTexture;
    std::optional<sf::Sprite> m_mainSprite;
    static constexpr int m_mainWidth = 160; // 20 * 8
    static constexpr int m_mainHeight = 144; // 18 * 8
    int m_mainScale = 5;
    
    sf::RenderWindow m_tileDataWindow;
    std::optional<sf::Texture> m_tileDataTexture;
    std::optional<sf::Sprite> m_tileDataSprite;
    static constexpr int m_tileDataWidth = 128; // 16 * 8
    static constexpr int m_tileDataHeight = 192; // 24 * 8
    int m_tileDataScale = 5;

    sf::RenderWindow m_tileMapWindow;
    std::optional<sf::Texture> m_tileMapTexture;
    std::optional<sf::Sprite> m_tileMapSprite;
    static constexpr int m_tileMapWidth = 256; // 32 * 8
    static constexpr int m_tileMapHeight = 512; // 2 * 32 * 8
    int m_tileMapScale = 2;

    sf::RenderWindow m_objectWindow;
    std::optional<sf::Texture> m_objectTexture;
    std::optional<sf::Sprite> m_objectSprite;
    static constexpr int m_objectWidth = 160; // 20 * 8
    static constexpr int m_objectHeight = 144; // 18 * 8
    int m_objectScale = 3;

    bool m_running = true;

    Bus* m_bus;
    uint8_t m_joypad{0xFF}; // down, up, left, right, start, select, b, a
};
