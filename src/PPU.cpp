#include "PPU.hpp"

#include "Bus.hpp"
#include "Utils.hpp"

#include <cstdlib>

PPU::PPU(Bus* bus) : m_bus(bus), m_dots(0), m_mode(Mode::OAMSCAN), m_currentLine(0), m_dotsDrawn(0)
{
    m_frameBuffer.data.resize(160 * 144 * 4); // 160 x 144 x 4 bytes RGBA
    m_frameBuffer.width = 160;

    m_tileDataBuffer.data.resize(3 * 128 * 8 * 8 * 4); // 3 blocks, 128 tiles in each block, each tile 8x8 pixels, each pixel 4 bytes RGBA
    m_tileDataBuffer.width = 128;

    m_tileMapBuffer.data.resize(2 * 32 * 32 * 8 * 8 * 4); // 2 maps, 32x32 tiles in each map, each tile 8x8 pixels, each pixel 4 bytes RGBA
    m_tileMapBuffer.width = 256;

    m_objectBuffer.data.resize(160 * 144 * 4); // same as frameBuffer
    m_objectBuffer.width = 160;
}

std::array<uint8_t, 3> PPU::colorLookup(bool msb, bool lsb) const
{
#define GREEN
    const auto pallet = m_bus->read(0xFF47);
    std::array<uint8_t, 4> colourForId{};
    for (size_t i{}; i < colourForId.size(); ++i) {
        colourForId[i] = static_cast<uint8_t>((pallet >> (2 * i)) & 0b0000'0011);
    }

    const auto id = (static_cast<uint8_t>(msb) << 1) | static_cast<uint8_t>(lsb);
    switch (colourForId[id])
    {
#ifdef GREEN
        case 0: return { 0x9B, 0xBC, 0x0F };
        case 1: return { 0x8B, 0xAC, 0x0F };
        case 2: return { 0x30, 0x62, 0x30 };
        default: return { 0x0F, 0x38, 0x0F };
#else
        case 0: return { 255, 255, 255 };
        case 1: return { 170, 170, 170 };
        case 2: return { 80, 80, 80 };
        default: return { 0, 0, 0 };
#endif
    }
};

void PPU::drawObject(Vbuffer& buffer, XY pixelPos, uint16_t tile, uint8_t flags)
{
    const uint16_t tileStart = 0x8000 + tile * 16;
    const auto screenStart = (pixelPos.first - 8) * 4 + (pixelPos.second - 16) * buffer.width * 4;
    if (screenStart<0 || screenStart > ((4 * 160 * 144) - (4 * 8 * 8))) {
        return;
    }

    const auto xFlip = static_cast<bool>(flags & 0b0010'0000);
    const auto yFlip = static_cast<bool>(flags & 0b0100'0000);
    const auto priority = static_cast<bool>(flags & 0b1000'0000);
    for (int j = 0; j < 8; ++j) { // 8 rows in a tile
        const auto J = yFlip ? 8 - 1 - j : j;
        const auto lsByte = m_bus->read(tileStart + 2 * J);
        const auto msByte = m_bus->read(tileStart + 2 * J + 1);
        for (int i = 0; i < 8; ++i) { // one 8 tile row at a time
            const auto I = xFlip ? 8 - 1 - i : i;
            const auto lsBit = static_cast<bool>(lsByte & (1 << (7 - I)));
            const auto msBit = static_cast<bool>(msByte & (1 << (7 - I)));
            const auto [r, g, b] = colorLookup(msBit, lsBit);
            const auto transparent = (msBit == false) && (lsBit == false);
            if (!transparent) {
                buffer.data[screenStart + 4 * (i + j * buffer.width)] = r;
                buffer.data[screenStart + 4 * (i + j * buffer.width) + 1] = g;
                buffer.data[screenStart + 4 * (i + j * buffer.width) + 2] = b;
                buffer.data[screenStart + 4 * (i + j * buffer.width) + 3] = 255;
            }
        }
    }
};

void PPU::drawAlignedTile(Vbuffer& buffer, XY tilePos, uint16_t tile, bool unsignedMode)
{
    uint16_t tileStart;
    if (unsignedMode) {
        tileStart = 0x8000 + tile * 16;
    }
    else {
        const auto signedTile = (int8_t)tile;
        tileStart = 0x9000 + signedTile * 16;
    }
    const auto screenStart = tilePos.first * 8 + tilePos.second * 8 * buffer.width;
    for (int j = 0; j < 8; ++j) { // 8 rows in a tile
        const auto lsByte = m_bus->read(tileStart + 2 * j);
        const auto msByte = m_bus->read(tileStart + 2 * j + 1);
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

void PPU::drawLine(uint8_t LCDC, uint8_t SCX, uint8_t SCY, uint8_t WX, uint8_t WY, int LC) {
    const auto unsignedMode = static_cast<bool>(LCDC & 0b0001'0000);
    const auto backgroundTileMapAddr = static_cast<bool>(LCDC & 0b0000'1000) ? 0x9C00 : 0x9800; // 9800-9BFF : 9C00-9FFF (32x32 = 1024 bytes)
    const auto windowTileMapAddr = static_cast<bool>(LCDC & 0b0100'0000) ? 0x9C00 : 0x9800;
    const auto windowEnable = static_cast<bool>(LCDC & 0b0010'0000);
    const auto backgroundAndWindowEnable = static_cast<bool>(LCDC & 0b0000'0001);

    uint16_t tileMapAddr{};
    uint8_t scrollX{};
    uint8_t scrollY{};
    //if (windowEnable && (LC >= WY)) { // TODO - window
    if (false) {
        tileMapAddr = windowTileMapAddr;
        scrollX = WX - 7;
        scrollY = LC - WY;
    }
    else {
        tileMapAddr = backgroundTileMapAddr;
        scrollX = SCX;
        scrollY = SCY;
    }

    for (int tileSlice = 0; tileSlice < 20; ++tileSlice) {
        for (int pixel = 0; pixel < 8; ++pixel) { // one 8 tile row at a time
            const auto xTile = ((scrollX + 8 * tileSlice + pixel) % 256) / 8;
            const auto yTile = ((scrollY + LC) % 256) / 8;
            const auto tileNumber = m_bus->read(tileMapAddr + xTile + 32 * yTile);

            uint16_t tileStart;
            if (unsignedMode) {
                tileStart = 0x8000 + tileNumber * 16;
            }
            else {
                const auto signedTile = (int8_t)tileNumber;
                tileStart = 0x9000 + signedTile * 16;
            }

            const auto lsByteIndex = tileStart + 2 * ((scrollY + LC) % 8);
            const auto lsByte = m_bus->read(lsByteIndex);
            const auto msByte = m_bus->read(lsByteIndex + 1);
    
            const auto nonAlignedPixel = (pixel + scrollX) % 8;
            const auto lsBit = static_cast<bool>(lsByte & (1 << (7 - nonAlignedPixel)));
            const auto msBit = static_cast<bool>(msByte & (1 << (7 - nonAlignedPixel)));
            const auto [r, g, b] = colorLookup(msBit, lsBit);

            const int index = pixel + tileSlice * 8 + LC * m_frameBuffer.width;
            m_frameBuffer.data[4 * index] = r;
            m_frameBuffer.data[4 * index + 1] = g;
            m_frameBuffer.data[4 * index + 2] = b;
            m_frameBuffer.data[4 * index + 3] = 255;
        }
    }
};

void PPU::blitObjects(Vbuffer& buffer)
{
    const auto numObjects = 40;
    for (int i = 0; i < numObjects; ++i) {
        const auto Y = m_bus->read(0xFE00 + i * 4);
        const auto X = m_bus->read(0xFE00 + (i * 4) + 1);
        const auto TILE = m_bus->read(0xFE00 + (i * 4) + 2);
        const auto FLAGS = m_bus->read(0xFE00 + (i * 4) + 3);
        drawObject(buffer, { X, Y }, TILE, FLAGS);
    }
}

void PPU::updateDebugVramDisplays()
{
     // tile blocks
    constexpr int tileBlockWidth = 16;
    constexpr int tileBlockHeight = 8;
    for (int j = 0; j < tileBlockHeight; ++j) {
        for (int i = 0; i < tileBlockWidth; ++i) {
            drawAlignedTile(m_tileDataBuffer, { i, j }, i + tileBlockWidth * j); // block 0
            drawAlignedTile(m_tileDataBuffer, { i, j + tileBlockHeight }, i + j * tileBlockWidth + tileBlockHeight * tileBlockWidth); // block 1
            drawAlignedTile(m_tileDataBuffer, { i, j + 2 * tileBlockHeight }, i + j * tileBlockWidth + 2 * tileBlockHeight * tileBlockWidth); // block 2
        }
    }

    // tilemaps
    const auto LCDC = m_bus->read(0xFF40);
    const auto unsignedMode = static_cast<bool>(LCDC & 0b00010000);
    constexpr int tileMapSize = 32;
    for (int j = 0; j < tileMapSize; ++j) {
        for (int i = 0; i < tileMapSize; ++i) {
            drawAlignedTile(m_tileMapBuffer, { i, j }, m_bus->read(0x9800 + i + tileMapSize * j), unsignedMode); // background
            drawAlignedTile(m_tileMapBuffer, { i, j + 32 }, m_bus->read(0x9C00 + i + tileMapSize * j), unsignedMode); // window
        }
    }

    // objects
    m_objectBuffer.clear();
    blitObjects(m_objectBuffer);
}

void PPU::verticalInterrupt()
{
    const auto newInterruptFlag = Utils::setBit(m_bus->read(0xFF0F), static_cast<int>(Interrupt::VBlank));
    m_bus->write(0xFF0F, newInterruptFlag);
}

void PPU::statInterrupt()
{
    const auto newInterruptFlag = Utils::setBit(m_bus->read(0xFF0F), static_cast<int>(Interrupt::LCD));
    m_bus->write(0xFF0F, newInterruptFlag);
}

void PPU::drawDots() {
    const auto LCDC = m_bus->read(0xFF40);

    const auto enableLcdAndPpu = static_cast<bool>((LCDC & 0b1000'0000) >> 7);
    if (!enableLcdAndPpu) {
        return;
    }

    // tiles are 8x8 pixels and 2 bits per pixel -> 16 bytes per tile
    const auto windowTileMapAddr = static_cast<bool>((LCDC & 0b0100'0000) >> 6) ? 0x9800 : 0x9C00; // 9800-9BFF : 9C00-9FFF (32x32 = 1024 bytes)
    const auto enableWindow = static_cast<bool>((LCDC & 0b0010'0000) >> 5);
    const auto tileDataArea = static_cast<bool>((LCDC & 0b0001'0000) >> 4) ? 0x8000 : 0x9000; // 8000-97FF : 8800-8FFF (4096 bytes)
    const auto backgroundTileMapAddr = static_cast<bool>((LCDC & 0b0000'1000) >> 3) ? 0x9800 : 0x9C00; // 9800-9BFF : 9C00-9FFF (32x32 = 1024 bytes)
    const auto objSize = static_cast<bool>((LCDC & 0b0000'0100) >> 2); // false:8x8, true8x16
    const auto enableObj = static_cast<bool>((LCDC & 0b0000'0010) >> 1);
    const auto enableBackgroundAndWindow = static_cast<bool>(LCDC & 0b0000'0001);

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

    const auto LCDC = m_bus->read(0xFF40);
    if (!static_cast<bool>((LCDC & 0b10000000) >> 7)) {
        return; // LCD and PPU disabled
    }

    const auto windowTileMapAddr = static_cast<bool>((LCDC & 0b0100'0000) >> 6) ? 0x9800 : 0x9C00; // 9800-9BFF : 9C00-9FFF (32x32 = 1024 bytes)
    const auto enableWindow = static_cast<bool>((LCDC & 0b0010'0000) >> 5);
    const auto tileDataArea = static_cast<bool>((LCDC & 0b0001'0000) >> 4) ? 0x8000 : 0x9000; // 8000-97FF : 8800-8FFF (4096 bytes)
    const auto backgroundTileMapAddr = static_cast<bool>((LCDC & 0b0000'1000) >> 3) ? 0x9800 : 0x9C00; // 9800-9BFF : 9C00-9FFF (32x32 = 1024 bytes)
    const auto objSize = static_cast<bool>((LCDC & 0b0000'0100) >> 2); // false:8x8, true8x16
    const auto enableObj = static_cast<bool>((LCDC & 0b0000'0010) >> 1);
    const auto enableBackgroundAndWindow = static_cast<bool>(LCDC & 0b0000'0001);

    const auto SCY = m_bus->read(0xFF42);
    const auto SCX = m_bus->read(0xFF43);

    const auto WY = m_bus->read(0xFF4A);
    const auto WX = m_bus->read(0xFF4B);

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

    m_bus->write(0xFF44, m_currentLine);

    const auto LYC = m_bus->read(0xFF45);
    const auto STAT = m_bus->read(0xFF41);
    if (m_currentLine == LYC) {
        const auto newSTAT = Utils::setBit(STAT, 2);
        m_bus->write(0xFF41, newSTAT);
        if (STAT & 0b0100'0000) {
            statInterrupt();
        }
    }
    else {
        const auto newSTAT = Utils::clearBit(STAT, 2);
        m_bus->write(0xFF41, newSTAT);
    }
    
    if (m_currentLine < 144) {
        drawLine(LCDC, SCX, SCY, WX, WY, m_currentLine);
        blitObjects(m_frameBuffer); // TODO - line blit
    }
    if (m_currentLine == 144) {
        verticalInterrupt();
    }
    if (m_currentLine >= 153) {
        m_currentLine = -1;
    }
    m_currentLine++;
}
