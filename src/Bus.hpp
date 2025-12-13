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

    template<typename T>
    T read(uint16_t addr);

    template<typename T>
    void write(uint16_t addr, T value);

    // debug
    uint8_t* getMap() { return m_map.get(); }

private:
    void readFile(char* buffer, const char* filename);
    void printMap(uint16_t offset, uint16_t lines);
    void compareLogo();

    void timerInterrupt()
    {
        const auto newInterruptFlag = Utils::setBit(read<uint8_t>(0xFF0F), 1);
        write<uint8_t>(0xFF0F, newInterruptFlag);
        LOG("Timer interrupt");
    }

    void tickTimer() {
        const auto tima = read<uint8_t>(0xFF05);
        const auto modulo = read<uint8_t>(0xFF06);
        const auto update = (tima == 0xFF) ? modulo : tima + 1;
        write<uint8_t>(0xFF05, update);
        timerInterrupt();
    }

    bool bootRom = true;
    std::unique_ptr<uint8_t[]> m_boot = nullptr;
    std::unique_ptr<uint8_t[]> m_map = nullptr;
    CPULR35902 cpu;
    PPU ppu;
    Screen screen;
};

template<typename T>
T Bus::read(uint16_t addr) {
    auto & map = (bootRom && (addr < 0x100)) ? m_boot : m_map;   
   
    if(sizeof(T)==1) { 
        return map[addr];
    }
    else {
        return static_cast<T>(map[addr]) | (map[addr + 1] << 8);
    }
}  

template<typename T>
void Bus::write(uint16_t addr, T value) {
    if(addr < 0x8000) // ROM
        return;

    if (addr == 0xFF04) { // divider register
        m_map[0xFF04] = 0;
        return;
    }

    if (addr == 0xFF50)
        bootRom = false;

    if(sizeof(T)==1) {
       	m_map[addr] = value;
    }
    else {
        m_map[addr] = static_cast<uint8_t>(value);
        m_map[addr + 1] = static_cast<uint8_t>(value >> 8);
    }
}
