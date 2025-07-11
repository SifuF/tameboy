#include "Bus.hpp"

#include <cstring>
#include <fstream>
#include <optional>

Bus::Bus() : m_boot(std::make_unique<uint8_t[]>(0x100)),
             m_map(std::make_unique<uint8_t[]>(0x10000)),
             cpu(this),
             ppu(this) {
    
    readFile((char*)m_boot.get(), "../roms/DMG_ROM.bin");
    readFile((char*)m_map.get(), "../roms/tetris.bin");

    cpu.reset();
    
    printMap(0x0, 4);
    compareLogo();
    initVram();
}

Bus::~Bus() {

}

void Bus::initVram() {
    constexpr size_t tileSize = 16;
    uint8_t tileX[tileSize] = {
        0b10000010, 0b00000000,
        0b01000100, 0b00000000,
        0b00101000, 0b00000000,
        0b00010000, 0b00000000,
        0b00101000, 0b00000000,
        0b01000100, 0b00000000,
        0b10000010, 0b00000000,
        0b00000000, 0b00000000
    };

    uint8_t tileF[tileSize] = {
        0b11111110, 0b00000000,
        0b10000000, 0b01111110,
        0b10000000, 0b00000000,
        0b11110000, 0b00000000,
        0b10000000, 0b01110000,
        0b10000000, 0b00000000,
        0b10000000, 0b00000000,
        0b00000000, 0b00000000
    };

    uint8_t tileO[tileSize] = {
        0b01111100, 0b01111100,
        0b10000010, 0b10000010,
        0b10000010, 0b10000010,
        0b10000010, 0b10000010,
        0b10000010, 0b10000010,
        0b10000010, 0b10000010,
        0b01111100, 0b01111100,
        0b00000000, 0b00000000
    };

    // X in 0th and F in 1st tile in block 0
    for (int i = 0; i < 128; ++i) {
        std::memcpy(m_map.get() + 0x8000 + i * tileSize, tileX, tileSize); // fill block 0 with X
        std::memcpy(m_map.get() + 0x8800 + i * tileSize, tileF, tileSize); // fill block 1 with F
        std::memcpy(m_map.get() + 0x9000 + i * tileSize, tileO, tileSize); // fill block 2 with O
    }

    // fill background with 0th tile
    std::memset(m_map.get() + 0x9800, 0, 32*32);

    // fill window with 1st tile
    std::memset(m_map.get() + 0x9C00, 1, 32*32);
}

void Bus::start() {
    const auto update = [&](){
        ppu.updateVramDisplay();
        screen.update(ppu.getFrameBuffer(), ppu.getTileDataBuffer(), ppu.getTileMapBuffer());
    };

    unsigned long long instructionCounter = 0;
    std::optional<unsigned long long> instructionTarget{};

    while (true) {
        if (instructionTarget == instructionCounter) {
            try {
                update();
                std::cout << "Enter no. of instructions to execute: ";
                std::string str{};
                std::getline(std::cin, str);
                instructionTarget = instructionCounter + std::stoi(str);
            }
            catch (...) {
                throw std::runtime_error("invalid no. of instructions!");
            }
        }
        else {
            LOGND(instructionCounter); LOGND(": ");
            const auto cycles = cpu.fetchDecodeExecute();
            instructionCounter++;
            ppu.tick(cycles);
            if (instructionCounter % 100 == 0) {
                update();
            }
        }
    }
}

void Bus::readFile(char* buffer, const char* filename) {
    std::ifstream fs(filename, std::ios::binary | std::ios::ate);
    if(!fs) {
        throw std::runtime_error("Cannot open ROM file!");
    }

    const auto bytes = fs.tellg();
    fs.seekg(0);
    fs.read(buffer, bytes);
}
   
void Bus::printMap(uint16_t offset, uint16_t lines) {
    const uint16_t bytesPerLine = 16;
    for(uint16_t j=0; j<lines; ++j) {
        for(uint16_t i=0; i<bytesPerLine; ++i) {
            const auto byte = read<uint8_t>(offset + i + j*bytesPerLine);
            std::cout << std::hex << static_cast<int>(byte) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Bus::compareLogo() {
    for(int i=0; i<48; ++i) {
        if(m_map[0x104 + i] != m_boot[0xA8 + i]) {
            std::cout << i << " ";
            throw std::runtime_error("Logos don't match!");
        }
    }
}
