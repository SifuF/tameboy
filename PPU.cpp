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
/*    const auto LCDC = bus->read<uint8_t>(0xFF40);
    
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
*/
    // visible lcd area is 160x144 pixels or 20x18 tiles
    // scrollable over an area of 256x256 pixels or 32x32 tiles
    const auto colorLookup = [](const bool msb, const bool lsb) -> uint8_t {
        switch( (msb<<1) | lsb ) {
            case 0 : return 0;
            case 1 : return 80;
            case 2 : return 170;
            default : return 255;
        }
    };    

    auto drawTile = [this, colorLookup](const int x, const int y, const int tile) {
        const int screenStart = x*8*4 + y*8*4*160;
        const int tileData = 0x8000;
        const int tileStart = tileData + tile*16;

        for(int j=0; j<8; ++j) { // 8 rows in a tile
            const auto lsByte = bus->read<uint8_t>(tileStart + 2*j);
            const auto msByte = bus->read<uint8_t>(tileStart + 2*j + 1);
            for(int i=0; i<8; ++i) { // one 8 tile row at a time
                const auto lsBit = static_cast<bool>(lsByte >> (7-i));
                const auto msBit = static_cast<bool>(msByte >> (7-i));
                const auto color = colorLookup(msBit, lsBit);
                frameBuffer[screenStart + 4*i + j*160*4] = color;  
                frameBuffer[screenStart + 4*i + 1 + j*160*4] = color;
                frameBuffer[screenStart + 4*i + 2 + j*160*4] = color;
                frameBuffer[screenStart + 4*i + 3 + j*160*4] = 255;    
            }
        }
    };

    for(int j = 0; j<18; ++j) {
        for(int i=0; i<20; ++i) {
            drawTile(i, j, 0);
        }
    }
}
