#pragma once

#include <cstdint>
#include <vector>

class Bus;

class PPU {
public:
    PPU(Bus* bus);
    void tick();
    std::vector<uint8_t>& getFrameBuffer() { return frameBuffer; }
    std::vector<uint8_t>& getTileDataBuffer() { return tileDataBuffer; }
    std::vector<uint8_t>& getTileMapBuffer() { return tileMapBuffer; }

private:
    uint8_t colorLookup(const bool msb, const bool lsb);
    auto drawTile(std::vector<uint8_t>& buffer, int width, const uint8_t x, const uint8_t y, const uint8_t tile, const bool mode);
    void updateVramDisplay();

    std::vector<uint8_t> frameBuffer;
    std::vector<uint8_t> tileDataBuffer;
    std::vector<uint8_t> tileMapBuffer;
    Bus* bus = nullptr;
};
