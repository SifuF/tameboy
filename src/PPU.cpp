#include "PPU.hpp"

#include "Bus.hpp"

#include <cstdlib>

PPU::PPU(Bus* bus) : bus(bus),
frameBuffer(160 * 144 * 4), // 160 x 144 x 4 bits RGBA. 
tileDataBuffer(3 * 128 * 8 * 8 * 4), // 3 blocks, 128 tiles in each block, each tile 8x8 pixels, each pixel 4 bits RGBA
tileMapBuffer(2 * 32 * 32 * 8 * 8 * 4) // 2 maps, 32x32 tiles in each map, each tile 8x8 pixels, each pixel 4 bits RGBA
{}

uint8_t PPU::colorLookup(const bool msb, const bool lsb) {
    switch ((msb << 1) | lsb) {
        case 0: return 0;
        case 1: return 80;
        case 2: return 170;
        default: return 255;
    }
};

auto PPU::drawTile(std::vector<uint8_t>& buffer, int width, const uint8_t x, const uint8_t y, const uint8_t tile, const bool mode) {
    const int screenStart = x * 8 * 4 + y * 8 * 4 * width;
    int tileData = 0x8000;
    if (!mode) {
        if (tile < 128) {
            tileData = 0x9000;
        }
        else {
            tileData = 0x8800;
        }
    }
    const int tileStart = tileData + tile * 16;

    for (int j = 0; j < 8; ++j) { // 8 rows in a tile
        const auto lsByte = bus->read<uint8_t>(tileStart + 2 * j);
        const auto msByte = bus->read<uint8_t>(tileStart + 2 * j + 1);
        for (int i = 0; i < 8; ++i) { // one 8 tile row at a time
            const auto lsBit = static_cast<bool>(lsByte & (1 << (7 - i)));
            const auto msBit = static_cast<bool>(msByte & (1 << (7 - i)));
            const auto color = colorLookup(msBit, lsBit);
            buffer[screenStart + 4 * i + j * width * 4] = color;
            buffer[screenStart + 4 * i + 1 + j * width * 4] = color;
            buffer[screenStart + 4 * i + 2 + j * width * 4] = color;
            buffer[screenStart + 4 * i + 3 + j * width * 4] = 255;
        }
    }
};

void PPU::updateVramDisplay() {
    // blocks 1 and 3
    for (int j = 0; j < 16; ++j) {
        for (int i = 0; i < 16; ++i) {
            drawTile(tileDataBuffer, 128, i, j, i + 16 * j, 1);
        }
    }
    // block 3
    for (int j = 16; j < 24; ++j) {
        for (int i = 0; i < 16; ++i) {
            drawTile(tileDataBuffer, 128, i, j, i + 16 * j, 0);
        }
    }

    // background
    for (int j = 0; j < 32; ++j) {
        for (int i = 0; i < 32; ++i) {
            const auto tile = bus->read<uint8_t>(0x9800 + i + 32 * j);
            drawTile(tileMapBuffer, 256, i, j, tile, 1);
        }
    }

    // window
    for (int j = 0; j < 32; ++j) {
        for (int i = 0; i < 32; ++i) {
            const auto tile = bus->read<uint8_t>(0x9C00 + i + 32 * j);
            drawTile(tileMapBuffer, 256, i, j + 32, tile, 0);
        }
    }
}

void PPU::tick() {
    updateVramDisplay();

    const auto LCDC = bus->read<uint8_t>(0xFF40);
    
    const auto enableLcdAndPpu = static_cast<bool>((LCDC & 0b10000000) >> 7); 
    if(!enableLcdAndPpu) {
        return;
    }
    
    // tiles are 8x8 pixels and 2 bits per pixel -> 16 bytes per tile
    const auto windowTileMap = static_cast<bool>((LCDC & 0b01000000) >> 6); // false:9800-9BFF, true:9C00-9FFF (32x32 = 1024 bytes)
    const auto enableWindow =  static_cast<bool>((LCDC & 0b00100000) >> 5);
    const auto tileDataArea = static_cast<bool>((LCDC & 0b00010000) >> 4); // false:8000-97FF, true:8800-8FFF (4096 bytes)
    const auto backgroundTileMap = static_cast<bool>((LCDC & 0b00001000) >> 3); // false:9800-9BFF, true:9C00-9FFF (32x32 = 1024 bytes)
    const auto objSize = static_cast<bool>((LCDC & 0b00000100) >> 2); // false:8x8, true8x16
    const auto enableObj = static_cast<bool>((LCDC & 0b00000010) >> 1);
    const auto enableBackgroundAndWindow = static_cast<bool>(LCDC & 0b00000001);

    const unsigned tileData = tileDataArea ? 0x8000 : 0x9000;
    const unsigned background = backgroundTileMap ? 0x9800 : 0x9C00;
    const unsigned window = windowTileMap ? 0x9800 : 0x9C00;
    
    // visible lcd area is 160x144 pixels or 20x18 tiles
    // scrollable over an area of 256x256 pixels or 32x32 tiles
    for(int j = 0; j<18; ++j) {
        for(int i=0; i<20; ++i) {
            const uint16_t addr = backgroundTileMap ? 0x9C00 : 0x9800;
            const auto tile = bus->read<uint8_t>(addr + i + 32 * j); // 32 width because we are sampling 32x32 tilemap
            drawTile(frameBuffer, 160, i, j, tile, tileDataArea);
        }
    }
}
