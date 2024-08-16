#pragma once

#include <SFML/Graphics.hpp>

class Screen {
public:
    Screen();
    void update(const std::vector<uint8_t> & frameBuffer);

private:
    sf::RenderWindow window;
    sf::Texture texture;
    sf::Sprite sprite;
    bool running = true;

    static constexpr int width = 160;
    static constexpr int height = 144;
    int scale = 5;
};

