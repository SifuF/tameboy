#include "Bus.hpp"

#include <cstring>
#include <fstream>
#include <optional>

Bus::Bus() : m_boot(std::make_unique<uint8_t[]>(0x100)),
             m_map(std::make_unique<uint8_t[]>(0x10000)),
             cpu(this),
             ppu(this)
{    
    std::memset((char*)m_boot.get(), 0, 0x100);
    std::memset((char*)m_map.get(), 0, 0x10000);

    readFile((char*)m_boot.get(), "../roms/DMG_ROM.bin");
    //readFile((char*)m_map.get(), "../roms/tetris.bin");
    //readFile((char*)m_map.get(), "../roms/cpu_instrs.gb");
    //readFile((char*)m_map.get(), "../roms/tennis.bin");
    //readFile((char*)m_map.get(), "../roms/Alleyway.bin");
    //readFile((char*)m_map.get(), "../roms/dr.bin");
    //readFile((char*)m_map.get(), "../roms/spot.gb");
    readFile((char*)m_map.get(), "../roms/taz.gb");

    cpu.reset();
    compareLogo();
}

void Bus::start()
{
    const auto updateScreens = [&]()
    {
        screen.update(ppu.getFrameBuffer());
        ppu.updateDebugVramDisplays();
        screen.updateDebug(ppu.getTileDataBuffer(), ppu.getTileMapBuffer());
    };

    const auto processTimer = [&](uint64_t& counter)
    {
        const auto tac = read<uint8_t>(0xFF07);
        const auto enable = static_cast<bool>(tac & 0b0000'0100);
        if (!enable) {
            return;
        }

        const auto clockSelect = static_cast<uint8_t>(tac & 0b0000'0011);
        uint16_t clock{};
        switch (clockSelect) {
            case 0: clock = 256; break;
            case 1: clock = 4; break;
            case 2: clock = 16; break;
            case 3: clock = 64; break;
            default: throw std::runtime_error("Bad clock select");
        }
        if (counter >= clock) {
            const auto tima = read<uint8_t>(0xFF05);
            const auto modulo = read<uint8_t>(0xFF06);
            if (tima == 0xFF) {
                write<uint8_t>(0xFF05, modulo);

                const auto newInterruptFlag = Utils::setBit(read<uint8_t>(0xFF0F), static_cast<int>(Interrupt::Timer));
                write<uint8_t>(0xFF0F, newInterruptFlag);
                LOG("Timer interrupt");
            }
            else {
                write<uint8_t>(0xFF05, tima + 1);
            }
            counter -= clock;
        }
    };

    const auto processDivider = [&](uint64_t& counter)
    {
        if (counter >= 64) {
            m_map[0xFF04] += 1;
            counter -= 64;
        }
    };

    const auto processSerial = [&](uint64_t& counter)
    {
        if (counter >= 4) {                        // |        7        | 6 5 4 3 2 |      1      |      0       |
            const auto SC = read<uint8_t>(0xFF02); // | Transfer enable |           | Clock speed | Clock select |
            if (SC == 0b1000'0001) {  // transfer enable & master clock
                const auto SB = read<uint8_t>(0xFF01);
                std::cout << static_cast<int >(SB);
                const auto clearEnable = Utils::clearBit(SC, 7);
                write<uint8_t>(0xFF02, clearEnable);

                const auto newInterruptFlag = Utils::setBit(read<uint8_t>(0xFF0F), static_cast<int>(Interrupt::Serial));
                write<uint8_t>(0xFF0F, newInterruptFlag);
                LOG("Serial interrupt");
            }
            counter -= 4;
        }
    };

    uint64_t instructionCounter{};
    uint64_t cycleCounter{};
    
    uint64_t timerCycleCounter{};
    uint64_t dividerCycleCounter{};
    uint64_t serialCycleCounter{};

    while (true)
    {
        processTimer(timerCycleCounter);
        processDivider(dividerCycleCounter);
        processSerial(serialCycleCounter);

        const auto cycles = cpu.fetchDecodeExecute();

        if (instructionCounter % 70 == 0) {
        //if (cycleCounter % 456 == 0) {
            ppu.tick(cycles);
        }
        if (instructionCounter % 10000 == 0) {
            updateScreens();
        }

        instructionCounter++;
        
        cycleCounter += cycles;
        timerCycleCounter += cycles;
        dividerCycleCounter += cycles;
        serialCycleCounter += cycles;
    }
}

void Bus::readFile(char* buffer, const char* filename)
{
    std::ifstream fs(filename, std::ios::binary | std::ios::ate);
    if(!fs) {
        throw std::runtime_error("Cannot open ROM file!");
    }

    const auto bytes = fs.tellg();
    fs.seekg(0);
    fs.read(buffer, bytes);
}
  
void Bus::compareLogo()
{
    for(int i=0; i<48; ++i) {
        if(m_map[0x104 + i] != m_boot[0xA8 + i]) {
            std::cout << i << " ";
            throw std::runtime_error("Logos don't match!");
        }
    }
}
