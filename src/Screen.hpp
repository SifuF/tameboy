#pragma once

#include <SFML/Graphics.hpp>

#include <optional>

class Screen {
public:
    Screen();
    void update(const std::vector<uint8_t>& frameBuffer);
    void updateDebug(const std::vector<uint8_t>& tileDataBuffer,
        const std::vector<uint8_t>& tileMapBuffer, const std::vector<uint8_t>& objectBuffer);

    uint8_t getJoypad() { return m_joypad; }

private:
    sf::RenderWindow mainWindow;
    std::optional<sf::Texture> mainTexture;
    std::optional<sf::Sprite> mainSprite;
    static constexpr int mainWidth = 160; // 20 * 8
    static constexpr int mainHeight = 144; // 18 * 8
    int mainScale = 5;
    
    sf::RenderWindow tileDataWindow;
    std::optional<sf::Texture> tileDataTexture;
    std::optional<sf::Sprite> tileDataSprite;
    static constexpr int tileDataWidth = 128; // 16 * 8
    static constexpr int tileDataHeight = 192; // 24 * 8
    int tileDataScale = 5;

    sf::RenderWindow tileMapWindow;
    std::optional<sf::Texture> tileMapTexture;
    std::optional<sf::Sprite> tileMapSprite;
    static constexpr int tileMapWidth = 256; // 32 * 8
    static constexpr int tileMapHeight = 512; // 2 * 32 * 8
    int tileMapScale = 2;

    sf::RenderWindow objectWindow;
    std::optional<sf::Texture> objectTexture;
    std::optional<sf::Sprite> objectSprite;
    static constexpr int objectWidth = 160; // 20 * 8
    static constexpr int objectHeight = 144; // 18 * 8
    int objectScale = 3;

    bool running = true;

    uint8_t m_joypad{0xFF}; // down, up, left, right, start, select, b, a
};

