#pragma once

#include "CPULR35902.hpp"
#include "PPU.hpp"
#include "Screen.hpp"

#include <cassert>
#include <iostream>
#include <memory>

class Bus {
public:
    Bus();
    void start();
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);

    // debug
    uint8_t* getMap() { return m_map.get(); }
    void forceDraw()
    {
        ppu.updateDebugVramDisplays();
        screen.updateDebug(ppu.getTileDataBuffer(), ppu.getTileMapBuffer(), ppu.getObjectBuffer());
    }

private:
    void readFile(char* buffer, const char* filename);
    void compareLogo();

    bool bootRom = true;
    std::unique_ptr<uint8_t[]> m_boot = nullptr;
    std::unique_ptr<uint8_t[]> m_map = nullptr;
    CPULR35902 cpu;
    PPU ppu;
    Screen screen;

    uint64_t m_instructionCounter{};
    uint64_t m_cycleCounter{};
};
