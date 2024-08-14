#include "PPU.hpp"

#include "Bus.hpp"

#include <cstdlib>

PPU::PPU(Bus* bus) : bus(bus), frameBuffer(160*144*4) {
   // for(int i=0; i<160*144*4; i+=4) {
   //     frameBuffer[i] = rand() % 256;
   //     frameBuffer[i+1] = rand() % 256;
   //     frameBuffer[i+2] = rand() % 256;
   //     frameBuffer[i+3] = 255;
   // }

    for(int i=0x8000; i<0x9800; i++) {
        bus->write<uint8_t>(i, rand() % 256);
    } 
}

void PPU::tick() {
    const auto LCDC = bus->read<uint8_t>(0xFF40);
    
    const auto enableLcdAndPpu = static_cast<bool>((LCDC & 0x10000000) >> 7); 
    if(!enableLcdAndPpu) {
        return;
    }
    
    // tiles are 8x8 pixels and 2 bits per pixel -> 16 bytes per tile
    const auto windowTileMap = static_cast<bool>((LCDC & 0x01000000) >> 6); // false:9800-9BFF, true:9C00-9FFF (32x32 = 1024 bytes)
    const auto enableWindow =  static_cast<bool>((LCDC & 0x00100000) >> 5);
    const auto tileDataArea = static_cast<bool>((LCDC & 0x00010000) >> 4); // false:8000-97FF, true:8800-8FFF (4096 bytes)
    const auto backgroundTileMap = static_cast<bool>((LCDC & 0x00001000) >> 3); // false:9800-9BFF, true:9C00-9FFF (32x32 = 1024 bytes)
    const auto objSize = static_cast<bool>((LCDC & 0x00000100) >> 2); // false:8x8, true8x16
    const auto enableObj = static_cast<bool>((LCDC & 0x00000010) >> 1);
    const auto enableBackgroundAndWindow = static_cast<bool>(LCDC & 0x00000001);

    const unsigned tileData = tileDataArea ? 0x8000 : 0x9000;
    const unsigned background = backgroundTileMap ? 0x9800 : 0x9C00;
    const unsigned window = windowTileMap ? 0x9800 : 0x9C00;

    for(int y = 0; y<144; ++y) {
        for(int x=0; x<160; ++x) {
            auto value = bus->read<uint8_t>(tileData + x);
            if(value != 0)
                value = 255;

            frameBuffer[4*x + 160*4*y] = value;
            frameBuffer[4*x + 1 + 160*4*y] = value;
            frameBuffer[4*x + 2 + 160*4*y] = value;
            frameBuffer[4*x + 3 + 160*4*y] = 255;
        }
    }
}
