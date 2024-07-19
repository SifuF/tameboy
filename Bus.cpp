#include "Bus.hpp"
#include <fstream>

Bus::Bus() : map(std::make_unique<unsigned char[]>(0x10000)), cpu(this) {
    std::ifstream fs("roms/tetris.bin", std::ios::binary | std::ios::ate);
    if(!fs) {
        throw std::runtime_error("Cannot open ROM file!");
    }

    const auto bytes = fs.tellg();
    fs.seekg(0);
    fs.read((char*)map.get(), bytes);

    printRom();
}

Bus::~Bus() {

}

void Bus::start() {
   //while(true) {
       cpu.fetchDecodeExecute();
   //}
}

void Bus::printRom() {
    for(int i=0; i<16; ++i) {
        std::cout << std::hex << static_cast<int>(map[i]) << " ";
    }
    std::cout << std::endl;
}
