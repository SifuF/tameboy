#pragma once

#include <SFML/Graphics.hpp>

class Screen {
public:
    Screen();
    void update(const std::vector<uint8_t>& frameBuffer, const std::vector<uint8_t>& vramDisplayBuffer);

private:
    sf::RenderWindow mainWindow;
    sf::Texture mainTexture;
    sf::Sprite mainSprite;
    static constexpr int mainWidth = 160;
    static constexpr int mainHeight = 144;
    int mainScale = 5;
    
    sf::RenderWindow debugWindow;
    sf::Texture debugTexture;
    sf::Sprite debugSprite;
    static constexpr int debugWidth = 128;
    static constexpr int debugHeight = 192;
    int debugScale = 5;

    bool running = true;
};

