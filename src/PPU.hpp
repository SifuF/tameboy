#pragma once

#include <cstdint>
#include <vector>

class Bus;

class PPU {
public:
    PPU(Bus* bus);
    void tick();
    std::vector<uint8_t>& getFrameBuffer() { return frameBuffer; }
    std::vector<uint8_t>& getVramDisplayBuffer() { return vramDisplayBuffer; }

private:
    uint8_t colorLookup(const bool msb, const bool lsb);
    auto drawTile(std::vector<uint8_t>& buffer, int width, const uint8_t x, const uint8_t y, const uint8_t tile, const bool mode);
    void updateVramDisplay();

    std::vector<uint8_t> frameBuffer;
    std::vector<uint8_t> vramDisplayBuffer;
    Bus* bus = nullptr;
};
