#include "PPU.hpp"

#include "Bus.hpp"

#include <cstdlib>

PPU::PPU(Bus* bus) : bus(bus),
frameBuffer(160 * 144 * 4), // 160 x 144 x 4 bytes RGBA
tileDataBuffer(3 * 128 * 8 * 8 * 4), // 3 blocks, 128 tiles in each block, each tile 8x8 pixels, each pixel 4 bytes RGBA
tileMapBuffer(2 * 32 * 32 * 8 * 8 * 4), // 2 maps, 32x32 tiles in each map, each tile 8x8 pixels, each pixel 4 bytes RGBA
m_dots(0),
m_mode(Mode::OAMSCAN),
m_currentLine(0),
m_dotsDrawn(0)
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

void PPU::drawLine() {
    const auto LCDC = bus->read<uint8_t>(0xFF40);

    const auto enableLcdAndPpu = static_cast<bool>((LCDC & 0b10000000) >> 7);
    if (!enableLcdAndPpu) {
        return;
    }

    // tiles are 8x8 pixels and 2 bits per pixel -> 16 bytes per tile
    const auto windowTileMapAddr = static_cast<bool>((LCDC & 0b01000000) >> 6) ? 0x9800 : 0x9C00; // 9800-9BFF : 9C00-9FFF (32x32 = 1024 bytes)
    const auto enableWindow = static_cast<bool>((LCDC & 0b00100000) >> 5);
    const auto tileDataArea = static_cast<bool>((LCDC & 0b00010000) >> 4) ? 0x8000 : 0x9000; // 8000-97FF : 8800-8FFF (4096 bytes)
    const auto backgroundTileMapAddr = static_cast<bool>((LCDC & 0b00001000) >> 3) ? 0x9800 : 0x9C00; // 9800-9BFF : 9C00-9FFF (32x32 = 1024 bytes)
    const auto objSize = static_cast<bool>((LCDC & 0b00000100) >> 2); // false:8x8, true8x16
    const auto enableObj = static_cast<bool>((LCDC & 0b00000010) >> 1);
    const auto enableBackgroundAndWindow = static_cast<bool>(LCDC & 0b00000001);
   
    int tileData = 0x8000;
    const auto mode = 0;
    //if (!mode) {
    //    if (tile < 128) {
            tileData = 0x9000;
    //    }
    //    else {
           tileData = tileDataArea;
    //    }
    //}

    for (int i = 0; i < 20; ++i) {  // 20 tile slices per row
        constexpr auto bytesPerLine = 20 * 8 * 2; // 8 pixels, 2 bytes per pixel
        const auto lsByte = bus->read<uint8_t>(tileData + 2*i + i*bytesPerLine + 16*m_currentLine);
        const auto msByte = bus->read<uint8_t>(tileData + 2*i + i*bytesPerLine + 16*m_currentLine + 1);
        for (int i = 0; i < 8; ++i) { // one 8 tile row at a time
            const auto lsBit = static_cast<bool>(lsByte & (1 << (7 - i)));
            const auto msBit = static_cast<bool>(msByte & (1 << (7 - i)));
            const auto color = colorLookup(msBit, lsBit);
            const auto width = 160; // TODO
            frameBuffer[4 * i + m_currentLine * width * 4] = color;
            frameBuffer[4 * i + 1 + m_currentLine * width * 4] = color;
            frameBuffer[4 * i + 2 + m_currentLine * width * 4] = color;
            frameBuffer[4 * i + 3 + m_currentLine * width * 4] = 255;
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

void PPU::horizontalInterrupt() {

}

void PPU::verticalInterrupt() {
}

void PPU::drawDots() {
    const auto LCDC = bus->read<uint8_t>(0xFF40);

    const auto enableLcdAndPpu = static_cast<bool>((LCDC & 0b10000000) >> 7);
    if (!enableLcdAndPpu) {
        return;
    }

    // tiles are 8x8 pixels and 2 bits per pixel -> 16 bytes per tile
    const auto windowTileMapAddr = static_cast<bool>((LCDC & 0b01000000) >> 6) ? 0x9800 : 0x9C00; // 9800-9BFF : 9C00-9FFF (32x32 = 1024 bytes)
    const auto enableWindow = static_cast<bool>((LCDC & 0b00100000) >> 5);
    const auto tileDataArea = static_cast<bool>((LCDC & 0b00010000) >> 4) ? 0x8000 : 0x9000; // 8000-97FF : 8800-8FFF (4096 bytes)
    const auto backgroundTileMapAddr = static_cast<bool>((LCDC & 0b00001000) >> 3) ? 0x9800 : 0x9C00; // 9800-9BFF : 9C00-9FFF (32x32 = 1024 bytes)
    const auto objSize = static_cast<bool>((LCDC & 0b00000100) >> 2); // false:8x8, true8x16
    const auto enableObj = static_cast<bool>((LCDC & 0b00000010) >> 1);
    const auto enableBackgroundAndWindow = static_cast<bool>(LCDC & 0b00000001);

    // visible lcd area is 160x144 pixels or 20x18 tiles
    // scrollable over an area of 256x256 pixels or 32x32 tiles
    const uint32_t dotsToDraw = m_dots - m_dotsDrawn;
    m_dotsDrawn += dotsToDraw;
}

void PPU::tick(uint32_t cycles) {
    m_dots += cycles;

    // FRAME total = 456 * 154 = 70224 dots
    //        OAM         DRAW            HBLANK          LINETOTAL       
    // 0      80 dots     172-289 dots    87-204 dots     456 dots
    // 1      80 dots     172-289 dots    87-204 dots     456 dots
    // ...    80 dots     172-289 dots    87-204 dots     456 dots
    // 143    80 dots     172-289 dots    87-204 dots     456 dots
    // 144    VBLANK                                      456 dots
    // ...    VBLANK                                      456 dots
    // 153    VBLANK                                      456 dots

    const uint32_t oamLength = 80;
    const uint32_t drawLength = 172;
    const uint32_t lineLength = 456;
    const uint32_t screenHeight = 144;
    const uint32_t vblankLines = 10;
    
    if (m_mode == Mode::OAMSCAN) {
        if (m_dots >= oamLength) {
            m_mode = Mode::DRAW;
        }
    }
    else if (m_mode == Mode::DRAW) {
        // TODO - instead of drawLine()
        // drawDots(); 
        if (m_dots >= oamLength + drawLength) {
            horizontalInterrupt();
            //drawLine();
            m_mode = Mode::HBLANK;
        }
    }
    else if (m_dots >= lineLength) {
        m_dots -= lineLength;
        m_currentLine++;
        if (m_currentLine < screenHeight) {
            m_mode = Mode::OAMSCAN;
        }
        else if (m_mode == Mode::HBLANK) {
            verticalInterrupt();
            m_mode = Mode::VBLANK;
        }
        else if (m_currentLine >= screenHeight + vblankLines) {
            m_currentLine = 0;
            m_mode = Mode::OAMSCAN;
        }
    }

    // HACK to advance the screen in tetris
    static int line = 0;
    bus->write<uint8_t>(0xFF44, line++);
    if (line > 20000) {
        line = 0;
    }

}
