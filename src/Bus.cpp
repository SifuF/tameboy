#include "Bus.hpp"

#include <cstring>
#include <fstream>
#include <optional>

Bus::Bus() :
    m_boot(std::make_unique<uint8_t[]>(0x100)),
    m_map(std::make_unique<uint8_t[]>(0x10000)),
    m_cpu(this),
    m_ppu(this),
    m_screen(this)
{    
    std::memset((char*)m_boot.get(), 0, 0x100);
    std::memset((char*)m_map.get(), 0, 0x10000);

    readFile((char*)m_boot.get(), "../roms/DMG_ROM.bin");
    readFile((char*)m_map.get(), "../roms/tetris_no_UpdateAudio.bin");
    //readFile((char*)m_map.get(), "../roms/cpu_instrs.gb");
    //readFile((char*)m_map.get(), "../roms/tennis.bin");
    //readFile((char*)m_map.get(), "../roms/Alleyway.bin");
    //readFile((char*)m_map.get(), "../roms/dr.bin");
    //readFile((char*)m_map.get(), "../roms/spot.gb");
    //readFile((char*)m_map.get(), "../roms/taz.gb");

    m_cpu.reset();
    compareLogo();

    m_sound.run(); // TODO
}

void Bus::start()
{
    const auto updateScreens = [&]()
    {
        m_screen.update(m_ppu.getFrameBuffer());
        m_ppu.updateDebugVramDisplays();
        m_screen.updateDebug(m_ppu.getTileDataBuffer(), m_ppu.getTileMapBuffer(), m_ppu.getObjectBuffer());
    };

    const auto processTimer = [&](uint64_t& counter)
    {
        const auto tac = read(0xFF07);
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
            const auto tima = read(0xFF05);
            const auto modulo = read(0xFF06);
            if (tima == 0xFF) {
                write(0xFF05, modulo);
                const auto newInterruptFlag = Utils::setBit(read(0xFF0F), static_cast<int>(Interrupt::Timer));
                write(0xFF0F, newInterruptFlag);
            }
            else {
                write(0xFF05, tima + 1);
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
            const auto SC = read(0xFF02); // | Transfer enable |           | Clock speed | Clock select |
            if (SC == 0b1000'0001) {  // transfer enable & master clock
                const auto SB = read(0xFF01);
                std::cout << static_cast<char>(SB) << " ";
                const auto clearEnable = Utils::clearBit(SC, 7);
                write(0xFF02, clearEnable);

                const auto newInterruptFlag = Utils::setBit(read(0xFF0F), static_cast<int>(Interrupt::Serial));
                write(0xFF0F, newInterruptFlag);
            }
            counter -= 4;
        }
    };
    
    uint64_t timerCycleCounter{};
    uint64_t dividerCycleCounter{};
    uint64_t serialCycleCounter{};

    while (true)
    {
        processTimer(timerCycleCounter);
        processDivider(dividerCycleCounter);
        processSerial(serialCycleCounter);

        const auto cycles = m_cpu.fetchDecodeExecute();

        if (m_instructionCounter % 70 == 0) {
        //if (cycleCounter % 456 == 0) {
            m_ppu.tick(cycles);
        }
        if (m_instructionCounter % 10000 == 0) {
            updateScreens();
        }

        m_instructionCounter++;
        
        m_cycleCounter += cycles;
        timerCycleCounter += cycles;
        dividerCycleCounter += cycles;
        serialCycleCounter += cycles;
    }
}

uint8_t Bus::read(uint16_t addr)
{
    auto& map = (m_bootRom && (addr < 0x100)) ? m_boot : m_map;

    if (addr == 0xFF00) {
        const auto joypad = m_screen.getJoypad();
        const auto dPad = (joypad & 0xF0) >> 4;
        const auto buttons = joypad & 0x0F;
        const auto bits4to5 = static_cast<uint8_t>((m_map[0xFF00] & 0b0011'0000) >> 4);
        const auto msn = m_map[0xFF00] & 0xF0;
        switch (bits4to5) {
            case 0: return msn | (buttons & dPad);
            case 1: return msn | buttons;
            case 2: return msn | dPad;
            default: return msn | 0x0F;
        }
    }

    return map[addr];
}

void Bus::write(uint16_t addr, uint8_t value)
{
    if (addr < 0x8000) // ROM
        return;

    if (addr == 0xFF00) { // Joypad. Only bits 4 and 5 are writable
        m_map[0xFF00] = (m_map[0xFF00] & 0b1100'1111) | (value & 0b0011'0000);
        return;
    }

    if (addr == 0xFF04) { // divider register
        m_map[0xFF04] = 0;
        return;
    }

    if (addr == 0xFF46) { // DMA
        m_cycleCounter += 160;
        const auto src = static_cast<uint16_t>(value << 8);
        std::memcpy((m_map.get() + 0xFE00), (m_map.get() + src), 160);
        return;
    }

    if (addr == 0xFF50)
        m_bootRom = false;

    m_map[addr] = value;
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
