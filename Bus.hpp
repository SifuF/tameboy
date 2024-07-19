#pragma once

#include "CPULR35902.hpp"

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

private:
    void printRom();

    std::unique_ptr<unsigned char[]> map = nullptr;
    CPULR35902 cpu;

};

template<typename T>
T Bus::read(uint16_t addr) {
    if(sizeof(T)==1) {
        return map[addr];
    }
    else {
        return static_cast<T>((map[addr] << 8 ) | map[addr + 1]);
    }
}

template<typename T>
void Bus::write(uint16_t addr, T value) {
   if(addr < 0x8000)
       return;

   if(sizeof(T)==1) {
       	map[addr] = value;
   }
   else {
       map[addr] = static_cast<uint8_t>(value >> 8);
       map[addr + 1] = static_cast<uint8_t>(value);  
   }
}

