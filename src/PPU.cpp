#include "PPU.hpp"

#include "Bus.hpp"
#include "Utils.hpp"

#include <cstdlib>

PPU::PPU(Bus* bus) : bus(bus), m_dots(0), m_mode(Mode::OAMSCAN), m_currentLine(0), m_dotsDrawn(0)
{
    frameBuffer.data.resize(160 * 144 * 4); // 160 x 144 x 4 bytes RGBA
    frameBuffer.width = 160;

    tileDataBuffer.data.resize(3 * 128 * 8 * 8 * 4); // 3 blocks, 128 tiles in each block, each tile 8x8 pixels, each pixel 4 bytes RGBA
    tileDataBuffer.width = 128;

    tileMapBuffer.data.resize(2 * 32 * 32 * 8 * 8 * 4); // 2 maps, 32x32 tiles in each map, each tile 8x8 pixels, each pixel 4 bytes RGBA
    tileMapBuffer.width = 256;
}

std::array<uint8_t, 3> PPU::colorLookup(bool msb, bool lsb) const
{
    switch ((msb << 1) | lsb)
    {
#ifdef GREEN
        case 0: return { 0x0F, 0x38, 0x0F };
        case 1: return { 0x30, 0x62, 0x30 };
        case 2: return { 0x8B, 0xAC, 0x0F };
        default: return { 0x9B, 0xBC, 0x0F };
#else
        case 0: return { 255, 255, 255 };
        case 1: return { 170, 170, 170 };
        case 2: return { 80, 80, 80 };
        default: return { 0, 0, 0 };
#endif
    }
};

void PPU::drawAlignedTile(Vbuffer& buffer, XY tilePos, uint16_t tile, bool unsignedMode)
{
    const int tileData = unsignedMode ? 0x8000 : (tile < 128 ? 0x9000 : 0x8800);
    const int tileStart = tileData + tile * 16;
    const auto screenStart = tilePos.first * 8 + tilePos.second * 8 * buffer.width;
    for (int j = 0; j < 8; ++j) { // 8 rows in a tile
        const auto lsByte = bus->read<uint8_t>(tileStart + 2 * j);
        const auto msByte = bus->read<uint8_t>(tileStart + 2 * j + 1);
        for (int i = 0; i < 8; ++i) { // one 8 tile row at a time
            const auto lsBit = static_cast<bool>(lsByte & (1 << (7 - i)));
            const auto msBit = static_cast<bool>(msByte & (1 << (7 - i)));
            const auto [r, g, b] = colorLookup(msBit, lsBit);
            const int index = screenStart + i + j * buffer.width;
            buffer.data[4 * index] = r;
            buffer.data[4 * index + 1] = g;
            buffer.data[4 * index + 2] = b;
            buffer.data[4 * index + 3] = 255;
        }
    }
};

void PPU::drawLine(uint8_t LCDC, uint8_t SCX, uint8_t SCY, int LC) {
    const auto tileDataArea = static_cast<bool>((LCDC & 0b00010000) >> 4) ? 0x8000 : 0x9000; // 8000-97FF : 8800-8FFF (4096 bytes)
    const auto backgroundTileMapAddr = static_cast<bool>((LCDC & 0b00001000) >> 3) ? 0x9C00 : 0x9800; // 9800-9BFF : 9C00-9FFF (32x32 = 1024 bytes)
    for (int tileSlice = 0; tileSlice < 20; ++tileSlice) {   
        for (int pixel = 0; pixel < 8; ++pixel) { // one 8 tile row at a time
            const auto xTile = ( (SCX + 8 * tileSlice + pixel) % 160 ) / 8;
            const auto yTile = ( (SCY + LC) % 256 ) / 8;
            const auto tileNumber = bus->read<uint8_t>(backgroundTileMapAddr + xTile + 32 * yTile);

            const auto lsByteIndex = tileDataArea + 16 * tileNumber + 2 * ((SCY + LC) % 8);
            const auto lsByte = bus->read<uint8_t>(lsByteIndex);
            const auto msByte = bus->read<uint8_t>(lsByteIndex + 1);
    
            const auto lsBit = static_cast<bool>(lsByte & (1 << (7 - pixel)));
            const auto msBit = static_cast<bool>(msByte & (1 << (7 - pixel)));
            const auto [r, g, b] = colorLookup(msBit, lsBit);

            const int index = pixel + tileSlice * 8 + LC * frameBuffer.width;
            frameBuffer.data[4 * index] = r;
            frameBuffer.data[4 * index + 1] = g;
            frameBuffer.data[4 * index + 2] = b;
            frameBuffer.data[4 * index + 3] = 255;
        }
    }
};

