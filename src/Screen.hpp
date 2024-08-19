#pragma once

#include <SFML/Graphics.hpp>

class Screen {
public:
    Screen();
    void update(const std::vector<uint8_t>& frameBuffer, const std::vector<uint8_t>& tileDataBuffer, const std::vector<uint8_t>& tileMapBuffer);

private:
    sf::RenderWindow mainWindow;
    sf::Texture mainTexture;
    sf::Sprite mainSprite;
    static constexpr int mainWidth = 160; // 20 * 8
    static constexpr int mainHeight = 144; // 18 * 8
    int mainScale = 5;
    
    sf::RenderWindow tileDataWindow;
    sf::Texture tileDataTexture;
    sf::Sprite tileDataSprite;
    static constexpr int tileDataWidth = 128; // 16 * 8
    static constexpr int tileDataHeight = 192; // 24 * 8
    int tileDataScale = 5;

    sf::RenderWindow tileMapWindow;
    sf::Texture tileMapTexture;
    sf::Sprite tileMapSprite;
    static constexpr int tileMapWidth = 256; // 32 * 8
    static constexpr int tileMapHeight = 512; // 2 * 32 * 8
    int tileMapScale = 2;

    bool running = true;
};

