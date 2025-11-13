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

    template<typename T>
    class RingBuffer {
    public:
        explicit RingBuffer(size_t capacity)
            : data(capacity), capacity(capacity) {}

        void push(const T& value) {
            data[head] = value;
            head = (head + 1) % capacity;
            if (size < capacity) ++size;
        }

        const T& operator[](size_t index) const {
            return data[(head + capacity - size + index) % capacity];
        }

        size_t getSize() const { return size; }

    private:
        std::vector<T> data;
        size_t capacity;
        size_t head = 0;
        size_t size = 0;
    };
};
