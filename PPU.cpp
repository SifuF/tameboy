#include "PPU.hpp"

#include "Bus.hpp"

#include <cstdlib>

PPU::PPU(Bus* bus) : bus(bus), frameBuffer(160*144*4) {
    for(int i=0; i<160*144*4; i+=4) {
        frameBuffer[i] = rand() % 256;
        frameBuffer[i+1] = rand() % 256;
        frameBuffer[i+2] = rand() % 256;
        frameBuffer[i+3] = 255;
    } 
}

void PPU::tick() {
    for(int i = 0; i<160*144; i++) {
        const auto value = bus->read<uint8_t>(0x8000 + i);
        frameBuffer[4*i] = value;
        frameBuffer[4*i + 1] = value;
        frameBuffer[4*i + 2] = value;
        frameBuffer[4*i + 3] = 255;
    }
}