void PPU::updateVramDisplay()
{
     // tile blocks
    constexpr int tileBlockWidth = 16;
    constexpr int tileBlockHeight = 8;
    for (int j = 0; j < tileBlockHeight; ++j) {
        for (int i = 0; i < tileBlockWidth; ++i) {
            drawAlignedTile(tileDataBuffer, { i, j }, i + tileBlockWidth * j); // block 0
            drawAlignedTile(tileDataBuffer, { i, j + tileBlockHeight }, i + j + tileBlockHeight * tileBlockWidth); // block 1
            drawAlignedTile(tileDataBuffer, { i, j + 2 * tileBlockHeight }, i + j + 2 * tileBlockHeight * tileBlockWidth); // block 2
        }
    }

    // tilemaps
    constexpr int tileMapkWidth = 32;
    for (int j = 0; j < tileMapkWidth; ++j) {
        for (int i = 0; i < tileMapkWidth; ++i) {
            drawAlignedTile(tileMapBuffer, { i, j }, bus->read<uint8_t>(0x9800 + i + tileMapkWidth * j)); // background
            drawAlignedTile(tileMapBuffer, { i, j + 32 }, bus->read<uint8_t>(0x9C00 + i + tileMapkWidth * j), 0); // window
        }
    }
}

void PPU::horizontalInterrupt()
{
    LOG("hSync")
}

void PPU::verticalInterrupt()
{
    const auto newInterruptFlag = Utils::setBit(bus->read<uint8_t>(0xFF0F), 0);
    bus->write<uint8_t>(0xFF0F, newInterruptFlag);
    LOG("vSync")
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

    const auto LCDC = bus->read<uint8_t>(0xFF40);
    if (!static_cast<bool>((LCDC & 0b10000000) >> 7)) {
        return; // LCD and PPU disabled
    }

    const auto windowTileMapAddr = static_cast<bool>((LCDC & 0b01000000) >> 6) ? 0x9800 : 0x9C00; // 9800-9BFF : 9C00-9FFF (32x32 = 1024 bytes)
    const auto enableWindow = static_cast<bool>((LCDC & 0b00100000) >> 5);
    const auto tileDataArea = static_cast<bool>((LCDC & 0b00010000) >> 4) ? 0x8000 : 0x9000; // 8000-97FF : 8800-8FFF (4096 bytes)
    const auto backgroundTileMapAddr = static_cast<bool>((LCDC & 0b00001000) >> 3) ? 0x9800 : 0x9C00; // 9800-9BFF : 9C00-9FFF (32x32 = 1024 bytes)
    const auto objSize = static_cast<bool>((LCDC & 0b00000100) >> 2); // false:8x8, true8x16
    const auto enableObj = static_cast<bool>((LCDC & 0b00000010) >> 1);
    const auto enableBackgroundAndWindow = static_cast<bool>(LCDC & 0b00000001);

    const auto SCY = bus->read<uint8_t>(0xFF42);
    const auto SCX = bus->read<uint8_t>(0xFF43);

    /*
    if (m_mode == Mode::OAMSCAN && m_dots >= oamLength) {
        m_mode = Mode::DRAW;
    }
    else if (m_mode == Mode::DRAW && m_dots >= oamLength + drawLength) {
        horizontalInterrupt();
        drawLine(); // TODO - drawDots();
        m_mode = Mode::HBLANK;
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
    */

    static int line = 0;
    bus->write<uint8_t>(0xFF44, line);
    if (line < 144) {
        drawLine(LCDC, SCX, SCY, line);
    }
    if (line == 144) {
        verticalInterrupt();
    }
    if (line >= 153) {
        line = -1;
    }
    line++;
}
