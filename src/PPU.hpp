#pragma once

#include <array>
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
    void clear() {
        std::memset(data.data(), 0, data.size());
    }
};

class PPU {
public:
    PPU(Bus* bus);
    void tick(uint32_t cycles);
    void updatePaletteLookup(uint8_t map);
    void updateDebugVramDisplays();
    const std::vector<uint8_t>& getFrameBuffer() const { return m_frameBuffer.data; }
    const std::vector<uint8_t>& getTileDataBuffer() const { return m_tileDataBuffer.data; }
    const std::vector<uint8_t>& getTileMapBuffer() const { return m_tileMapBuffer.data; }
    const std::vector<uint8_t>& getObjectBuffer() const { return m_objectBuffer.data; }

private:
    void drawAlignedTile(Vbuffer& buffer, XY tilePos, uint16_t tile, bool unsignedMode = true);
    void drawObject(Vbuffer& buffer, XY pos, uint16_t tile, uint8_t flags);
    void drawLine(uint8_t LCDC, uint8_t SCX, uint8_t SCY, uint8_t WX, uint8_t WY, int LC);
    void drawDots();
    void blitObjects(Vbuffer& buffer);

    void verticalInterrupt();
    void statInterrupt();

    std::array<uint8_t, 4> m_paletteLookup{};

#define GREEN
#ifdef GREEN
    static constexpr std::array<std::array<uint8_t, 3>, 4> m_palette{ { {{ 0x9B, 0xBC, 0x0F}}, {{0x8B, 0xAC, 0x0F}}, {{0x30, 0x62, 0x30}}, {{0x0F, 0x38, 0x0F}} }  };
#else
    static constexpr std::array<std::array<uint8_t, 3>, 4> m_palette{ { {{255, 255, 255}}, {{170, 170, 170}}, {{ 80,  80,  80}}, {{  0,   0,   0}} } };
#endif

    Vbuffer m_frameBuffer;
    Vbuffer m_tileDataBuffer;
    Vbuffer m_tileMapBuffer;
    Vbuffer m_objectBuffer;
    Bus* m_bus{};

    uint32_t m_dots;
    Mode m_mode = Mode::OAMSCAN;
    uint8_t m_currentLine{};
    uint32_t m_dotsDrawn{};
    uint64_t m_cycleCounter{};
};
