#pragma once

#include <cstdint>
#include <vector>

class Bus;

using XY = std::pair<uint8_t, uint8_t>;

enum class Mode {
    OAMSCAN = 2,
    DRAW = 3,
    HBLANK = 0,
    VBLANK = 1
};

struct Vbuffer {
    std::vector<uint8_t> data{};
    uint16_t width{};
};

class PPU {
public:
    PPU(Bus* bus);
    void tick(uint32_t cycles);
    void updateDebugVramDisplays();
    const std::vector<uint8_t>& getFrameBuffer() const { return frameBuffer.data; }
    const std::vector<uint8_t>& getTileDataBuffer() const { return tileDataBuffer.data; }
    const std::vector<uint8_t>& getTileMapBuffer() const { return tileMapBuffer.data; }

private:
    std::array<uint8_t, 3> colorLookup(bool msb, bool lsb) const;
    void drawAlignedTile(Vbuffer& buffer, XY tilePos, uint16_t tile, bool unsignedMode = true);
    void drawLine(uint8_t LCDC, uint8_t SCX, uint8_t SCY, int LC);
    void drawDots();
    
    void verticalInterrupt();
    void statInterrupt();

    Vbuffer frameBuffer;
    Vbuffer tileDataBuffer;
    Vbuffer tileMapBuffer;
    Bus* bus = nullptr;

    uint32_t m_dots;
    Mode m_mode = Mode::OAMSCAN;
    uint8_t m_currentLine{};
    uint32_t m_dotsDrawn;
};
