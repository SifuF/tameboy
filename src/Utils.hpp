#pragma once

#include <cstdint>

enum Interrupt {
    VBlank,
    LCD,
    Timer,
    Serial,
    Joypad
};

namespace Utils {
    inline uint8_t setBit(uint8_t val, int bit) {
        return val | (1 << bit);
    };

    inline uint8_t clearBit(uint8_t val, int bit) {
        return val & (~(1 << bit));
    };
};
