#include "PPU.hpp"

#include <cstdlib>

PPU::PPU() : frameBuffer(160*144*4) {
    for(int i=0; i<160*144*4; i+=4) {
        frameBuffer[i] = rand() % 256;
        frameBuffer[i+1] = rand() % 256;
        frameBuffer[i+2] = rand() % 256;
        frameBuffer[i+3] = 255;
    } 
}

void PPU::tick() {

}
