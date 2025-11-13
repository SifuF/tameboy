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
    ~Bus();
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
    void initVram();

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
    if(addr < 0x8000)
        return;

    if (addr == 0xFF50)
        bootRom = false;

    const auto & map = m_map;
    if(sizeof(T)==1) {
       	map[addr] = value;
    }
    else {
        map[addr] = static_cast<uint8_t>(value);
        map[addr + 1] = static_cast<uint8_t>(value >> 8);
    }
}
