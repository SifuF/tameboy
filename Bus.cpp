#include "Bus.hpp"
#include <fstream>

Bus::Bus() : m_boot(std::make_unique<uint8_t[]>(0x100)),
             m_map(std::make_unique<uint8_t[]>(0x10000)),
             cpu(this) {
    
    readFile((char*)m_boot.get(), "roms/DMG_ROM.bin");
    readFile((char*)m_map.get(), "roms/tetris.bin");
    
    parseHeader();
    printMap(0x0, 4);
}

Bus::~Bus() {

}

void Bus::start() {
   while(true) {
       cpu.fetchDecodeExecute();
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

void Bus::parseHeader() {
    for(int i=0; i<48; ++i) {
        if(m_map[0x104 + i] != m_boot[0xA8 + i]) {
            std::cout << i << " ";
            throw std::runtime_error("Logos don't match!");
        }
    }
}
