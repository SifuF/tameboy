#pragma once

#include <cstdint>
#include <vector>

class PPU {
public:
    PPU();
    void tick();
    std::vector<uint8_t> & getFrameBuffer() { return frameBuffer; }

private:
    std::vector<uint8_t> frameBuffer;
};
