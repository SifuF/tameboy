#pragma once

#include "CPULR35902.hpp"
#include "PPU.hpp"
#include "Screen.hpp"
#include "Sound.hpp"

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
        m_ppu.updateDebugVramDisplays();
        m_screen.updateDebug(m_ppu.getTileDataBuffer(), m_ppu.getTileMapBuffer(), m_ppu.getObjectBuffer());
    }

private:
    void readFile(char* buffer, const char* filename);
    void compareLogo();

    bool m_bootRom = true;
    std::unique_ptr<uint8_t[]> m_boot = nullptr;
    std::unique_ptr<uint8_t[]> m_map = nullptr;
    CPULR35902 m_cpu;
    PPU m_ppu;
    Screen m_screen;
    Sound m_sound;

    uint64_t m_instructionCounter{};
    uint64_t m_cycleCounter{};
};
