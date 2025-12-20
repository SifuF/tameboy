#pragma once

#include <cstdint>

enum class Interrupt {
    VBlank = 0,
    LCD,
    Timer,
    Serial,
    Joypad
};

namespace Utils {
    [[nodiscard]] inline uint8_t setBit(uint8_t val, int bit) {
        return val | (1 << bit);
    };

    [[nodiscard]] inline uint8_t clearBit(uint8_t val, int bit) {
        return val & (~(1 << bit));
    };
};
