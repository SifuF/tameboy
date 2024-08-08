#pragma once

#include <cstdint>
#include <vector>

class Bus;

class PPU {
public:
    PPU(Bus* bus);
    void tick();
    std::vector<uint8_t> & getFrameBuffer() { return frameBuffer; }

private:
    std::vector<uint8_t> frameBuffer;
    Bus* bus = nullptr;
};
