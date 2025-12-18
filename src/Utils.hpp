#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

// #define DEBUG_LOG

#if defined(DEBUG_LOG)
#define LOG(x) std::cout << std::hex << x << std::endl;
#define LOGN(x) std::cout << std::hex << x;
#define LOGD(x) std::cout << std::dec << x << std::endl;
#define LOGND(x) std::cout << std::dec << x;
#else
#define LOG(x)
#define LOGN(x)
#define LOGD(x)
#define LOGND(x)
#endif

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
