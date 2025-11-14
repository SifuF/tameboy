#include "CPULR35902.hpp"

#include "Bus.hpp"

#include <iostream>
#include <sstream>
#include <string>

CPULR35902::CPULR35902(Bus* bus) : bus(bus) {
    initOpcodeHandlers();
}

CPULR35902::~CPULR35902() {

}

void CPULR35902::printState() {
    auto* mem = bus->getMap();
    std::cout << std::hex << "CPU state:\n"
        << "AF=" << AF.w << " "
        << "BC=" << BC.w << " "
        << "DE=" << DE.w << " "
        << "HL=" << HL.w << " "
        << "SP=" << SP.w << " "
        << "PC=" << PC.w << "\n"

        // Timer registers
        << "TIMA 0xFF05=" << static_cast<int>(*(mem + 0xFF05)) << " "  // Timer counter
        << "TMA 0xFF06=" << static_cast<int>(*(mem + 0xFF06)) << " "  // Timer modulo
        << "TAC 0xFF07=" << static_cast<int>(*(mem + 0xFF07)) << " "  // Timer control

        // LCD control & status
        << "LCDC 0xFF40=" << static_cast<int>(*(mem + 0xFF40)) << " "  // LCD Control
        << "STAT 0xFF41=" << static_cast<int>(*(mem + 0xFF41)) << " "  // LCD Status
        << "SCY 0xFF42=" << static_cast<int>(*(mem + 0xFF42)) << " "  // Scroll Y
        << "SCX 0xFF43=" << static_cast<int>(*(mem + 0xFF43)) << " "  // Scroll X

        // More LCD / PPU registers
        << "LYC 0xFF45=" << static_cast<int>(*(mem + 0xFF45)) << " "  // LY Compare
        << "BGP 0xFF47=" << static_cast<int>(*(mem + 0xFF47)) << " "  // BG Palette Data
        << "OBP0 0xFF48=" << static_cast<int>(*(mem + 0xFF48)) << " "  // OBJ Palette 0 Data
        << "OBP1 0xFF49=" << static_cast<int>(*(mem + 0xFF49)) << " "  // OBJ Palette 1 Data
        << "WY 0xFF4A=" << static_cast<int>(*(mem + 0xFF4A)) << " "  // Window Y position
        << "WX 0xFF4B=" << static_cast<int>(*(mem + 0xFF4B)) << " "  // Window X position

        // Interrupt Enable register
        << "IE 0xFFFF=" << static_cast<int>(*(mem + 0xFFFF)) << " "  // Interrupt Enable Flags
        << "\n";
}

void CPULR35902::reset() {
    AF.w = 0;
    BC.w = 0;
    DE.w = 0;
    HL.w = 0;
    SP.w = 0;
    PC.w = 0x0; // TODO PC.w = 0x100;
}

void CPULR35902::setFlags(int Z, int  N, int  H, int C) {
    const auto set = [this](Flag flag, bool high){
        uint8_t mask = 0;
        switch(flag) {
            case Flag::Z : { mask = 0b10000000; break; }
            case Flag::N : { mask = 0b01000000; break; }
            case Flag::H : { mask = 0b00100000; break; }
            case Flag::C : { mask = 0b00010000; break; }
        }

        if(high)
            AF.right |= mask;
        else
            AF.right &= ~mask;
    };
   
    if(Z >= 0)
        set(Flag::Z, Z);

    if(N >= 0)
        set(Flag::N, N);

    if(H >= 0)
        set(Flag::H, H);

    if(C >= 0)
        set(Flag::C, C);
}

bool CPULR35902::getFlag(Flag flag) {
    switch(flag) {
        using enum Flag;
        case Z : { return static_cast<bool>(AF.right & 0b10000000); }
        case N : { return static_cast<bool>(AF.right & 0b01000000); }
        case H : { return static_cast<bool>(AF.right & 0b00100000); }
        case C : { return static_cast<bool>(AF.right & 0b00010000); }
        default : { throw std::runtime_error("Invalid flag get!"); }
    }
}

void CPULR35902::processInterrupts() {
    const auto interruptEnable = bus->read<uint8_t>(0xFFFF);
    const auto interruptFlag = bus->read<uint8_t>(0xFF0F);
    for (int i = static_cast<int>(Interrupt::VBlank); i <= static_cast<int>(Interrupt::Joypad); ++i)
    {
        const auto mask = 1 << i;
        const auto interrupt = static_cast<Interrupt>(i);
        if (interruptFlag & interruptEnable & mask)
        {
            interruptMasterEnable = false;
            
            auto jumpToHandler = [&](uint16_t addr) {
                T += 20;
                SP.w -= 2;
                bus->write<uint16_t>(SP.w, PC.w);
                PC.w = addr;

                if(m_debug)
                    logInstruction("INT " + toHexString(addr), false);
            };
            
            switch (interrupt) { 
                using enum Interrupt;
                case VBlank: {
                    bus->write<uint8_t>(0xFF0F, Utils::clearBit(interruptFlag, 0));
                    jumpToHandler(0x40);
                    break;
                }
                case LCD: {
                    bus->write<uint8_t>(0xFF0F, Utils::clearBit(interruptFlag, 1));
                    jumpToHandler(0x48);
                    break;
                }
                case Timer: {
                    bus->write<uint8_t>(0xFF0F, Utils::clearBit(interruptFlag, 2));
                    jumpToHandler(0x50);
                    break;
                }
                case Serial: {
                    bus->write<uint8_t>(0xFF0F, Utils::clearBit(interruptFlag, 3));
                    jumpToHandler(0x58);
                    break;
                }
                case Joypad: {
                    bus->write<uint8_t>(0xFF0F, Utils::clearBit(interruptFlag, 4));
                    jumpToHandler(0x60);
                    break;
                }
                default: { throw std::runtime_error("Invalid interrupt requested!"); }
            }
        }
    }
}

uint64_t CPULR35902::fetchDecodeExecute() {
    const uint64_t Tstart = T;

    static unsigned long long instructionCounter = 0;
    static unsigned long long instructionTarget = 100000000;
    static bool canStep = true;

    if (PC.w == 0x2bb) {
        // instructionTarget = instructionCounter;
    }

    if (canStep && (instructionTarget == instructionCounter)) {
        m_debug = true;
        std::cout << "Enter no. of instructions to run, Enter = step, s = dump CPU state, c = continue >>> ";
        std::string str{};
        try {
            std::getline(std::cin, str);
            instructionTarget = instructionCounter + 1;
            if (str == "c") {
                canStep = false;
            }
            else if (str == "s") {
                printState();
            }
            else if (str == "") {

            }
            else {
                instructionTarget += std::stoi(str);
            }
        }
        catch (...) {
            std::cout << "invalid no. of instructions!\n";
        }
    }
    else {
        m_debug = false;
    }

    if (m_debug)
        std::cout << std::dec << instructionCounter << ": ";

    if(interruptMasterEnable)
        processInterrupts();

    if(halt || stop)
        return 0;
 
    if (m_debug)
        std::cout << std::hex << "PC=" << static_cast<int>(PC.w) << " ";

    const auto instruction = bus->read<uint8_t>(PC.w);
    PC.w++;
    if (m_debug)
        logInstruction(toHexString(instruction), false);

    if(instruction == 0xCB) {
        const auto prefixInstruction = bus->read<uint8_t>(PC.w);
        PC.w++;
        if (m_debug)
            logInstruction(toHexString(prefixInstruction), false);
        
        prefixHandler[prefixInstruction]();
    }
    else {
        opcodeHandler[instruction]();
    }

    instructionCounter++;

    return T - Tstart;
}

std::string CPULR35902::toHexString(int value) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << value;
    return ss.str();
}

void CPULR35902::OP_00() {
    T += 4;
    if (m_debug) logInstruction("NOP");
}
void CPULR35902::OP_01() {
    T += 12;
    BC.w = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    if (m_debug) logInstruction("LD BC, $" + toHexString(BC.w));
}
void CPULR35902::OP_02() {
    T += 8;
    bus->write<uint8_t>(BC.w, AF.left);
    if (m_debug) logInstruction("LD (BC), A");
}
void CPULR35902::OP_03() {
    T += 8;
    BC.w++;
    if (m_debug) logInstruction("BC");
}
void CPULR35902::OP_04() {
    T += 4;
    const bool half = (BC.left == 0xF);
    BC.left++;
    setFlags((BC.left == 0), 0, half, -1);
    if (m_debug) logInstruction("INC B");
}
void CPULR35902::OP_05() {
    T += 4;
    const bool half = (BC.left == 0x0);
    BC.left--;
    setFlags((BC.left == 0), 1, half, -1);
    if (m_debug) logInstruction("DEC B");
}
void CPULR35902::OP_06() {
    T += 8;
    BC.left = bus->read<uint8_t>(PC.w);
    PC.w++;
    if (m_debug) logInstruction("LD B, $" + toHexString(BC.left));
}
void CPULR35902::OP_07() { // TODO Check Z flag
    T += 4;
    const uint8_t msb = AF.left >> 7;
    setFlags(0, 0, 0, msb);
    AF.left = (AF.left << 1) | msb;
    if (m_debug) logInstruction("RLCA");
}
void CPULR35902::OP_08() {
    T += 20;
    const auto addr = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    bus->write<uint16_t>(addr, SP.w);
    if (m_debug) logInstruction("LD ($" + toHexString(addr) + ") ,SP");
}
void CPULR35902::OP_09() {
    T += 8;
    const bool half = (HL.right + BC.right) > 0xFF;
    const bool carry = (HL.w + BC.w) > 0xFFFF;
    HL.w += BC.w;
    setFlags(-1, 0, half, carry);
    if (m_debug) logInstruction("ADD HL, BC");
}
void CPULR35902::OP_0A() {
    T += 8;
    AF.left = bus->read<uint8_t>(BC.w);
    if (m_debug) logInstruction("LD A, (BC)");
}
void CPULR35902::OP_0B() {
    T += 8;
    BC.w--;
    if (m_debug) logInstruction("DEC BC");
}
void CPULR35902::OP_0C() {
    T += 4;
    const bool half = (BC.right == 0xF);
    BC.right++;
    setFlags((BC.right == 0), 0, half, -1);
    if (m_debug) logInstruction("INC C");
}
void CPULR35902::OP_0D() {
    T += 4;
    const bool half = (BC.right == 0x0);
    BC.right--;
    setFlags((BC.right == 0), 1, half, -1);
    if (m_debug) logInstruction("DEC C");
}
void CPULR35902::OP_0E() {
    T += 8;
    BC.right = bus->read<uint8_t>(PC.w);
    PC.w++;
    if (m_debug) logInstruction("LD C, $" + toHexString(BC.right));
}
void CPULR35902::OP_0F() {
    T += 4;
    const uint8_t lsb = AF.left & 0x01;
    setFlags(0, 0, 0, lsb);
    AF.left = (AF.left >> 1) | (lsb << 7);
    if (m_debug) logInstruction("RRCA");
}
void CPULR35902::OP_10() {
    T += 4;
    stop = true;
    const auto value = bus->read<uint8_t>(PC.w);
    PC.w++;
    if (m_debug) logInstruction("STOP " + toHexString(value));
}
void CPULR35902::OP_11() {
    T += 12;
    DE.w = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    if (m_debug) logInstruction("LD DE, $" + toHexString(DE.w));
}
void CPULR35902::OP_12() {
    T += 8;
    bus->write<uint8_t>(DE.w, AF.left);
    if (m_debug) logInstruction("LD (DE), A");
}
void CPULR35902::OP_13() {
    T += 8;
    DE.w++;
    if (m_debug) logInstruction("INC DE");
}
void CPULR35902::OP_14() {
    T += 4;
    const bool half = (DE.left == 0xF);
    DE.left++;
    setFlags((DE.left == 0), 0, half, -1);
    if (m_debug) logInstruction("INC D");
}
void CPULR35902::OP_15() {
    T += 4;
    const bool half = (DE.left == 0x0);
    DE.left--;
    setFlags((DE.left == 0), 1, half, -1);
    if (m_debug) logInstruction("DEC D");
}
void CPULR35902::OP_16() {
    T += 8;
    DE.left = bus->read<uint8_t>(PC.w);
    PC.w++;
    if (m_debug) logInstruction("LD D, " + toHexString(DE.left));
}
void CPULR35902::OP_17() {
    T += 4;
    const uint8_t carry = static_cast<uint8_t>(getFlag(Flag::C));
    const bool msb = AF.left >> 7;
    setFlags(0, 0, 0, msb);
    AF.left = (AF.left << 1) | carry;
    if (m_debug) logInstruction("RLA");
}
void CPULR35902::OP_18() {
    T += 12;
    const auto relative = bus->read<uint8_t>(PC.w);
    PC.w++;
    PC.w += static_cast<int8_t>(relative);
    if (m_debug) logInstruction("JR $" + toHexString(relative));
}
void CPULR35902::OP_19() {
    T += 8;
    const bool half = (HL.right + DE.right) > 0xFF;
    const bool carry = (HL.w + DE.w) > 0xFFFF;
    HL.w += DE.w;
    setFlags(-1, 0, half, carry);
    if (m_debug) logInstruction("ADD HL, DE");
}
void CPULR35902::OP_1A() {
    T += 8;
    AF.left = bus->read<uint8_t>(DE.w);
    if (m_debug) logInstruction("LD A, (DE)");
}
void CPULR35902::OP_1B() {
    T += 8;
    DE.w--;
    if (m_debug) logInstruction("DEC DE");
}
void CPULR35902::OP_1C() {
    T += 4;
    const bool half = (DE.right == 0xF);
    DE.right++;
    setFlags((DE.right == 0), 0, half, -1);
    if (m_debug) logInstruction("INC E");
}
void CPULR35902::OP_1D() {
    T += 4;
    const bool half = (DE.right == 0x0);
    DE.right--;
    setFlags((DE.right == 0), 1, half, -1);
    if (m_debug) logInstruction("DEC E");
}
void CPULR35902::OP_1E() {
    T += 8;
    DE.right = bus->read<uint8_t>(PC.w);
    PC.w++;
    if (m_debug) logInstruction("LD E, $" + toHexString(DE.right));
}
void CPULR35902::OP_1F() {
    T += 4;
    const uint8_t carry = static_cast<uint8_t>(getFlag(Flag::C));
    const bool lsb = AF.left & 0x01;
    setFlags(0, 0, 0, lsb);
    AF.left = (AF.left >> 1) | (carry << 7);
    if (m_debug) logInstruction("RRA");
}
void CPULR35902::OP_20() { // JP NZ, e8
    const auto relative = bus->read<uint8_t>(PC.w);
    PC.w++;
    const bool zero = getFlag(Flag::Z); 
    if(zero) {
        T += 8;
    }
    else {
        T += 12;
        PC.w += static_cast<int8_t>(relative);
    }
    if (m_debug) logInstruction("JR NZ, $" + toHexString(relative));
}
void CPULR35902::OP_21() { // LD HL, n16
    T += 12;
    HL.w = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    if (m_debug) logInstruction("LD HL, $" + toHexString(HL.w));
}
void CPULR35902::OP_22() {
    T += 8;
    bus->write<uint8_t>(HL.w, AF.left);
    HL.w++;
    if (m_debug) logInstruction("LD (HL+), A");
}
void CPULR35902::OP_23() {
    T += 8;
    HL.w++;
    if (m_debug) logInstruction("INC HL");
}
void CPULR35902::OP_24() {
    T += 4;
    const bool half = (HL.left == 0xF);
    HL.left++;
    setFlags((HL.left == 0), 0, half, -1);
    if (m_debug) logInstruction("INC H");
}
void CPULR35902::OP_25() {
    T += 4;
    const bool half = (HL.left == 0x0);
    HL.left--;
    setFlags((HL.left == 0), 1, half, -1);
    if (m_debug) logInstruction("DEC H");
}
void CPULR35902::OP_26() {
    T += 8;
    HL.left = bus->read<uint8_t>(PC.w);
    PC.w++;
    if (m_debug) logInstruction("LD H, " + toHexString(HL.left));
}
void CPULR35902::OP_27() {
    T += 4;
    const uint8_t quo = AF.left / 100;
    const uint8_t rem = AF.left % 100;
    const uint8_t lsd = rem % 10;
    const uint8_t msd = rem / 10;
    AF.left = (msd << 4) | lsd;
    setFlags((AF.left == 0), -1, 0, (quo > 0));
    if (m_debug) logInstruction("DAA");
}
void CPULR35902::OP_28() {
    const auto relative = bus->read<uint8_t>(PC.w);
    PC.w++;
    const bool zero = getFlag(Flag::Z); 
    if(zero) {
        T += 12;
        PC.w += static_cast<int8_t>(relative);
    }
    else {
        T += 8;
    }
    if (m_debug) logInstruction("JR Z, $" + toHexString(relative));
}
void CPULR35902::OP_29() {
    T += 8;
    const bool half = (HL.right + HL.right) > 0xFF;
    const bool carry = (HL.w + HL.w) > 0xFFFF;
    HL.w += HL.w;
    setFlags(-1, 0, half, carry);
    if (m_debug) logInstruction("ADD HL, HL");
}
void CPULR35902::OP_2A() {
    T += 8;
    AF.left = bus->read<uint8_t>(HL.w);
    HL.w++;
    if (m_debug) logInstruction("LD A, (HL+)");
}
void CPULR35902::OP_2B() {
    T += 8;
    HL.w--;
    if (m_debug) logInstruction("DEC HL");
}
void CPULR35902::OP_2C() {
    T += 4;
    const bool half = (HL.right == 0xF);
    HL.right++;
    setFlags((HL.right == 0), 0, half, -1);
    if (m_debug) logInstruction("INC L");
}
void CPULR35902::OP_2D() {
    T += 4;
    const bool half = (HL.right == 0x0);
    HL.right--;
    setFlags((HL.right == 0), 1, half, -1);
    if (m_debug) logInstruction("DEC L");
}
void CPULR35902::OP_2E() {
    T += 8;
    HL.right = bus->read<uint8_t>(PC.w);
    PC.w++;
    if (m_debug) logInstruction("LD L, $" + toHexString(HL.right));
}
void CPULR35902::OP_2F() {
    T += 4;
    AF.left = ~AF.left;
    setFlags(-1, 1, 1, -1);
    if (m_debug) logInstruction("CPL");
}
void CPULR35902::OP_30() {
    const auto relative = bus->read<uint8_t>(PC.w);
    PC.w++;
    const bool carry = getFlag(Flag::C); 
    if(carry) {
        T += 8;
    }
    else {
        T += 12;
        PC.w += static_cast<int8_t>(relative);
    }
    if (m_debug) logInstruction("JR NC, $" + toHexString(relative));
}
void CPULR35902::OP_31() {
    T += 12;
    SP.w = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    if (m_debug) logInstruction("LD SP, $" + toHexString(SP.w));
}
void CPULR35902::OP_32() {
    T += 8;
    HL.w--;
    bus->write<uint8_t>(HL.w, AF.left);
    if (m_debug) logInstruction("LD (HL-), A");
}
void CPULR35902::OP_33() {
    T += 8;
    SP.w++;
    if (m_debug) logInstruction("INC SP");
}
void CPULR35902::OP_34() {
    T += 12;
    auto value = bus->read<uint16_t>(HL.w);
    const bool half = (value == 0xF);
    value++;
    bus->write<uint8_t>(HL.w, value);
    setFlags((value == 0), 0, half, -1);
    if (m_debug) logInstruction("INC (HL)");
}
void CPULR35902::OP_35() {
    T += 12;
    auto value = bus->read<uint16_t>(HL.w);
    const bool half = (value == 0x0);
    value--;
    bus->write<uint8_t>(HL.w, value);
    setFlags((value == 0), 1, half, -1);
    if (m_debug) logInstruction("DEC (HL)");
}
void CPULR35902::OP_36() {
    T += 12;
    const auto value  = bus->read<uint8_t>(PC.w);
    PC.w++;
    bus->write<uint8_t>(HL.w, value);
    if (m_debug) logInstruction("LD (HL), " + toHexString(BC.left));
}
void CPULR35902::OP_37() {
    T += 4;
    setFlags(-1, 0, 0, 1);
    if (m_debug) logInstruction("SCF");
}
void CPULR35902::OP_38() {
    const auto relative = bus->read<uint8_t>(PC.w);
    PC.w++;
    const bool carry = getFlag(Flag::C); 
    if(carry) {
        T += 12;
        PC.w += static_cast<int8_t>(relative);
    }
    else {
        T += 8;
    }
    if (m_debug) logInstruction("JR C, $" + toHexString(relative));
}
void CPULR35902::OP_39() {
    T += 8;
    const bool half = (HL.right + SP.right) > 0xFF;
    const bool carry = (HL.w + SP.w) > 0xFFFF;
    HL.w += SP.w;
    setFlags(-1, 0, half, carry);
    if (m_debug) logInstruction("ADD HL, SP");
}
void CPULR35902::OP_3A() {
    T += 8;
    HL.w--;
    AF.left = bus->read<uint8_t>(HL.w);
    if (m_debug) logInstruction("LD A, (HL-)");
}
void CPULR35902::OP_3B() {
    T += 8;
    SP.w--;
    if (m_debug) logInstruction("DEC SP");
}
void CPULR35902::OP_3C() {
    T += 4;
    const bool half = (AF.left == 0xF);
    AF.left++;
    setFlags((AF.left == 0), 0, half, -1);
    if (m_debug) logInstruction("INC A");
}
void CPULR35902::OP_3D() {
    T += 4;
    const bool half = (AF.left == 0x0);
    AF.left--;
    setFlags((AF.left == 0), 1, half, -1);
    if (m_debug) logInstruction("DEC A");
}
void CPULR35902::OP_3E() {
    T += 8;
    AF.left = bus->read<uint8_t>(PC.w);
    PC.w++;
    if (m_debug) logInstruction("LD A, $" + toHexString(AF.left));
}
void CPULR35902::OP_3F() {
    T += 4;
    setFlags(-1, 0, 0, 0);
    if (m_debug) logInstruction("CCF");
}
void CPULR35902::OP_40() {
    T += 4;
    if (m_debug) logInstruction("LD B, B");
}
void CPULR35902::OP_41() {
    T += 4;
    BC.left = BC.right;
    if (m_debug) logInstruction("LD B, C");
}
void CPULR35902::OP_42() {
    T += 4;
    BC.left = DE.left;
    if (m_debug) logInstruction("LD B, D");
}
void CPULR35902::OP_43() {
    T += 4;
    BC.left = DE.right;
    if (m_debug) logInstruction("LD B, E");
}
void CPULR35902::OP_44() {
    T += 4;
    BC.left = HL.left;
    if (m_debug) logInstruction("LD B, H");
}
void CPULR35902::OP_45() {
    T += 4;
    BC.left = HL.right;
    if (m_debug) logInstruction("LD B, L");
}
void CPULR35902::OP_46() {
    T += 8;
    BC.left = bus->read<uint8_t>(HL.w);
    if (m_debug) logInstruction("LD B, (HL)");
}
void CPULR35902::OP_47() {
    T += 4;
    BC.left = AF.left;
    if (m_debug) logInstruction("LD B, A");
}
void CPULR35902::OP_48() {
    T += 4;
    BC.right = BC.left;
    if (m_debug) logInstruction("LD C, B");
}
void CPULR35902::OP_49() {
    T += 4;
    if (m_debug) logInstruction("LD C, C");
}
void CPULR35902::OP_4A() {
    T += 4;
    BC.right = DE.left;
    if (m_debug) logInstruction("LD C, D");
}
void CPULR35902::OP_4B() {
    T += 4;
    BC.right = DE.right;
    if (m_debug) logInstruction("LD C, E");
}
void CPULR35902::OP_4C() {
    T += 4;
    BC.right = HL.left;
    if (m_debug) logInstruction("LD C, H");
}
void CPULR35902::OP_4D() {
    T += 4;
    BC.right = HL.right;
    if (m_debug) logInstruction("LD C, L");
}
void CPULR35902::OP_4E() {
    T += 8;
    BC.right = bus->read<uint8_t>(HL.w);
    if (m_debug) logInstruction("LD C, (HL)");
}
void CPULR35902::OP_4F() {
    T += 4;
    BC.right = AF.left;
    if (m_debug) logInstruction("LD C, A");
}
void CPULR35902::OP_50() {
    T += 4;
    DE.left = BC.left;
    if (m_debug) logInstruction("LD D, B");
}
void CPULR35902::OP_51() {
    T += 4;
    DE.left = BC.right;
    if (m_debug) logInstruction("LD D, C");
}
void CPULR35902::OP_52() {
    T += 4;
    if (m_debug) logInstruction("LD D, D");
}
void CPULR35902::OP_53() {
    T += 4;
    DE.left = DE.right;
    if (m_debug) logInstruction("LD D, E");
}
void CPULR35902::OP_54() {
    T += 4;
    DE.left = HL.left;
    if (m_debug) logInstruction("LD D, H");
}
void CPULR35902::OP_55() {
    T += 4;
    DE.left = HL.right;
    if (m_debug) logInstruction("LD D, L");
}
void CPULR35902::OP_56() {
    T += 8;
    DE.left = bus->read<uint8_t>(HL.w);
    if (m_debug) logInstruction("LD D, (HL)");
}
void CPULR35902::OP_57() {
    T += 4;
    DE.left = AF.left;
    if (m_debug) logInstruction("LD D, A");
}
void CPULR35902::OP_58() {
    T += 4;
    DE.right = BC.left;
    if (m_debug) logInstruction("LD E, B");
}
void CPULR35902::OP_59() {
    T += 4;
    DE.right = BC.right;
    if (m_debug) logInstruction("LD E, C");
}
void CPULR35902::OP_5A() {
    T += 4;
    DE.right = DE.left;
    if (m_debug) logInstruction("LD E, D");
}
void CPULR35902::OP_5B() {
    T += 4;
    if (m_debug) logInstruction("LD E, E");
}
void CPULR35902::OP_5C() {
    T += 4;
    DE.right = HL.left;
    if (m_debug) logInstruction("LD E, H");
}
void CPULR35902::OP_5D() {
    T += 4;
    DE.right = HL.right;
    if (m_debug) logInstruction("LD E, L");
}
void CPULR35902::OP_5E() {
    T += 8;
    DE.right = bus->read<uint8_t>(HL.w);
    if (m_debug) logInstruction("LD E, (HL)");
}
void CPULR35902::OP_5F() {
    T += 4;
    DE.right = AF.left;
    if (m_debug) logInstruction("LD E, A");
}
void CPULR35902::OP_60() {
    T += 4;
    HL.left = BC.left;
    if (m_debug) logInstruction("LD H, B");
}
void CPULR35902::OP_61() {
    T += 4;
    HL.left = BC.right;
    if (m_debug) logInstruction("LD H, C");
}
void CPULR35902::OP_62() {
    T += 4;
    HL.left = DE.left;
    if (m_debug) logInstruction("LD H, D");
}
void CPULR35902::OP_63() {
    T += 4;
    HL.left = DE.right;
    if (m_debug) logInstruction("LD H, E");
}
void CPULR35902::OP_64() {
    T += 4;
    if (m_debug) logInstruction("LD H, H");
}
void CPULR35902::OP_65() {
    T += 4;
    HL.left = HL.right;
    if (m_debug) logInstruction("LD H, L");
}
void CPULR35902::OP_66() {
    T += 8;
    HL.left = bus->read<uint8_t>(HL.w);
    if (m_debug) logInstruction("LD H, (HL)");
}
void CPULR35902::OP_67() {
    T += 4;
    HL.left = AF.left;
    if (m_debug) logInstruction("LD H, A");
}
void CPULR35902::OP_68() {
    T += 4;
    HL.right = BC.left;
    if (m_debug) logInstruction("LD L, B");
}
void CPULR35902::OP_69() {
    T += 4;
    HL.right = BC.right;
    if (m_debug) logInstruction("LD L, C");
}
void CPULR35902::OP_6A() {
    T += 4;
    BC.right = DE.left;
    if (m_debug) logInstruction("LD C, D");
}
void CPULR35902::OP_6B() {
    T += 4;
    HL.right = DE.right;
    if (m_debug) logInstruction("LD L, E");
}
void CPULR35902::OP_6C() {
    T += 4;
    HL.right = HL.left;
    if (m_debug) logInstruction("LD L, H");
}
void CPULR35902::OP_6D() {
    T += 4;
    if (m_debug) logInstruction("LD L, L");
}
void CPULR35902::OP_6E() {
    T += 8;
    HL.right = bus->read<uint8_t>(HL.w);
    if (m_debug) logInstruction("LD L, (HL)");
}
void CPULR35902::OP_6F() {
    T += 4;
    HL.right = AF.left;
    if (m_debug) logInstruction("LD L, A");
}
void CPULR35902::OP_70() {
    T += 8;
    bus->write<uint8_t>(HL.w, BC.left);
    if (m_debug) logInstruction("LD (HL), B");
}
void CPULR35902::OP_71() {
    T += 8;
    bus->write<uint8_t>(HL.w, BC.right);
    if (m_debug) logInstruction("LD (HL), C");
}
void CPULR35902::OP_72() {
    T += 8;
    bus->write<uint8_t>(HL.w, DE.left);
    if (m_debug) logInstruction("LD (HL), D");
}
void CPULR35902::OP_73() {
    T += 8;
    bus->write<uint8_t>(HL.w, DE.right);
    if (m_debug) logInstruction("LD (HL), E");
}
void CPULR35902::OP_74() {
    T += 8;
    bus->write<uint8_t>(HL.w, HL.left);
    if (m_debug) logInstruction("LD (HL), H");
}
void CPULR35902::OP_75() {
    T += 8;
    bus->write<uint8_t>(HL.w, HL.right);
    if (m_debug) logInstruction("LD (HL), L");
}
void CPULR35902::OP_76() {
    T += 4;
    halt = true;
    if (m_debug) logInstruction("HALT");
}
void CPULR35902::OP_77() {
    T += 8;
    bus->write<uint8_t>(HL.w, AF.left);
    if (m_debug) logInstruction("LD (HL), A");
}
void CPULR35902::OP_78() {
    T += 4;
    AF.left = BC.left;
    if (m_debug) logInstruction("LD A, B");
}
void CPULR35902::OP_79() {
    T += 4;
    AF.left = BC.right;
    if (m_debug) logInstruction("LD A, C");
}
void CPULR35902::OP_7A() {
    T += 4;
    AF.left = DE.left;
    if (m_debug) logInstruction("LD A, D");
}
void CPULR35902::OP_7B() {
    T += 4;
    AF.left = DE.right;
    if (m_debug) logInstruction("LD A, E");
}
void CPULR35902::OP_7C() {
    T += 4;
    AF.left = HL.left;
    if (m_debug) logInstruction("LD A, H");
}
void CPULR35902::OP_7D() {
    T += 4;
    AF.left = HL.right;
    if (m_debug) logInstruction("LD A, L");
}
void CPULR35902::OP_7E() {
    T += 8;
    AF.left = bus->read<uint8_t>(HL.w);
    if (m_debug) logInstruction("LD A, (HL)");
}
void CPULR35902::OP_7F() {
    T += 4;
    if (m_debug) logInstruction("LD A, A");
}
void CPULR35902::OP_80() {
    T += 4;
    const bool carry = (AF.left + BC.left) > 0xFF;
    const bool half = ((AF.left & 0xF) + (BC.left & 0xF)) > 0xF;
    AF.left += BC.left;
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADD A, B");
}
void CPULR35902::OP_81() {
    T += 4;
    const bool carry = (AF.left + BC.right) > 0xFF;
    const bool half = ((AF.left & 0xF) + (BC.right & 0xF)) > 0xF;
    AF.left += BC.right;
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADD A, C");
}
void CPULR35902::OP_82() {
    T += 4;
    const bool carry = (AF.left + DE.left) > 0xFF;
    const bool half = ((AF.left & 0xF) + (DE.left & 0xF)) > 0xF;
    AF.left += DE.left;
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADD A, D");
}
void CPULR35902::OP_83() {
    T += 4;
    const bool carry = (AF.left + DE.right) > 0xFF;
    const bool half = ((AF.left & 0xF) + (DE.right & 0xF)) > 0xF;
    AF.left += DE.right;
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADD A, E");
}
void CPULR35902::OP_84() {
    T += 4;
    const bool carry = (AF.left + HL.left) > 0xFF;
    const bool half = ((AF.left & 0xF) + (HL.left & 0xF)) > 0xF;
    AF.left += HL.left;
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADD A, H");
}
void CPULR35902::OP_85() {
    T += 4;
    const bool carry = (AF.left + HL.right) > 0xFF;
    const bool half = ((AF.left & 0xF) + (HL.right & 0xF)) > 0xF;
    AF.left += HL.right;
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADD A, L");
}
void CPULR35902::OP_86() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    const bool carry = (AF.left + value) > 0xFF;
    const bool half = ((AF.left & 0xF) + (value & 0xF)) > 0xF;
    AF.left += value;
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADD A, (HL)");
}
void CPULR35902::OP_87() {
    T += 4;
    const bool carry = (AF.left + AF.left) > 0xFF;
    const bool half = ((AF.left & 0xF) + (AF.left & 0xF)) > 0xF;
    AF.left += AF.left;
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADD A, A");
}
void CPULR35902::OP_88() {
    T += 4;
    const bool carry = (AF.left + BC.left + getFlag(Flag::C)) > 0xFF;
    const bool half = ((AF.left & 0xF) + (BC.left & 0xF) + getFlag(Flag::C)) > 0xF;
    AF.left += BC.left + getFlag(Flag::C);
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADC A, B");
}
void CPULR35902::OP_89() {
    T += 4;
    const bool carry = (AF.left + BC.right + getFlag(Flag::C)) > 0xFF;
    const bool half = ((AF.left & 0xF) + (BC.right & 0xF) + getFlag(Flag::C)) > 0xF;
    AF.left += BC.right + getFlag(Flag::C);
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADC A, C");
}
void CPULR35902::OP_8A() {
    T += 4;
    const bool carry = (AF.left + DE.left + getFlag(Flag::C)) > 0xFF;
    const bool half = ((AF.left & 0xF) + (DE.left & 0xF) + getFlag(Flag::C)) > 0xF;
    AF.left += DE.left + getFlag(Flag::C);
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADC A, D");
}
void CPULR35902::OP_8B() {
    T += 4;
    const bool carry = (AF.left + DE.right + getFlag(Flag::C)) > 0xFF;
    const bool half = ((AF.left & 0xF) + (DE.right & 0xF) + getFlag(Flag::C)) > 0xF;
    AF.left += DE.right + getFlag(Flag::C);
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADC A, E");
}
void CPULR35902::OP_8C() {
    T += 4;
    const bool carry = (AF.left + HL.left + getFlag(Flag::C)) > 0xFF;
    const bool half = ((AF.left & 0xF) + (HL.left & 0xF) + getFlag(Flag::C)) > 0xF;
    AF.left += HL.left + getFlag(Flag::C);
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADC A, H");
}
void CPULR35902::OP_8D() {
    T += 4;
    const bool carry = (AF.left + HL.right + getFlag(Flag::C)) > 0xFF;
    const bool half = ((AF.left & 0xF) + (HL.right & 0xF) + getFlag(Flag::C)) > 0xF;
    AF.left += HL.right + getFlag(Flag::C);
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADC A, L");
}
void CPULR35902::OP_8E() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    const bool carry = (AF.left + value + getFlag(Flag::C)) > 0xFF;
    const bool half = ((AF.left & 0xF) + (value & 0xF) + getFlag(Flag::C)) > 0xF;
    AF.left += value + getFlag(Flag::C);
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADC A, (HL)");
}
void CPULR35902::OP_8F() {
    T += 4;
    const bool carry = (AF.left + AF.left + getFlag(Flag::C)) > 0xFF;
    const bool half = ((AF.left & 0xF) + (AF.left & 0xF) + getFlag(Flag::C)) > 0xF;
    AF.left += AF.left + getFlag(Flag::C);
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADC A, A");
}
void CPULR35902::OP_90() {
    T += 4;
    const bool carry = AF.left < BC.left;
    const bool half = (AF.left & 0xF) < (BC.left & 0xF);
    AF.left -= BC.left;
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SUB A, B");
}
void CPULR35902::OP_91() {
    T += 4;
    const bool carry = AF.left < BC.right;
    const bool half = (AF.left & 0xF) < (BC.right & 0xF);
    AF.left -= BC.right;
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SUB A, C");
}
void CPULR35902::OP_92() {
    T += 4;
    const bool carry = AF.left < DE.left;
    const bool half = (AF.left & 0xF) < (DE.left & 0xF);
    AF.left -= DE.left;
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SUB A, D");
}
void CPULR35902::OP_93() {
    T += 4;
    const bool carry = AF.left < DE.right;
    const bool half = (AF.left & 0xF) < (DE.right & 0xF);
    AF.left -= DE.right;
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SUB A, E");
}
void CPULR35902::OP_94() {
    T += 4;
    const bool carry = AF.left < HL.left;
    const bool half = (AF.left & 0xF) < (HL.left & 0xF);
    AF.left -= HL.left;
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SUB A, H");
}
void CPULR35902::OP_95() {
    T += 4;
    const bool carry = AF.left < HL.right;
    const bool half = (AF.left & 0xF) < (HL.right & 0xF);
    AF.left -= HL.right;
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SUB A, L");
}
void CPULR35902::OP_96() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    const bool carry = AF.left < value;
    const bool half = (AF.left & 0xF) < (value & 0xF);
    AF.left -= value;
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SUB A, (HL)");
}
void CPULR35902::OP_97() {
    T += 4;
    AF.left = 0;
    setFlags(1, 1, 0, 0);
    if (m_debug) logInstruction("SUB A, A");
}
void CPULR35902::OP_98() {
    T += 4;
    const bool carry = AF.left < (BC.left + getFlag(Flag::C));
    const bool half = (AF.left & 0xF) < ((BC.left & 0xF) + getFlag(Flag::C));
    AF.left -= (BC.left + getFlag(Flag::C));
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SBC A, B");
}
void CPULR35902::OP_99() {
    T += 4;
    const bool carry = AF.left < (BC.right + getFlag(Flag::C));
    const bool half = (AF.left & 0xF) < ((BC.right & 0xF) + getFlag(Flag::C));
    AF.left -= (BC.right + getFlag(Flag::C));
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SBC A, C");
}
void CPULR35902::OP_9A() {
    T += 4;
    const bool carry = AF.left < (DE.left + getFlag(Flag::C));
    const bool half = (AF.left & 0xF) < ((DE.left & 0xF) + getFlag(Flag::C));
    AF.left -= (DE.left + getFlag(Flag::C));
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SBC A, D");
}
void CPULR35902::OP_9B() {
    T += 4;
    const bool carry = AF.left < (DE.right + getFlag(Flag::C));
    const bool half = (AF.left & 0xF) < ((DE.right & 0xF) + getFlag(Flag::C));
    AF.left -= (DE.right + getFlag(Flag::C));
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SBC A, E");
}
void CPULR35902::OP_9C() {
    T += 4;
    const bool carry = AF.left < (HL.left + getFlag(Flag::C));
    const bool half = (AF.left & 0xF) < ((HL.left & 0xF) + getFlag(Flag::C));
    AF.left -= (HL.left + getFlag(Flag::C));
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SBC A, H");
}
void CPULR35902::OP_9D() {
    T += 4;
    const bool carry = AF.left < (HL.right + getFlag(Flag::C));
    const bool half = (AF.left & 0xF) < ((HL.right & 0xF) + getFlag(Flag::C));
    AF.left -= (HL.right + getFlag(Flag::C));
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SBC A, L");
}
void CPULR35902::OP_9E() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    const bool carry = AF.left < (value + getFlag(Flag::C));
    const bool half = (AF.left & 0xF) < ((value & 0xF) + getFlag(Flag::C));
    AF.left -= (value + getFlag(Flag::C));
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SBC A, (HL)");
}
void CPULR35902::OP_9F() {
    T += 4;
    const bool half = getFlag(Flag::C);
    AF.left = -getFlag(Flag::C);
    setFlags((AF.left == 0), 1, half, -1);
    if (m_debug) logInstruction("SBC A, A");
}
void CPULR35902::OP_A0() {
    T += 4;
    AF.left &= BC.left;
    setFlags((AF.left == 0), 0, 1, 0);
    if (m_debug) logInstruction("AND A, B");
}
void CPULR35902::OP_A1() {
    T += 4;
    AF.left &= BC.right;
    setFlags((AF.left == 0), 0, 1, 0);
    if (m_debug) logInstruction("AND A, C");
}
void CPULR35902::OP_A2() {
    T += 4;
    AF.left &= DE.left;
    setFlags((AF.left == 0), 0, 1, 0);
    if (m_debug) logInstruction("AND A, D");
}
void CPULR35902::OP_A3() {
    T += 4;
    AF.left &= DE.right;
    setFlags((AF.left == 0), 0, 1, 0);
    if (m_debug) logInstruction("AND A, E");
}
void CPULR35902::OP_A4() {
    T += 4;
    AF.left &= HL.left;
    setFlags((AF.left == 0), 0, 1, 0);
    if (m_debug) logInstruction("AND A, H");
}
void CPULR35902::OP_A5() {
    T += 4;
    AF.left &= HL.right;
    setFlags((AF.left == 0), 0, 1, 0);
    if (m_debug) logInstruction("AND A, L");
}
void CPULR35902::OP_A6() {
    T += 8;
    AF.left &= bus->read<uint8_t>(HL.w);
    setFlags((AF.left == 0), 0, 1, 0);
    if (m_debug) logInstruction("AND A, (HL)");
}
void CPULR35902::OP_A7() {
    T += 4;
    setFlags((AF.left == 0), 0, 1, 0);
    if (m_debug) logInstruction("AND A, A");
}
void CPULR35902::OP_A8() {
    T += 4;
    AF.left ^= BC.left;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("XOR A, B");
}
void CPULR35902::OP_A9() {    
    T += 4;
    AF.left ^= BC.right;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("XOR A, C");
}
void CPULR35902::OP_AA() {
    T += 4;
    AF.left ^= DE.left;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("XOR A, D");
}
void CPULR35902::OP_AB() {
    T += 4;
    AF.left ^= DE.right;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("XOR A, E");
}
void CPULR35902::OP_AC() {
    T += 4;
    AF.left ^= HL.left;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("XOR A, H");
}
void CPULR35902::OP_AD() {
    T += 4;
    AF.left ^= HL.right;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("XOR A, L");
}
void CPULR35902::OP_AE() {
    T += 8;
    AF.left ^= bus->read<uint8_t>(HL.w);
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("XOR A, (HL)");
}
void CPULR35902::OP_AF() {
    T += 4;
    AF.left ^= AF.left;
    setFlags(1, 0, 0, 0);
    if (m_debug) logInstruction("XOR A, A");
}
void CPULR35902::OP_B0() {
    T += 4;
    AF.left |= BC.left;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("OR A, B");
}
void CPULR35902::OP_B1() {
    T += 4;
    AF.left |= BC.right;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("OR A, C");
}
void CPULR35902::OP_B2() {
    T += 4;
    AF.left |= DE.left;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("OR A, D");
}
void CPULR35902::OP_B3() {
    T += 4;
    AF.left |= DE.right;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("OR A, E");
}
void CPULR35902::OP_B4() {
    T += 4;
    AF.left |= HL.left;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("OR A, H");
}
void CPULR35902::OP_B5() {
    T += 4;
    AF.left |= HL.right;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("OR A, L");
}
void CPULR35902::OP_B6() {
    T += 8;
    AF.left |= bus->read<uint8_t>(HL.w);
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("OR A, (HL)");
}
void CPULR35902::OP_B7() {
    T += 4;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("OR A, A");
}
void CPULR35902::OP_B8() {
    T += 4;
    const bool carry = AF.left < BC.left;
    const bool half = (AF.left & 0xF) < (BC.left & 0xF);
    setFlags((AF.left == BC.left), 1, half, carry);
    if (m_debug) logInstruction("CP A, B");
}
void CPULR35902::OP_B9() {
    T += 4;
    const bool carry = AF.left < BC.right;
    const bool half = (AF.left & 0xF) < (BC.right & 0xF);
    setFlags((AF.left == BC.right), 1, half, carry);
    if (m_debug) logInstruction("CP A, C");
}
void CPULR35902::OP_BA() {
    T += 4;
    const bool carry = AF.left < DE.left;
    const bool half = (AF.left & 0xF) < (DE.left & 0xF);
    setFlags((AF.left == DE.left), 1, half, carry);
    if (m_debug) logInstruction("CP A, D");
}
void CPULR35902::OP_BB() {
    T += 4;
    const bool carry = AF.left < DE.right;
    const bool half = (AF.left & 0xF) < (DE.right & 0xF);
    setFlags((AF.left == DE.right), 1, half, carry);
    if (m_debug) logInstruction("CP A, E");
}
void CPULR35902::OP_BC() {
    T += 4;
    const bool carry = AF.left < HL.left;
    const bool half = (AF.left & 0xF) < (HL.left & 0xF);
    setFlags((AF.left == HL.left), 1, half, carry);
    if (m_debug) logInstruction("CP A, H");
}
void CPULR35902::OP_BD() {
    T += 4;
    const bool carry = AF.left < HL.right;
    const bool half = (AF.left & 0xF) < (HL.right & 0xF);
    setFlags((AF.left == HL.right), 1, half, carry);
    if (m_debug) logInstruction("CP A, L");
}
void CPULR35902::OP_BE() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w); 
    const bool carry = AF.left < value;
    const bool half = (AF.left & 0xF) < (value & 0xF);
    setFlags((AF.left == value), 1, half, carry);
    if (m_debug) logInstruction("CP A, (HL)");
}
void CPULR35902::OP_BF() {
    T += 4;
    setFlags(1, 1, 0, 0);
    if (m_debug) logInstruction("CP A, A");
}
void CPULR35902::OP_C0() {
    const bool zero = getFlag(Flag::Z);
    if(zero) {
        T += 8;
    }
    else {
        T += 20;
        PC.w = bus->read<uint16_t>(SP.w);
        SP.w++;
    }
    if (m_debug) logInstruction("RET NZ");
}
void CPULR35902::OP_C1() {
    T += 12;
    BC.w = bus->read<uint16_t>(SP.w);
    SP.w += 2;
    if (m_debug) logInstruction("POP BC");
}
void CPULR35902::OP_C2() {
    const auto addr = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    const bool zero = getFlag(Flag::Z);
    if(zero) {
        T += 12;
    }
    else {
        T += 16;
        PC.w = addr;
    }
    if (m_debug) logInstruction("JP NZ, $" + toHexString(addr));
}
void CPULR35902::OP_C3() {  
    T += 16;
    PC.w = bus->read<uint16_t>(PC.w);
    if (m_debug) logInstruction("JP, $" + toHexString(PC.w));
}
void CPULR35902::OP_C4() {
    const auto addr = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    const bool zero = getFlag(Flag::Z);
    if(zero) {
        T += 12;
    }
    else {
        T += 24;
        SP.w -= 2;
        bus->write<uint16_t>(SP.w, PC.w);
        PC.w = addr;
    }
    if (m_debug) logInstruction("CALL NZ, $" + toHexString(addr));
}
void CPULR35902::OP_C5() {
    T += 16;
    SP.w -= 2;
    bus->write<uint16_t>(SP.w, BC.w);
    if (m_debug) logInstruction("PUSH BC");
}
void CPULR35902::OP_C6() {
    T += 8;
    const auto value = bus->read<uint8_t>(PC.w);
    PC.w++;
    const bool carry = (AF.left + value) > 0xFF;
    const bool half = ((AF.left & 0xF) + (value & 0xF)) > 0xF;
    AF.left += value;
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADD A, $" + toHexString(value));
}
void CPULR35902::OP_C7() {
    T += 16;
    SP.w -= 2;
    bus->write<uint16_t>(SP.w, PC.w);
    PC.w = 0x00;
    if (m_debug) logInstruction("RST $00");
}
void CPULR35902::OP_C8() {
    const bool zero = getFlag(Flag::Z);
    if(zero) {
        T += 20;
        PC.w = bus->read<uint16_t>(SP.w);
        SP.w += 2;
    }
    else {
        T += 8;
    }
    if (m_debug) logInstruction("RET Z");
}
void CPULR35902::OP_C9() {
    T += 16;
    PC.w = bus->read<uint16_t>(SP.w);
    SP.w += 2;
    if (m_debug) logInstruction("RET");
}
void CPULR35902::OP_CA() {
    const auto addr = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    const bool zero = getFlag(Flag::Z);
    if(zero) {
        T += 16;
        PC.w = addr;
    }
    else {
        T += 12;
    }
    if (m_debug) logInstruction("JP Z, $" + toHexString(addr));
}
void CPULR35902::OP_CB() {
    T += 4;
    if (m_debug) logInstruction("PREFIX");
}
void CPULR35902::OP_CC() {
    const auto addr = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    const bool zero = getFlag(Flag::Z);
    if(zero) {
        T += 24;
        SP.w -= 2;
        bus->write<uint16_t>(SP.w, PC.w);
        PC.w = addr;
    }
    else {
        T += 12;    
    }
    if (m_debug) logInstruction("CALL Z, $" + toHexString(addr));
}
void CPULR35902::OP_CD() {
    T += 24;
    const auto addr = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    SP.w -= 2;
    bus->write<uint16_t>(SP.w, PC.w);
    PC.w = addr;
    if (m_debug) logInstruction("CALL, $" + toHexString(addr));
}
void CPULR35902::OP_CE() {
    T += 8;
    const auto value = bus->read<uint8_t>(PC.w);
    PC.w++;
    const bool carry = (AF.left + value + getFlag(Flag::C)) > 0xFF;
    const bool half = ((AF.left & 0xF) + (value & 0xF) + getFlag(Flag::C)) > 0xF;
    AF.left += value + getFlag(Flag::C);
    setFlags((AF.left == 0), 0, half, carry);
    if (m_debug) logInstruction("ADC A, $" + toHexString(value));
}
void CPULR35902::OP_CF() {
    T += 16;
    SP.w -= 2;
    bus->write<uint16_t>(SP.w, PC.w);
    PC.w = 0x08;
    if (m_debug) logInstruction("RST $08");
}
void CPULR35902::OP_D0() {
    const bool carry = getFlag(Flag::C);
    if(carry) {
        T += 8;
    }
    else {
        T += 20;
        PC.w = bus->read<uint16_t>(SP.w);
        SP.w++;
    }
    if (m_debug) logInstruction("RET NC");
}
void CPULR35902::OP_D1() {
    T += 12;
    DE.w = bus->read<uint16_t>(SP.w);
    SP.w += 2;
    if (m_debug) logInstruction("POP DE");
}
void CPULR35902::OP_D2() {
    const auto addr = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    const bool carry = getFlag(Flag::C);
    if(carry) {
        T += 12;
    }
    else {
        T += 16;
        PC.w = addr;
    }
    if (m_debug) logInstruction("RET NZ, $" + toHexString(addr));
}
void CPULR35902::OP_D3() {
    throw std::runtime_error("Illegal instruction!");
}
void CPULR35902::OP_D4() {
    const auto addr = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    const bool carry = getFlag(Flag::C);
    if(carry) {
        T += 12;
    }
    else {
        T += 24;
        SP.w -= 2;
        bus->write<uint16_t>(SP.w, PC.w);
        PC.w = addr;
    }
    if (m_debug) logInstruction("CALL NC, $" + toHexString(addr));
}
void CPULR35902::OP_D5() {
    T += 16;
    SP.w -= 2;
    bus->write<uint16_t>(SP.w, DE.w);
    if (m_debug) logInstruction("PUSH DE");
}
void CPULR35902::OP_D6() {
    T += 8;
    const auto value = bus->read<uint8_t>(PC.w);
    PC.w++;
    const bool carry = AF.left < value; 
    const bool half = (AF.left & 0xF) < (value & 0xF);
    AF.left -= value; 
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SUB A, $" + toHexString(value));
}
void CPULR35902::OP_D7() {
    T += 16;
    SP.w -= 2;
    bus->write<uint16_t>(SP.w, PC.w);
    PC.w = 0x10;
    if (m_debug) logInstruction("RST $10");
}
void CPULR35902::OP_D8() {
    const bool carry = getFlag(Flag::C);
    if(carry) {
        T += 20;
        PC.w = bus->read<uint16_t>(SP.w);
        SP.w += 2;
    }
    else {
        T += 8;
    }
    if (m_debug) logInstruction("RET C");
}
void CPULR35902::OP_D9() {
    T += 16;
    PC.w = bus->read<uint16_t>(SP.w);
    SP.w += 2;
    interruptMasterEnable = true;
    if (m_debug) logInstruction("RETI");
}
void CPULR35902::OP_DA() {
    const auto addr = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    const bool carry = getFlag(Flag::C);
    if(carry) {
        T += 16;
        PC.w = addr;
    }
    else {
        T += 12;
    }
    if (m_debug) logInstruction("JP C, $" + toHexString(addr));
}
void CPULR35902::OP_DB() {
    throw std::runtime_error("Illegal instruction!");    
}
void CPULR35902::OP_DC() {
    const auto addr = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    const bool carry = getFlag(Flag::C);
    if(carry) {
        T += 24;
        SP.w -= 2;
        bus->write<uint16_t>(SP.w, PC.w);
        PC.w = addr;
    }
    else {
        T += 12;
    }
    if (m_debug) logInstruction("CALL C, $" + toHexString(addr));
}
void CPULR35902::OP_DD() {
    throw std::runtime_error("Illegal instruction!");
}
void CPULR35902::OP_DE() {
    T += 8;
    const auto value = bus->read<uint8_t>(PC.w);
    PC.w++;
    const bool carry = AF.left < (value + getFlag(Flag::C));
    const bool half = (AF.left & 0xF) < ((value & 0xF) + getFlag(Flag::C));
    AF.left -= (value + getFlag(Flag::C));
    setFlags((AF.left == 0), 1, half, carry);
    if (m_debug) logInstruction("SBC A, $" + toHexString(value));
}
void CPULR35902::OP_DF() {
    T += 16;
    SP.w -= 2;
    bus->write<uint16_t>(SP.w, PC.w);
    PC.w = 0x18;
    if (m_debug) logInstruction("RST $18");
}
void CPULR35902::OP_E0() { 
    const auto value = bus->read<uint8_t>(PC.w);
    PC.w++;
    bus->write<uint8_t>(0xFF00 + value, AF.left);
    if (m_debug) logInstruction("LDH ($FF00+$" + toHexString(value) + "), A");
}
void CPULR35902::OP_E1() {
    T += 12;
    HL.w = bus->read<uint16_t>(SP.w);
    SP.w += 2;
    if (m_debug) logInstruction("POP HL");
}
void CPULR35902::OP_E2() {
    T += 8;
    bus->write<uint8_t>(0xFF00 + BC.right, AF.left);
    if (m_debug) logInstruction("LD (SFF00+C), A");
}
void CPULR35902::OP_E3() {
    throw std::runtime_error("Illegal instruction!");
}
void CPULR35902::OP_E4() {
    throw std::runtime_error("Illegal instruction!");
}
void CPULR35902::OP_E5() {
    T += 16;
    SP.w -= 2;
    bus->write<uint16_t>(SP.w, HL.w);
    if (m_debug) logInstruction("PUSH HL");
}
void CPULR35902::OP_E6() {
    T += 8;
    const auto value = bus->read<uint8_t>(PC.w);
    PC.w++;
    AF.left &= value;
    setFlags((AF.left == 0), 0, 1, 0);
    if (m_debug) logInstruction("AND A, $" + toHexString(value));
}
void CPULR35902::OP_E7() {
    T += 16;
    SP.w -= 2;
    bus->write<uint16_t>(SP.w, PC.w);
    PC.w = 0x20;
    if (m_debug) logInstruction("RST $20");
}
void CPULR35902::OP_E8() {
    T+=16;
    const auto value = bus->read<uint8_t>(PC.w);
    PC.w++;
    const bool carry = (SP.right + value) > 0xFF;   
    const bool half = ((SP.w & 0xF) + (value & 0xF)) > 0xF;
    SP.w += value;
    setFlags(0, 0, half, carry);
    if (m_debug) logInstruction("ADD SP, $" + toHexString(value));
}
void CPULR35902::OP_E9() {
    T += 4;
    PC.w = HL.w;
    if (m_debug) logInstruction("JP HL");
}
void CPULR35902::OP_EA() {
    T += 16;
    const auto addr = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    bus->write<uint8_t>(addr, AF.left);
    if (m_debug) logInstruction("LD ($" + toHexString(addr) + "), A");
}
void CPULR35902::OP_EB() {
    throw std::runtime_error("Illegal instruction!");
}
void CPULR35902::OP_EC() {
    throw std::runtime_error("Illegal instruction!");
}
void CPULR35902::OP_ED() {
    //throw std::runtime_error("Illegal instruction!");
    T += 4;
    if (m_debug) logInstruction("Illegal instruction");
}
void CPULR35902::OP_EE() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    AF.left ^= value;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("XOR A, $" + toHexString(value));
}
void CPULR35902::OP_EF() {
    T += 16;
    SP.w -= 2;
    bus->write<uint16_t>(SP.w, PC.w);
    PC.w = 0x28;
    if (m_debug) logInstruction("RST $28");
}
void CPULR35902::OP_F0() {
    T += 12;
    const auto value = bus->read<uint8_t>(PC.w);
    PC.w++;
    AF.left = bus->read<uint8_t>(0xFF00 + value);
    if (m_debug) logInstruction("LDH A, ($FF00+" + toHexString(value) + ")");
}
void CPULR35902::OP_F1() {
    T += 12;
    AF.w = bus->read<uint16_t>(SP.w);
    SP.w += 2;
    if (m_debug) logInstruction("POP AF");
}
void CPULR35902::OP_F2() {
    T += 8;
    AF.left = bus->read<uint8_t>(0xFF00 + BC.right);
    if (m_debug) logInstruction("LD A, (SFF00+C)");
}
void CPULR35902::OP_F3() {
    interruptMasterEnable = false;
    if (m_debug) logInstruction("DI");
}
void CPULR35902::OP_F4() {
    throw std::runtime_error("Illegal instruction!");
}
void CPULR35902::OP_F5() {
    T += 16;
    SP.w -= 2;
    bus->write<uint16_t>(SP.w, AF.w);
    if (m_debug) logInstruction("PUSH AF");
}
void CPULR35902::OP_F6() {
    T += 8;
    const auto value = bus->read<uint8_t>(PC.w);
    PC.w++;
    AF.left |= value;
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("OR A, $" + toHexString(value));
}
void CPULR35902::OP_F7() {
    T += 16;
    SP.w -= 2;
    bus->write<uint16_t>(SP.w, PC.w);
    PC.w = 0x30;
    if (m_debug) logInstruction("RST $30");
}
void CPULR35902::OP_F8() {
    T+=12;
    const auto value = bus->read<uint8_t>(PC.w);
    PC.w++;
    const bool carry = (SP.right + value) > 0xFF;
    const bool half = ((SP.w & 0xF) + (value & 0xF)) > 0xF;
    HL.w = SP.w + value;
    setFlags(0, 0, half, carry);
    if (m_debug) logInstruction("LD HL, SP + $" + toHexString(value));
}
void CPULR35902::OP_F9() {
    T += 8;
    SP.w = HL.w;
    if (m_debug) logInstruction("LD SP, HL");
}
void CPULR35902::OP_FA() {
    T += 16;
    const auto addr = bus->read<uint16_t>(PC.w);
    PC.w += 2;
    AF.left = bus->read<uint8_t>(addr);
    if (m_debug) logInstruction("LD A, ($" + toHexString(addr) + ")");
}
void CPULR35902::OP_FB() {
    interruptMasterEnable = true;
    if (m_debug) logInstruction("EI");
}
void CPULR35902::OP_FC() {
    throw std::runtime_error("Illegal instruction!");
}
void CPULR35902::OP_FD() {
    throw std::runtime_error("Illegal instruction!");
}
void CPULR35902::OP_FE() {
    T += 8;
    const auto value = bus->read<uint8_t>(PC.w);
    PC.w++;
    const bool carry = AF.left < value;
    const bool half = (AF.left & 0xF) < (value & 0xF);
    setFlags((AF.left == value), 1, half, carry);
    if (m_debug) logInstruction("CP A, $" + toHexString(value));
}
void CPULR35902::OP_FF() {
    T += 16;
    SP.w -= 2;
    bus->write<uint16_t>(SP.w, PC.w);
    PC.w = 0x00;
    if (m_debug) logInstruction("RST $38");
}
void CPULR35902::PR_00() {
    T += 8;
    const auto msb = (BC.left & 0b10000000) >> 7;
    BC.left = (BC.left << 1) | msb;
    setFlags((BC.left == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC B");
}
void CPULR35902::PR_01() {
    T += 8;
    const auto msb = (BC.right & 0b10000000) >> 7;
    BC.right = (BC.right << 1) | msb;
    setFlags((BC.right == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC C");
}
void CPULR35902::PR_02() {
    T += 8;
    const auto msb = (DE.left & 0b10000000) >> 7;
    DE.left = (DE.left << 1) | msb;
    setFlags((DE.left == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC D");
}
void CPULR35902::PR_03() {
    T += 8;
    const auto msb = (DE.right & 0b10000000) >> 7;
    DE.right = (DE.right << 1) | msb;
    setFlags((DE.right == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC E");
}
void CPULR35902::PR_04() {
    T += 8;
    const auto msb = (HL.left & 0b10000000) >> 7;
    HL.left = (HL.left << 1) | msb;
    setFlags((HL.left == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC H");
}
void CPULR35902::PR_05() {
    T += 8;
    const auto msb = (HL.right & 0b10000000) >> 7;
    HL.right = (HL.right << 1) | msb;
    setFlags((HL.right == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC L");
}
void CPULR35902::PR_06() {
    T += 16;
    auto value = bus->read<uint8_t>(HL.w);
    const auto msb = (value & 0b10000000) >> 7;
    value = (value << 1) | msb;
    setFlags((value == 0), 0, 0, msb);
    bus->write<uint8_t>(HL.w, value);
    if (m_debug) logInstruction("RLC (HL)");
}
void CPULR35902::PR_07() {
    T += 8;
    const auto msb = (AF.left & 0b10000000) >> 7;
    AF.left = (AF.left << 1) | msb;
    setFlags((AF.left == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC A");
}
void CPULR35902::PR_08() {
    T += 8;
    const auto lsb = (BC.left & 0b00000001);
    BC.left = (BC.left >> 1) | (lsb << 7);
    setFlags((BC.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC B");
}
void CPULR35902::PR_09() {
    T += 8;
    const auto lsb = (BC.right & 0b00000001);
    BC.right = (BC.right >> 1) | (lsb << 7);
    setFlags((BC.right == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC C");
}
void CPULR35902::PR_0A() {
    T += 8;
    const auto lsb = (DE.left & 0b00000001);
    DE.left = (DE.left >> 1) | (lsb << 7);
    setFlags((DE.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC D");
}
void CPULR35902::PR_0B() {
    T += 8;
    const auto lsb = (DE.right & 0b00000001);
    DE.right = (DE.right >> 1) | (lsb << 7);
    setFlags((DE.right == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC E");
}
void CPULR35902::PR_0C() {
    T += 8;
    const auto lsb = (HL.left & 0b00000001);
    HL.left = (HL.left >> 1) | (lsb << 7);
    setFlags((HL.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC H");
}
void CPULR35902::PR_0D() {
    T += 8;
    const auto lsb = (HL.right & 0b00000001);
    HL.right = (HL.right >> 1) | (lsb << 7);
    setFlags((HL.right == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC L");
}
void CPULR35902::PR_0E() {
    T += 16;
    auto value = bus->read<uint8_t>(HL.w);
    const auto lsb = (value & 0b00000001);
    value = (value >> 1) | (lsb << 7);
    setFlags((value == 0), 0, 0, lsb);
    bus->write<uint8_t>(HL.w, value);
    if (m_debug) logInstruction("RRC (HL)");
}
void CPULR35902::PR_0F() {
    T += 8;
    const auto lsb = (AF.left & 0b00000001);
    AF.left = (AF.left >> 1) | (lsb << 7);
    setFlags((AF.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC A");
}
void CPULR35902::PR_10() {
    T += 8;
    const auto msb = (BC.left & 0b10000000) >> 7;
    BC.left = (BC.left << 1) | getFlag(Flag::C);
    setFlags((BC.left == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC B");
}
void CPULR35902::PR_11() {
    T += 8;
    const auto msb = (BC.right & 0b10000000) >> 7;
    BC.right = (BC.right << 1) | getFlag(Flag::C);
    setFlags((BC.right == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC C");
}
void CPULR35902::PR_12() {
    T += 8;
    const auto msb = (DE.left & 0b10000000) >> 7;
    DE.left = (DE.left << 1) | getFlag(Flag::C);
    setFlags((DE.left == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC D");
}
void CPULR35902::PR_13() {
    T += 8;
    const auto msb = (DE.right & 0b10000000) >> 7;
    DE.right = (DE.right << 1) | getFlag(Flag::C);
    setFlags((DE.right == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC E");
}
void CPULR35902::PR_14() {
    T += 8;
    const auto msb = (HL.left & 0b10000000) >> 7;
    HL.left = (HL.left << 1) | getFlag(Flag::C);
    setFlags((HL.left == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC H");
}
void CPULR35902::PR_15() {
    T += 8;
    const auto msb = (HL.right & 0b10000000) >> 7;
    HL.right = (HL.right << 1) | getFlag(Flag::C);
    setFlags((HL.right == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC L");
}
void CPULR35902::PR_16() {
    T += 16;
    auto value = bus->read<uint8_t>(HL.w);
    const auto msb = (value & 0b10000000) >> 7;
    value = (value << 1) | getFlag(Flag::C);
    setFlags((value == 0), 0, 0, msb);
    bus->write<uint8_t>(HL.w, value);
    if (m_debug) logInstruction("RLC (HL)");
}
void CPULR35902::PR_17() {
    T += 8;
    const auto msb = (AF.left & 0b10000000) >> 7;
    AF.left = (AF.left << 1) | getFlag(Flag::C);
    setFlags((AF.left == 0), 0, 0, msb);
    if (m_debug) logInstruction("RLC A");
}
void CPULR35902::PR_18() {
    T += 8;
    const auto lsb = (BC.left & 0b00000001);
    BC.left = (BC.left >> 1) | (getFlag(Flag::C) << 7);
    setFlags((BC.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC B");
}
void CPULR35902::PR_19() {
    T += 8;
    const auto lsb = (BC.right & 0b00000001);
    BC.right = (BC.right >> 1) | (getFlag(Flag::C) << 7);
    setFlags((BC.right == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC C");
}
void CPULR35902::PR_1A() {
    T += 8;
    const auto lsb = (DE.left & 0b00000001);
    DE.left = (DE.left >> 1) | (getFlag(Flag::C) << 7);
    setFlags((DE.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC D");
}
void CPULR35902::PR_1B() {
    T += 8;
    const auto lsb = (DE.right & 0b00000001);
    DE.right = (DE.right >> 1) | (getFlag(Flag::C) << 7);
    setFlags((DE.right == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC E");
}
void CPULR35902::PR_1C() {
    T += 8;
    const auto lsb = (HL.left & 0b00000001);
    HL.left = (HL.left >> 1) | (getFlag(Flag::C) << 7);
    setFlags((HL.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC H");
}
void CPULR35902::PR_1D() {
    T += 8;
    const auto lsb = (HL.right & 0b00000001);
    HL.right = (HL.right >> 1) | (getFlag(Flag::C) << 7);
    setFlags((HL.right == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC L");
}
void CPULR35902::PR_1E() {
    T += 16;
    auto value = bus->read<uint8_t>(HL.w);
    const auto lsb = (value & 0b00000001);
    value = (value >> 1) | (getFlag(Flag::C) << 7);
    setFlags((value == 0), 0, 0, lsb);
    bus->write<uint8_t>(HL.w, value);
    if (m_debug) logInstruction("RRC (HL)");
}
void CPULR35902::PR_1F() {
    T += 8;
    const auto lsb = (AF.left & 0b00000001);
    AF.left = (AF.left >> 1) | (getFlag(Flag::C) << 7);
    setFlags((AF.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("RRC A");
}
void CPULR35902::PR_20() {
    T += 8;
    const auto msb = BC.left & 0b10000000;
    BC.left <<= 1;
    BC.left | msb;
    setFlags((BC.left == 0), 0, 0, msb);
    if (m_debug) logInstruction("SLA B");
}
void CPULR35902::PR_21() {
    T += 8;
    const auto msb = BC.right & 0b10000000;
    BC.right <<= 1;
    BC.right | msb;
    setFlags((BC.right == 0), 0, 0, msb);
    if (m_debug) logInstruction("SLA C");
}
void CPULR35902::PR_22() {
    T += 8;
    const auto msb = DE.left & 0b10000000;
    DE.left <<= 1;
    DE.left | msb;
    setFlags((DE.left == 0), 0, 0, msb);
    if (m_debug) logInstruction("SLA D");
}
void CPULR35902::PR_23() {
    T += 8;
    const auto msb = DE.right & 0b10000000;
    DE.right <<= 1;
    DE.right | msb;
    setFlags((DE.right == 0), 0, 0, msb);
    if (m_debug) logInstruction("SLA E");
}
void CPULR35902::PR_24() {
    T += 8;
    const auto msb = HL.left & 0b10000000;
    HL.left <<= 1;
    HL.left | msb;
    setFlags((HL.left == 0), 0, 0, msb);
    if (m_debug) logInstruction("SLA H");
}
void CPULR35902::PR_25() {
    T += 8;
    const auto msb = HL.right & 0b10000000;
    HL.right <<= 1;
    HL.right | msb;
    setFlags((HL.right == 0), 0, 0, msb);
    if (m_debug) logInstruction("SLA L");
}
void CPULR35902::PR_26() {
    T += 16;
    auto value = bus->read<uint8_t>(HL.w);
    const auto msb = value & 0b10000000;
    value <<= 1;
    value | msb;
    setFlags((value == 0), 0, 0, msb);
    bus->write<uint8_t>(HL.w, value);
    if (m_debug) logInstruction("SLA (HL)");
}
void CPULR35902::PR_27() {
    T += 8;
    const auto msb = AF.left & 0b10000000;
    AF.left <<= 1;
    AF.left | msb;
    setFlags((AF.left == 0), 0, 0, msb);
    if (m_debug) logInstruction("SLA A");
}
void CPULR35902::PR_28() {
    T += 8;
    const auto msb = BC.left & 0b10000000;
    const auto lsb = BC.left & 0b00000001;
    BC.left >>= 1;
    BC.left |= msb;
    setFlags((BC.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRA B");
}
void CPULR35902::PR_29() {
    T += 8;
    const auto msb = BC.left & 0b10000000;
    const auto lsb = BC.right & 0b00000001;
    BC.right >>= 1;
    BC.right | msb;
    setFlags((BC.right == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRA C");
}
void CPULR35902::PR_2A() {
    T += 8;
    const auto msb = BC.left & 0b10000000;
    const auto lsb = DE.left & 0b00000001;
    DE.left >>= 1;
    DE.left | msb;
    setFlags((DE.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRA D");
}
void CPULR35902::PR_2B() {
    T += 8;
    const auto msb = BC.left & 0b10000000;
    const auto lsb = DE.right & 0b00000001;
    DE.right >>= 1;
    DE.right | msb;
    setFlags((DE.right == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRA E");
}
void CPULR35902::PR_2C() {
    T += 8;
    const auto msb = BC.left & 0b10000000;
    const auto lsb = HL.left & 0b00000001;
    HL.left >>= 1;
    HL.left | msb;
    setFlags((HL.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRA H");
}
void CPULR35902::PR_2D() {
    T += 8;
    const auto msb = BC.left & 0b10000000;
    const auto lsb = HL.right & 0b00000001;
    HL.right >>= 1;
    HL.right | msb;
    setFlags((HL.right == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRA L");
}
void CPULR35902::PR_2E() {
    T += 16;
    auto value = bus->read<uint8_t>(HL.w);
    const auto msb = BC.left & 0b10000000;
    const auto lsb = value & 0b00000001;
    value >>= 1;
    value | msb;
    setFlags((value == 0), 0, 0, lsb);
    bus->write<uint8_t>(HL.w, value);
    if (m_debug) logInstruction("SRA (HL)");
}
void CPULR35902::PR_2F() {
    T += 8;
    const auto msb = BC.left & 0b10000000;
    const auto lsb = AF.left & 0b00000001;
    AF.left >>= 1;
    AF.left | msb;
    setFlags((AF.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRA A");
}
void CPULR35902::PR_30() {
    T += 8;
    BC.left = (BC.left << 4) | (BC.left >> 4);
    setFlags((BC.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("SWAP B");
}
void CPULR35902::PR_31() {
    T += 8;
    BC.right = (BC.right << 4) | (BC.right >> 4);
    setFlags((BC.right == 0), 0, 0, 0);
    if (m_debug) logInstruction("SWAP C");
}
void CPULR35902::PR_32() {
    T += 8;
    DE.left = (DE.left << 4) | (DE.left >> 4);
    setFlags((DE.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("SWAP D");
}
void CPULR35902::PR_33() {
    T += 8;
    DE.right = (DE.right << 4) | (DE.right >> 4);
    setFlags((DE.right == 0), 0, 0, 0);
    if (m_debug) logInstruction("SWAP E");
}
void CPULR35902::PR_34() {
    T += 8;
    HL.left = (HL.left << 4) | (HL.left >> 4);
    setFlags((HL.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("SWAP H");
}
void CPULR35902::PR_35() {
    T += 8;
    HL.right = (HL.right << 4) | (HL.right >> 4);
    setFlags((HL.right == 0), 0, 0, 0);
    if (m_debug) logInstruction("SWAP L");
}
void CPULR35902::PR_36() {
    T += 16;
    auto value = bus->read<uint8_t>(HL.w);
    value = (value << 4) | (value >> 4);
    setFlags((value == 0), 0, 0, 0);
    bus->write<uint8_t>(HL.w, value);
    if (m_debug) logInstruction("SWAP (HL)");
}
void CPULR35902::PR_37() {
    T += 8;
    AF.left = (AF.left << 4) | (AF.left >> 4);
    setFlags((AF.left == 0), 0, 0, 0);
    if (m_debug) logInstruction("SWAP A");
}
void CPULR35902::PR_38() {
    T += 8;
    const auto lsb = BC.left & 0b00000001;
    BC.left >>= 1;
    setFlags((BC.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRL B");
}
void CPULR35902::PR_39() {
    T += 8;
    const auto lsb = BC.right & 0b00000001;
    BC.right >>= 1;
    setFlags((BC.right == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRL C");
}
void CPULR35902::PR_3A() {
    T += 8;
    const auto lsb = DE.left & 0b00000001;
    DE.left >>= 1;
    setFlags((DE.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRL D");
}
void CPULR35902::PR_3B() {
    T += 8;
    const auto lsb = DE.right & 0b00000001;
    DE.right >>= 1;
    setFlags((DE.right == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRL E");
}
void CPULR35902::PR_3C() {
    T += 8;
    const auto lsb = HL.left & 0b00000001;
    HL.left >>= 1;
    setFlags((HL.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRL H");
}
void CPULR35902::PR_3D() {
    T += 8;
    const auto lsb = HL.right & 0b00000001;
    HL.right >>= 1;
    setFlags((HL.right == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRL L");
}
void CPULR35902::PR_3E() {
    T += 16;
    auto value = bus->read<uint8_t>(HL.w);
    const auto lsb = value & 0b00000001;
    value >>= 1;
    setFlags((value == 0), 0, 0, lsb);
    bus->write<uint8_t>(HL.w, value);
    if (m_debug) logInstruction("SRL (HL)");
}
void CPULR35902::PR_3F() {
    T += 8;
    const auto lsb = AF.left & 0b00000001;
    AF.left >>= 1;
    setFlags((AF.left == 0), 0, 0, lsb);
    if (m_debug) logInstruction("SRL A");
}
void CPULR35902::PR_40() {
    T += 8;
    setFlags(!(BC.left & 0b00000001), 0, 1, -1);
    if (m_debug) logInstruction("BIT 0, B");
}
void CPULR35902::PR_41() {
    T += 8;
    setFlags(!(BC.right & 0b00000001), 0, 1, -1);
    if (m_debug) logInstruction("BIT 0, C");
}
void CPULR35902::PR_42() {
    T += 8;
    setFlags(!(DE.left & 0b00000001), 0, 1, -1);
    if (m_debug) logInstruction("BIT 0, D");
}
void CPULR35902::PR_43() {
    T += 8;
    setFlags(!(DE.right & 0b00000001), 0, 1, -1);
    if (m_debug) logInstruction("BIT 0, E");
}
void CPULR35902::PR_44() {
    T += 8;
    setFlags(!(HL.left & 0b00000001), 0, 1, -1);
    if (m_debug) logInstruction("BIT 0, H");
}
void CPULR35902::PR_45() {
    T += 8;
    setFlags(!(HL.right & 0b00000001), 0, 1, -1);
    if (m_debug) logInstruction("BIT 0, L");
}
void CPULR35902::PR_46() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    setFlags(!(value & 0b00000001), 0, 1, -1);
    if (m_debug) logInstruction("BIT 0, (HL)");
}
void CPULR35902::PR_47() {
    T += 8;
    setFlags(!(AF.left & 0b00000001), 0, 1, -1);
    if (m_debug) logInstruction("BIT 0, A");
}

void CPULR35902::PR_48() {
    T += 8;
    setFlags(!(BC.left & 0b00000010), 0, 1, -1);
    if (m_debug) logInstruction("BIT 1, B");
}
void CPULR35902::PR_49() {
    T += 8;
    setFlags(!(BC.right & 0b00000010), 0, 1, -1);
    if (m_debug) logInstruction("BIT 1, C");
}
void CPULR35902::PR_4A() {
    T += 8;
    setFlags(!(DE.left & 0b00000010), 0, 1, -1);
    if (m_debug) logInstruction("BIT 1, D");
}
void CPULR35902::PR_4B() {
    T += 8;
    setFlags(!(DE.right & 0b00000010), 0, 1, -1);
    if (m_debug) logInstruction("BIT 1, E");
}
void CPULR35902::PR_4C() {
    T += 8;
    setFlags(!(HL.left & 0b00000010), 0, 1, -1);
    if (m_debug) logInstruction("BIT 1, H");
}
void CPULR35902::PR_4D() {
    T += 8;
    setFlags(!(HL.right & 0b00000010), 0, 1, -1);
    if (m_debug) logInstruction("BIT 1, L");
}
void CPULR35902::PR_4E() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    setFlags(!(value & 0b00000010), 0, 1, -1);
    if (m_debug) logInstruction("BIT 1, (HL)");
}
void CPULR35902::PR_4F() {
    T += 8;
    setFlags(!(AF.left & 0b00000010), 0, 1, -1);
    if (m_debug) logInstruction("BIT 1, A");
}
void CPULR35902::PR_50() {
    T += 8;
    setFlags(!(BC.left & 0b00000100), 0, 1, -1);
    if (m_debug) logInstruction("BIT 2, B");
}
void CPULR35902::PR_51() {
    T += 8;
    setFlags(!(BC.right & 0b00000100), 0, 1, -1);
    if (m_debug) logInstruction("BIT 2, C");
}
void CPULR35902::PR_52() {
    T += 8;
    setFlags(!(DE.left & 0b00000100), 0, 1, -1);
    if (m_debug) logInstruction("BIT 2, D");
}
void CPULR35902::PR_53() {
    T += 8;
    setFlags(!(DE.right & 0b00000100), 0, 1, -1);
    if (m_debug) logInstruction("BIT 2, E");
}
void CPULR35902::PR_54() {
    T += 8;
    setFlags(!(HL.left & 0b00000100), 0, 1, -1);
    if (m_debug) logInstruction("BIT 2, H");
}
void CPULR35902::PR_55() {
    T += 8;
    setFlags(!(HL.right & 0b00000100), 0, 1, -1);
    if (m_debug) logInstruction("BIT 2, L");
}
void CPULR35902::PR_56() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    setFlags(!(value & 0b00000100), 0, 1, -1);
    if (m_debug) logInstruction("BIT 2, (HL)");
}
void CPULR35902::PR_57() {
    T += 8;
    setFlags(!(AF.left & 0b00000100), 0, 1, -1);
    if (m_debug) logInstruction("BIT 2, A");
}
void CPULR35902::PR_58() {
    T += 8;
    setFlags(!(BC.left & 0b00001000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 3, B");
}
void CPULR35902::PR_59() {
    T += 8;
    setFlags(!(BC.right & 0b00001000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 3, C");
}
void CPULR35902::PR_5A() {
    T += 8;
    setFlags(!(DE.left & 0b00001000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 3, D");
}
void CPULR35902::PR_5B() {
    T += 8;
    setFlags(!(DE.right & 0b00001000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 3, E");
}
void CPULR35902::PR_5C() {
    T += 8;
    setFlags(!(HL.left & 0b00001000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 3, H");
}
void CPULR35902::PR_5D() {
    T += 8;
    setFlags(!(HL.right & 0b00001000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 3, L");
}
void CPULR35902::PR_5E() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    setFlags(!(value & 0b00001000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 3, (HL)");
}
void CPULR35902::PR_5F() {
    T += 8;
    setFlags(!(AF.left & 0b00001000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 3, A");
}
void CPULR35902::PR_60() {
    T += 8;
    setFlags(!(BC.left & 0b00010000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 4, B");
}
void CPULR35902::PR_61() {
    T += 8;
    setFlags(!(BC.right & 0b00010000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 4, C");
}
void CPULR35902::PR_62() {
    T += 8;
    setFlags(!(DE.left & 0b00010000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 4, D");
}
void CPULR35902::PR_63() {
    T += 8;
    setFlags(!(DE.right & 0b00010000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 4, E");
}
void CPULR35902::PR_64() {
    T += 8;
    setFlags(!(HL.left & 0b00010000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 4, H");
}
void CPULR35902::PR_65() {
    T += 8;
    setFlags(!(HL.right & 0b00010000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 4, L");
}
void CPULR35902::PR_66() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    setFlags(!(value & 0b00010000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 4, (HL)");
}
void CPULR35902::PR_67() {
    T += 8;
    setFlags(!(AF.left & 0b00010000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 4, A");
}
void CPULR35902::PR_68() {
    T += 8;
    setFlags(!(BC.left & 0b00100000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 5, B");
}
void CPULR35902::PR_69() {
    T += 8;
    setFlags(!(BC.right & 0b00100000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 5, C");
}
void CPULR35902::PR_6A() {
    T += 8;
    setFlags(!(DE.left & 0b00100000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 5, D");
}
void CPULR35902::PR_6B() {
    T += 8;
    setFlags(!(DE.right & 0b00100000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 5, E");
}
void CPULR35902::PR_6C() {
    T += 8;
    setFlags(!(HL.left & 0b00100000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 5, H");
}
void CPULR35902::PR_6D() {
    T += 8;
    setFlags(!(HL.right & 0b00100000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 5, L");
}
void CPULR35902::PR_6E() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    setFlags(!(value & 0b00100000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 5, (HL)");
}
void CPULR35902::PR_6F() {
    T += 8;
    setFlags(!(AF.left & 0b00100000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 5, A");
}
void CPULR35902::PR_70() {
    T += 8;
    setFlags(!(BC.left & 0b01000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 6, B");
}
void CPULR35902::PR_71() {
    T += 8;
    setFlags(!(BC.right & 0b01000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 6, C");
}
void CPULR35902::PR_72() {
    T += 8;
    setFlags(!(DE.left & 0b01000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 6, D");
}
void CPULR35902::PR_73() {
    T += 8;
    setFlags(!(DE.right & 0b01000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 6, E");
}
void CPULR35902::PR_74() {
    T += 8;
    setFlags(!(HL.left & 0b01000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 6, H");
}
void CPULR35902::PR_75() {
    T += 8;
    setFlags(!(HL.right & 0b01000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 6, L");
}
void CPULR35902::PR_76() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    setFlags(!(value & 0b01000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 6, (HL)");
}
void CPULR35902::PR_77() {
    T += 8;
    setFlags(!(AF.left & 0b01000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 6, A");
}
void CPULR35902::PR_78() {
    T += 8;
    setFlags(!(BC.left & 0b10000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 7, B");
}
void CPULR35902::PR_79() {
    T += 8;
    setFlags(!(BC.right & 0b10000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 7, C");
}
void CPULR35902::PR_7A() {
    T += 8;
    setFlags(!(DE.left & 0b10000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 7, D");
}
void CPULR35902::PR_7B() {
    T += 8;
    setFlags(!(DE.right & 0b10000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 7, E");
}
void CPULR35902::PR_7C() {
    T += 8;
    setFlags(!(HL.left & 0b10000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 7, H");
}
void CPULR35902::PR_7D() {
    T += 8;
    setFlags(!(HL.right & 0b10000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 7, L");
}
void CPULR35902::PR_7E() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    setFlags(!(value & 0b10000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 7, (HL)");
}
void CPULR35902::PR_7F() {
    T += 8;
    setFlags(!(AF.left & 0b10000000), 0, 1, -1);
    if (m_debug) logInstruction("BIT 7, A");
}
void CPULR35902::PR_80() {
    T += 8;
    BC.left &= 0b11111110;
    if (m_debug) logInstruction("RES 0, B");
}
void CPULR35902::PR_81() {
    T += 8;
    BC.right &= 0b11111110;
    if (m_debug) logInstruction("RES 0, C");
}
void CPULR35902::PR_82() {
    T += 8;
    DE.left &= 0b11111110;
    if (m_debug) logInstruction("RES 0, D");
}
void CPULR35902::PR_83() {
    T += 8;
    DE.right &= 0b11111110;
    if (m_debug) logInstruction("RES 0, E");
}
void CPULR35902::PR_84() {
    T += 8;
    HL.left &= 0b11111110;
    if (m_debug) logInstruction("RES 0, H");
}
void CPULR35902::PR_85() {
    T += 8;
    HL.right &= 0b11111110;
    if (m_debug) logInstruction("RES 0, L");
}
void CPULR35902::PR_86() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value & 0b11111110);
    if (m_debug) logInstruction("RES 0, HL");
}
void CPULR35902::PR_87() {
    T += 8;
    AF.left &= 0b11111110;
    if (m_debug) logInstruction("RES 0, A");
}
void CPULR35902::PR_88() {
    T += 8;
    BC.left &= 0b11111101;
    if (m_debug) logInstruction("RES 1, B");
}
void CPULR35902::PR_89() {
    T += 8;
    BC.right &= 0b11111101;
    if (m_debug) logInstruction("RES 1, C");
}
void CPULR35902::PR_8A() {
    T += 8;
    DE.left &= 0b11111101;
    if (m_debug) logInstruction("RES 1, D");
}
void CPULR35902::PR_8B() {
    T += 8;
    DE.right &= 0b11111101;
    if (m_debug) logInstruction("RES 1, E");
}
void CPULR35902::PR_8C() {
    T += 8;
    HL.left &= 0b11111101;
    if (m_debug) logInstruction("RES 1, H");
}
void CPULR35902::PR_8D() {
    T += 8;
    HL.right &= 0b11111101;
    if (m_debug) logInstruction("RES 1, L");
}
void CPULR35902::PR_8E() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value & 0b11111101);
    if (m_debug) logInstruction("RES 1, HL");
}
void CPULR35902::PR_8F() {
    T += 8;
    AF.left &= 0b11111101;
    if (m_debug) logInstruction("RES 1, A");
}
void CPULR35902::PR_90() {
    T += 8;
    BC.left &= 0b11111011;
    if (m_debug) logInstruction("RES 2, B");
}
void CPULR35902::PR_91() {
    T += 8;
    BC.right &= 0b11111011;
    if (m_debug) logInstruction("RES 2, C");
}
void CPULR35902::PR_92() {
    T += 8;
    DE.left &= 0b11111011;
    if (m_debug) logInstruction("RES 2, D");
}
void CPULR35902::PR_93() {
    T += 8;
    DE.right &= 0b11111011;
    if (m_debug) logInstruction("RES 2, E");
}
void CPULR35902::PR_94() {
    T += 8;
    HL.left &= 0b11111011;
    if (m_debug) logInstruction("RES 2, H");
}
void CPULR35902::PR_95() {
    T += 8;
    HL.right &= 0b11111011;
    if (m_debug) logInstruction("RES 2, L");
}
void CPULR35902::PR_96() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value & 0b11111011);
    if (m_debug) logInstruction("RES 2, HL");
}
void CPULR35902::PR_97() {
    T += 8;
    AF.left &= 0b11111011;
    if (m_debug) logInstruction("RES 2, A");
}
void CPULR35902::PR_98() {
    T += 8;
    BC.left &= 0b11110111;
    if (m_debug) logInstruction("RES 3, B");
}
void CPULR35902::PR_99() {
    T += 8;
    BC.right &= 0b11110111;
    if (m_debug) logInstruction("RES 3, C");
}
void CPULR35902::PR_9A() {
    T += 8;
    DE.left &= 0b11110111;
    if (m_debug) logInstruction("RES 3, D");
}
void CPULR35902::PR_9B() {
    T += 8;
    DE.right &= 0b11110111;
    if (m_debug) logInstruction("RES 3, E");
}
void CPULR35902::PR_9C() {
    T += 8;
    HL.left &= 0b11110111;
    if (m_debug) logInstruction("RES 3, H");
}
void CPULR35902::PR_9D() {
    T += 8;
    HL.right &= 0b11110111;
    if (m_debug) logInstruction("RES 3, L");
}
void CPULR35902::PR_9E() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value & 0b11110111);
    if (m_debug) logInstruction("RES 3, HL");
}
void CPULR35902::PR_9F() {
    T += 8;
    AF.left &= 0b11110111;
    if (m_debug) logInstruction("RES 3, A");
}
void CPULR35902::PR_A0() {
    T += 8;
    BC.left &= 0b11101111;
    if (m_debug) logInstruction("RES 4, B");
}
void CPULR35902::PR_A1() {
    T += 8;
    BC.right &= 0b11101111;
    if (m_debug) logInstruction("RES 4, C");
}
void CPULR35902::PR_A2() {
    T += 8;
    DE.left &= 0b11101111;
    if (m_debug) logInstruction("RES 4, D");
}
void CPULR35902::PR_A3() {
    T += 8;
    DE.right &= 0b11101111;
    if (m_debug) logInstruction("RES 4, E");
}
void CPULR35902::PR_A4() {
    T += 8;
    HL.left &= 0b11101111;
    if (m_debug) logInstruction("RES 4, H");
}
void CPULR35902::PR_A5() {
    T += 8;
    HL.right &= 0b11101111;
    if (m_debug) logInstruction("RES 4, L");
}
void CPULR35902::PR_A6() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value & 0b11101111);
    if (m_debug) logInstruction("RES 4, HL");
}
void CPULR35902::PR_A7() {
    T += 8;
    AF.left &= 0b11101111;
    if (m_debug) logInstruction("RES 4, A");
}
void CPULR35902::PR_A8() {
    T += 8;
    BC.left &= 0b11011111;
    if (m_debug) logInstruction("RES 5, B");
}
void CPULR35902::PR_A9() {
    T += 8;
    BC.right &= 0b11011111;
    if (m_debug) logInstruction("RES 5, C");
}
void CPULR35902::PR_AA() {
    T += 8;
    DE.left &= 0b11011111;
    if (m_debug) logInstruction("RES 5, D");
}
void CPULR35902::PR_AB() {
    T += 8;
    DE.right &= 0b11011111;
    if (m_debug) logInstruction("RES 5, E");
}
void CPULR35902::PR_AC() {
    T += 8;
    HL.left &= 0b11011111;
    if (m_debug) logInstruction("RES 5, H");
}
void CPULR35902::PR_AD() {
    T += 8;
    HL.right &= 0b11011111;
    if (m_debug) logInstruction("RES 5, L");
}
void CPULR35902::PR_AE() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value & 0b11011111);
    if (m_debug) logInstruction("RES 5, HL");
}
void CPULR35902::PR_AF() {
    T += 8;
    AF.left &= 0b11011111;
    if (m_debug) logInstruction("RES 5, A");
}
void CPULR35902::PR_B0() {
    T += 8;
    BC.left &= 0b10111111;
    if (m_debug) logInstruction("RES 6, B");
}
void CPULR35902::PR_B1() {
    T += 8;
    BC.right &= 0b10111111;
    if (m_debug) logInstruction("RES 6, C");
}
void CPULR35902::PR_B2() {
    T += 8;
    DE.left &= 0b10111111;
    if (m_debug) logInstruction("RES 6, D");
}
void CPULR35902::PR_B3() {
    T += 8;
    DE.right &= 0b10111111;
    if (m_debug) logInstruction("RES 6, E");
}
void CPULR35902::PR_B4() {
    T += 8;
    HL.left &= 0b10111111;
    if (m_debug) logInstruction("RES 6, H");
}
void CPULR35902::PR_B5() {
    T += 8;
    HL.right &= 0b10111111;
    if (m_debug) logInstruction("RES 6, L");
}
void CPULR35902::PR_B6() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value & 0b10111111);
    if (m_debug) logInstruction("RES 6, HL");
}
void CPULR35902::PR_B7() {
    T += 8;
    AF.left &= 0b10111111;
    if (m_debug) logInstruction("RES 6, A");
}
void CPULR35902::PR_B8() {
    T += 8;
    BC.left &= 0b01111111;
    if (m_debug) logInstruction("RES 7, B");
}
void CPULR35902::PR_B9() {
    T += 8;
    BC.right &= 0b01111111;
    if (m_debug) logInstruction("RES 7, C");
}
void CPULR35902::PR_BA() {
    T += 8;
    DE.left &= 0b01111111;
    if (m_debug) logInstruction("RES 7, D");
}
void CPULR35902::PR_BB() {
    T += 8;
    DE.right &= 0b01111111;
    if (m_debug) logInstruction("RES 7, E");
}
void CPULR35902::PR_BC() {
    T += 8;
    HL.left &= 0b01111111;
    if (m_debug) logInstruction("RES 7, H");
}
void CPULR35902::PR_BD() {
    T += 8;
    HL.right &= 0b01111111;
    if (m_debug) logInstruction("RES 7, L");
}
void CPULR35902::PR_BE() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value & 0b01111111);
    if (m_debug) logInstruction("RES 7, HL");
}
void CPULR35902::PR_BF() {
    T += 8;
    AF.left &= 0b01111111;
    if (m_debug) logInstruction("RES 7, A");
}
void CPULR35902::PR_C0() {
    T += 8;
    BC.left |= 0b00000001;
    if (m_debug) logInstruction("SET 0, B");
}
void CPULR35902::PR_C1() {
    T += 8;
    BC.right |= 0b00000001;
    if (m_debug) logInstruction("SET 0, C");
}
void CPULR35902::PR_C2() {
    T += 8;
    DE.left |= 0b00000001;
    if (m_debug) logInstruction("SET 0, D");
}
void CPULR35902::PR_C3() {
    T += 8;
    DE.right |= 0b00000001;
    if (m_debug) logInstruction("SET 0, E");
}
void CPULR35902::PR_C4() {
    T += 8;
    HL.left |= 0b00000001;
    if (m_debug) logInstruction("SET 0, H");
}
void CPULR35902::PR_C5() {
    T += 8;
    HL.right |= 0b00000001;
    if (m_debug) logInstruction("SET 0, L");
}
void CPULR35902::PR_C6() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value | 0b00000001);
    if (m_debug) logInstruction("SET 0, HL");
}
void CPULR35902::PR_C7() {
    T += 8;
    AF.left |= 0b00000001;
    if (m_debug) logInstruction("SET 0, A");
}
void CPULR35902::PR_C8() {
    T += 8;
    BC.left |= 0b00000010;
    if (m_debug) logInstruction("SET 1, B");
}
void CPULR35902::PR_C9() {
    T += 8;
    BC.right |= 0b00000010;
    if (m_debug) logInstruction("SET 1, C");
}
void CPULR35902::PR_CA() {
    T += 8;
    DE.left |= 0b00000010;
    if (m_debug) logInstruction("SET 1, D");
}
void CPULR35902::PR_CB() {
    T += 8;
    DE.right |= 0b00000010;
    if (m_debug) logInstruction("SET 1, E");
}
void CPULR35902::PR_CC() {
    T += 8;
    HL.left |= 0b00000010;
    if (m_debug) logInstruction("SET 1, H");
}
void CPULR35902::PR_CD() {
    T += 8;
    HL.right |= 0b00000010;
    if (m_debug) logInstruction("SET 1, L");
}
void CPULR35902::PR_CE() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value | 0b00000010);
    if (m_debug) logInstruction("SET 1, HL");
}
void CPULR35902::PR_CF() {
    T += 8;
    AF.left |= 0b00000010;
    if (m_debug) logInstruction("SET 1, A");
}
void CPULR35902::PR_D0() {
    T += 8;
    BC.left |= 0b00000100;
    if (m_debug) logInstruction("SET 2, B");
}
void CPULR35902::PR_D1() {
    T += 8;
    BC.right |= 0b00000100;
    if (m_debug) logInstruction("SET 2, C");
}
void CPULR35902::PR_D2() {
    T += 8;
    DE.left |= 0b00000100;
    if (m_debug) logInstruction("SET 2, D");
}
void CPULR35902::PR_D3() {
    T += 8;
    DE.right |= 0b00000100;
    if (m_debug) logInstruction("SET 2, E");
}
void CPULR35902::PR_D4() {
    T += 8;
    HL.left |= 0b00000100;
    if (m_debug) logInstruction("SET 2, H");
}
void CPULR35902::PR_D5() {
    T += 8;
    HL.right |= 0b00000100;
    if (m_debug) logInstruction("SET 2, L");
}
void CPULR35902::PR_D6() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value | 0b00000100);
    if (m_debug) logInstruction("SET 2, HL");
}
void CPULR35902::PR_D7() {
    T += 8;
    AF.left |= 0b00000100;
    if (m_debug) logInstruction("SET 2, A");
}
void CPULR35902::PR_D8() {
    T += 8;
    BC.left |= 0b00001000;
    if (m_debug) logInstruction("SET 3, B");
}
void CPULR35902::PR_D9() {
    T += 8;
    BC.right |= 0b00001000;
    if (m_debug) logInstruction("SET 3, C");
}
void CPULR35902::PR_DA() {
    T += 8;
    DE.left |= 0b00001000;
    if (m_debug) logInstruction("SET 3, D");
}
void CPULR35902::PR_DB() {
    T += 8;
    DE.right |= 0b00001000;
    if (m_debug) logInstruction("SET 3, E");
}
void CPULR35902::PR_DC() {
    T += 8;
    HL.left |= 0b00001000;
    if (m_debug) logInstruction("SET 3, H");
}
void CPULR35902::PR_DD() {
    T += 8;
    HL.right |= 0b00001000;
    if (m_debug) logInstruction("SET 3, L");
}
void CPULR35902::PR_DE() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value | 0b00001000);
    if (m_debug) logInstruction("SET 3, HL");
}
void CPULR35902::PR_DF() {
    T += 8;
    AF.left |= 0b00001000;
    if (m_debug) logInstruction("SET 3, A");
}
void CPULR35902::PR_E0() {
    T += 8;
    BC.left |= 0b00010000;
    if (m_debug) logInstruction("SET 4, B");
}
void CPULR35902::PR_E1() {
    T += 8;
    BC.right |= 0b00010000;
    if (m_debug) logInstruction("SET 4, C");
}
void CPULR35902::PR_E2() {
    T += 8;
    DE.left |= 0b00010000;
    if (m_debug) logInstruction("SET 4, D");
}
void CPULR35902::PR_E3() {
    T += 8;
    DE.right |= 0b00010000;
    if (m_debug) logInstruction("SET 4, E");
}
void CPULR35902::PR_E4() {
    T += 8;
    HL.left |= 0b00010000;
    if (m_debug) logInstruction("SET 4, H");
}
void CPULR35902::PR_E5() {
    T += 8;
    HL.right |= 0b00010000;
    if (m_debug) logInstruction("SET 4, L");
}
void CPULR35902::PR_E6() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value | 0b00010000);
    if (m_debug) logInstruction("SET 4, HL");
}
void CPULR35902::PR_E7() {
    T += 8;
    AF.left |= 0b00010000;
    if (m_debug) logInstruction("SET 4, A");
}
void CPULR35902::PR_E8() {
    T += 8;
    BC.left |= 0b00100000;
    if (m_debug) logInstruction("SET 5, B");
}
void CPULR35902::PR_E9() {
    T += 8;
    BC.right |= 0b00100000;
    if (m_debug) logInstruction("SET 5, C");
}
void CPULR35902::PR_EA() {
    T += 8;
    DE.left |= 0b00100000;
    if (m_debug) logInstruction("SET 5, D");
}
void CPULR35902::PR_EB() {
    T += 8;
    DE.right |= 0b00100000;
    if (m_debug) logInstruction("SET 5, E");
}
void CPULR35902::PR_EC() {
    T += 8;
    HL.left |= 0b00100000;
    if (m_debug) logInstruction("SET 5, H");
}
void CPULR35902::PR_ED() {
    T += 8;
    HL.right |= 0b00100000;
    if (m_debug) logInstruction("SET 5, L");
}
void CPULR35902::PR_EE() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value | 0b00100000);
    if (m_debug) logInstruction("SET 5, HL");
}
void CPULR35902::PR_EF() {
    T += 8;
    AF.left |= 0b00100000;
    if (m_debug) logInstruction("SET 5, A");
}
void CPULR35902::PR_F0() {
    T += 8;
    BC.left |= 0b01000000;
    if (m_debug) logInstruction("SET 6, B");
}
void CPULR35902::PR_F1() {
    T += 8;
    BC.right |= 0b01000000;
    if (m_debug) logInstruction("SET 6, C");
}
void CPULR35902::PR_F2() {
    T += 8;
    DE.left |= 0b01000000;
    if (m_debug) logInstruction("SET 6, D");
}
void CPULR35902::PR_F3() {
    T += 8;
    DE.right |= 0b01000000;
    if (m_debug) logInstruction("SET 6, E");
}
void CPULR35902::PR_F4() {
    T += 8;
    HL.left |= 0b01000000;
    if (m_debug) logInstruction("SET 6, H");
}
void CPULR35902::PR_F5() {
    T += 8;
    HL.right |= 0b01000000;
    if (m_debug) logInstruction("SET 6, L");
}
void CPULR35902::PR_F6() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value | 0b01000000);
    if (m_debug) logInstruction("SET 6, HL");
}
void CPULR35902::PR_F7() {
    T += 8;
    AF.left |= 0b01000000;
    if (m_debug) logInstruction("SET 6, A");
}
void CPULR35902::PR_F8() {
    T += 8;
    BC.left |= 0b10000000;
    if (m_debug) logInstruction("SET 7, B");
}
void CPULR35902::PR_F9() {
    T += 8;
    BC.right |= 0b10000000;
    if (m_debug) logInstruction("SET 7, C");
}
void CPULR35902::PR_FA() {
    T += 8;
    DE.left |= 0b10000000;
    if (m_debug) logInstruction("SET 7, D");
}
void CPULR35902::PR_FB() {
    T += 8;
    DE.right |= 0b10000000;
    if (m_debug) logInstruction("SET 7, E");
}
void CPULR35902::PR_FC() {
    T += 8;
    HL.left |= 0b10000000;
    if (m_debug) logInstruction("SET 7, H");
}
void CPULR35902::PR_FD() {
    T += 8;
    HL.right |= 0b10000000;
    if (m_debug) logInstruction("SET 7, L");
}
void CPULR35902::PR_FE() {
    T += 8;
    const auto value = bus->read<uint8_t>(HL.w);
    bus->write<uint8_t>(HL.w, value | 0b10000000);
    if (m_debug) logInstruction("SET 7, HL");
}
void CPULR35902::PR_FF() {
    T += 8;
    AF.left |= 0b10000000;
    if (m_debug) logInstruction("SET 7, A");
}

void CPULR35902::initOpcodeHandlers() {
    opcodeHandler[0x00] = std::bind(&CPULR35902::OP_00, this);
    opcodeHandler[0x01] = std::bind(&CPULR35902::OP_01, this);
    opcodeHandler[0x02] = std::bind(&CPULR35902::OP_02, this);
    opcodeHandler[0x03] = std::bind(&CPULR35902::OP_03, this);
    opcodeHandler[0x04] = std::bind(&CPULR35902::OP_04, this);
    opcodeHandler[0x05] = std::bind(&CPULR35902::OP_05, this);
    opcodeHandler[0x06] = std::bind(&CPULR35902::OP_06, this);
    opcodeHandler[0x07] = std::bind(&CPULR35902::OP_07, this);
    opcodeHandler[0x08] = std::bind(&CPULR35902::OP_08, this);
    opcodeHandler[0x09] = std::bind(&CPULR35902::OP_09, this);
    opcodeHandler[0x0A] = std::bind(&CPULR35902::OP_0A, this);
    opcodeHandler[0x0B] = std::bind(&CPULR35902::OP_0B, this);
    opcodeHandler[0x0C] = std::bind(&CPULR35902::OP_0C, this);
    opcodeHandler[0x0D] = std::bind(&CPULR35902::OP_0D, this);
    opcodeHandler[0x0E] = std::bind(&CPULR35902::OP_0E, this);
    opcodeHandler[0x0F] = std::bind(&CPULR35902::OP_0F, this);
    opcodeHandler[0x10] = std::bind(&CPULR35902::OP_10, this);
    opcodeHandler[0x11] = std::bind(&CPULR35902::OP_11, this);
    opcodeHandler[0x12] = std::bind(&CPULR35902::OP_12, this);
    opcodeHandler[0x13] = std::bind(&CPULR35902::OP_13, this);
    opcodeHandler[0x14] = std::bind(&CPULR35902::OP_14, this);
    opcodeHandler[0x15] = std::bind(&CPULR35902::OP_15, this);
    opcodeHandler[0x16] = std::bind(&CPULR35902::OP_16, this);
    opcodeHandler[0x17] = std::bind(&CPULR35902::OP_17, this);
    opcodeHandler[0x18] = std::bind(&CPULR35902::OP_18, this);
    opcodeHandler[0x19] = std::bind(&CPULR35902::OP_19, this);
    opcodeHandler[0x1A] = std::bind(&CPULR35902::OP_1A, this);
    opcodeHandler[0x1B] = std::bind(&CPULR35902::OP_1B, this);
    opcodeHandler[0x1C] = std::bind(&CPULR35902::OP_1C, this);
    opcodeHandler[0x1D] = std::bind(&CPULR35902::OP_1D, this);
    opcodeHandler[0x1E] = std::bind(&CPULR35902::OP_1E, this);
    opcodeHandler[0x1F] = std::bind(&CPULR35902::OP_1F, this);
    opcodeHandler[0x20] = std::bind(&CPULR35902::OP_20, this);
    opcodeHandler[0x21] = std::bind(&CPULR35902::OP_21, this);
    opcodeHandler[0x22] = std::bind(&CPULR35902::OP_22, this);
    opcodeHandler[0x23] = std::bind(&CPULR35902::OP_23, this);
    opcodeHandler[0x24] = std::bind(&CPULR35902::OP_24, this);
    opcodeHandler[0x25] = std::bind(&CPULR35902::OP_25, this);
    opcodeHandler[0x26] = std::bind(&CPULR35902::OP_26, this);
    opcodeHandler[0x27] = std::bind(&CPULR35902::OP_27, this);
    opcodeHandler[0x28] = std::bind(&CPULR35902::OP_28, this);
    opcodeHandler[0x29] = std::bind(&CPULR35902::OP_29, this);
    opcodeHandler[0x2A] = std::bind(&CPULR35902::OP_2A, this);
    opcodeHandler[0x2B] = std::bind(&CPULR35902::OP_2B, this);
    opcodeHandler[0x2C] = std::bind(&CPULR35902::OP_2C, this);
    opcodeHandler[0x2D] = std::bind(&CPULR35902::OP_2D, this);
    opcodeHandler[0x2E] = std::bind(&CPULR35902::OP_2E, this);
    opcodeHandler[0x2F] = std::bind(&CPULR35902::OP_2F, this);
    opcodeHandler[0x30] = std::bind(&CPULR35902::OP_30, this);
    opcodeHandler[0x31] = std::bind(&CPULR35902::OP_31, this);
    opcodeHandler[0x32] = std::bind(&CPULR35902::OP_32, this);
    opcodeHandler[0x33] = std::bind(&CPULR35902::OP_33, this);
    opcodeHandler[0x34] = std::bind(&CPULR35902::OP_34, this);
    opcodeHandler[0x35] = std::bind(&CPULR35902::OP_35, this);
    opcodeHandler[0x36] = std::bind(&CPULR35902::OP_36, this);
    opcodeHandler[0x37] = std::bind(&CPULR35902::OP_37, this);
    opcodeHandler[0x38] = std::bind(&CPULR35902::OP_38, this);
    opcodeHandler[0x39] = std::bind(&CPULR35902::OP_39, this);
    opcodeHandler[0x3A] = std::bind(&CPULR35902::OP_3A, this);
    opcodeHandler[0x3B] = std::bind(&CPULR35902::OP_3B, this);
    opcodeHandler[0x3C] = std::bind(&CPULR35902::OP_3C, this);
    opcodeHandler[0x3D] = std::bind(&CPULR35902::OP_3D, this);
    opcodeHandler[0x3E] = std::bind(&CPULR35902::OP_3E, this);
    opcodeHandler[0x3F] = std::bind(&CPULR35902::OP_3F, this);
    opcodeHandler[0x40] = std::bind(&CPULR35902::OP_40, this);
    opcodeHandler[0x41] = std::bind(&CPULR35902::OP_41, this);
    opcodeHandler[0x42] = std::bind(&CPULR35902::OP_42, this);
    opcodeHandler[0x43] = std::bind(&CPULR35902::OP_43, this);
    opcodeHandler[0x44] = std::bind(&CPULR35902::OP_44, this);
    opcodeHandler[0x45] = std::bind(&CPULR35902::OP_45, this);
    opcodeHandler[0x46] = std::bind(&CPULR35902::OP_46, this);
    opcodeHandler[0x47] = std::bind(&CPULR35902::OP_47, this);
    opcodeHandler[0x48] = std::bind(&CPULR35902::OP_48, this);
    opcodeHandler[0x49] = std::bind(&CPULR35902::OP_49, this);
    opcodeHandler[0x4A] = std::bind(&CPULR35902::OP_4A, this);
    opcodeHandler[0x4B] = std::bind(&CPULR35902::OP_4B, this);
    opcodeHandler[0x4C] = std::bind(&CPULR35902::OP_4C, this);
    opcodeHandler[0x4D] = std::bind(&CPULR35902::OP_4D, this);
    opcodeHandler[0x4E] = std::bind(&CPULR35902::OP_4E, this);
    opcodeHandler[0x4F] = std::bind(&CPULR35902::OP_4F, this);
    opcodeHandler[0x50] = std::bind(&CPULR35902::OP_50, this);
    opcodeHandler[0x51] = std::bind(&CPULR35902::OP_51, this);
    opcodeHandler[0x52] = std::bind(&CPULR35902::OP_52, this);
    opcodeHandler[0x53] = std::bind(&CPULR35902::OP_53, this);
    opcodeHandler[0x54] = std::bind(&CPULR35902::OP_54, this);
    opcodeHandler[0x55] = std::bind(&CPULR35902::OP_55, this);
    opcodeHandler[0x56] = std::bind(&CPULR35902::OP_56, this);
    opcodeHandler[0x57] = std::bind(&CPULR35902::OP_57, this);
    opcodeHandler[0x58] = std::bind(&CPULR35902::OP_58, this);
    opcodeHandler[0x59] = std::bind(&CPULR35902::OP_59, this);
    opcodeHandler[0x5A] = std::bind(&CPULR35902::OP_5A, this);
    opcodeHandler[0x5B] = std::bind(&CPULR35902::OP_5B, this);
    opcodeHandler[0x5C] = std::bind(&CPULR35902::OP_5C, this);
    opcodeHandler[0x5D] = std::bind(&CPULR35902::OP_5D, this);
    opcodeHandler[0x5E] = std::bind(&CPULR35902::OP_5E, this);
    opcodeHandler[0x5F] = std::bind(&CPULR35902::OP_5F, this);
    opcodeHandler[0x60] = std::bind(&CPULR35902::OP_60, this);
    opcodeHandler[0x61] = std::bind(&CPULR35902::OP_61, this);
    opcodeHandler[0x62] = std::bind(&CPULR35902::OP_62, this);
    opcodeHandler[0x63] = std::bind(&CPULR35902::OP_63, this);
    opcodeHandler[0x64] = std::bind(&CPULR35902::OP_64, this);
    opcodeHandler[0x65] = std::bind(&CPULR35902::OP_65, this);
    opcodeHandler[0x66] = std::bind(&CPULR35902::OP_66, this);
    opcodeHandler[0x67] = std::bind(&CPULR35902::OP_67, this);
    opcodeHandler[0x68] = std::bind(&CPULR35902::OP_68, this);
    opcodeHandler[0x69] = std::bind(&CPULR35902::OP_69, this);
    opcodeHandler[0x6A] = std::bind(&CPULR35902::OP_6A, this);
    opcodeHandler[0x6B] = std::bind(&CPULR35902::OP_6B, this);
    opcodeHandler[0x6C] = std::bind(&CPULR35902::OP_6C, this);
    opcodeHandler[0x6D] = std::bind(&CPULR35902::OP_6D, this);
    opcodeHandler[0x6E] = std::bind(&CPULR35902::OP_6E, this);
    opcodeHandler[0x6F] = std::bind(&CPULR35902::OP_6F, this);
    opcodeHandler[0x70] = std::bind(&CPULR35902::OP_70, this);
    opcodeHandler[0x71] = std::bind(&CPULR35902::OP_71, this);
    opcodeHandler[0x72] = std::bind(&CPULR35902::OP_72, this);
    opcodeHandler[0x73] = std::bind(&CPULR35902::OP_73, this);
    opcodeHandler[0x74] = std::bind(&CPULR35902::OP_74, this);
    opcodeHandler[0x75] = std::bind(&CPULR35902::OP_75, this);
    opcodeHandler[0x76] = std::bind(&CPULR35902::OP_76, this);
    opcodeHandler[0x77] = std::bind(&CPULR35902::OP_77, this);
    opcodeHandler[0x78] = std::bind(&CPULR35902::OP_78, this);
    opcodeHandler[0x79] = std::bind(&CPULR35902::OP_79, this);
    opcodeHandler[0x7A] = std::bind(&CPULR35902::OP_7A, this);
    opcodeHandler[0x7B] = std::bind(&CPULR35902::OP_7B, this);
    opcodeHandler[0x7C] = std::bind(&CPULR35902::OP_7C, this);
    opcodeHandler[0x7D] = std::bind(&CPULR35902::OP_7D, this);
    opcodeHandler[0x7E] = std::bind(&CPULR35902::OP_7E, this);
    opcodeHandler[0x7F] = std::bind(&CPULR35902::OP_7F, this);
    opcodeHandler[0x80] = std::bind(&CPULR35902::OP_80, this);
    opcodeHandler[0x81] = std::bind(&CPULR35902::OP_81, this);
    opcodeHandler[0x82] = std::bind(&CPULR35902::OP_82, this);
    opcodeHandler[0x83] = std::bind(&CPULR35902::OP_83, this);
    opcodeHandler[0x84] = std::bind(&CPULR35902::OP_84, this);
    opcodeHandler[0x85] = std::bind(&CPULR35902::OP_85, this);
    opcodeHandler[0x86] = std::bind(&CPULR35902::OP_86, this);
    opcodeHandler[0x87] = std::bind(&CPULR35902::OP_87, this);
    opcodeHandler[0x88] = std::bind(&CPULR35902::OP_88, this);
    opcodeHandler[0x89] = std::bind(&CPULR35902::OP_89, this);
    opcodeHandler[0x8A] = std::bind(&CPULR35902::OP_8A, this);
    opcodeHandler[0x8B] = std::bind(&CPULR35902::OP_8B, this);
    opcodeHandler[0x8C] = std::bind(&CPULR35902::OP_8C, this);
    opcodeHandler[0x8D] = std::bind(&CPULR35902::OP_8D, this);
    opcodeHandler[0x8E] = std::bind(&CPULR35902::OP_8E, this);
    opcodeHandler[0x8F] = std::bind(&CPULR35902::OP_8F, this);
    opcodeHandler[0x90] = std::bind(&CPULR35902::OP_90, this);
    opcodeHandler[0x91] = std::bind(&CPULR35902::OP_91, this);
    opcodeHandler[0x92] = std::bind(&CPULR35902::OP_92, this);
    opcodeHandler[0x93] = std::bind(&CPULR35902::OP_93, this);
    opcodeHandler[0x94] = std::bind(&CPULR35902::OP_94, this);
    opcodeHandler[0x95] = std::bind(&CPULR35902::OP_95, this);
    opcodeHandler[0x96] = std::bind(&CPULR35902::OP_96, this);
    opcodeHandler[0x97] = std::bind(&CPULR35902::OP_97, this);
    opcodeHandler[0x98] = std::bind(&CPULR35902::OP_98, this);
    opcodeHandler[0x99] = std::bind(&CPULR35902::OP_99, this);
    opcodeHandler[0x9A] = std::bind(&CPULR35902::OP_9A, this);
    opcodeHandler[0x9B] = std::bind(&CPULR35902::OP_9B, this);
    opcodeHandler[0x9C] = std::bind(&CPULR35902::OP_9C, this);
    opcodeHandler[0x9D] = std::bind(&CPULR35902::OP_9D, this);
    opcodeHandler[0x9E] = std::bind(&CPULR35902::OP_9E, this);
    opcodeHandler[0x9F] = std::bind(&CPULR35902::OP_9F, this);
    opcodeHandler[0xA0] = std::bind(&CPULR35902::OP_A0, this);
    opcodeHandler[0xA1] = std::bind(&CPULR35902::OP_A1, this);
    opcodeHandler[0xA2] = std::bind(&CPULR35902::OP_A2, this);
    opcodeHandler[0xA3] = std::bind(&CPULR35902::OP_A3, this);
    opcodeHandler[0xA4] = std::bind(&CPULR35902::OP_A4, this);
    opcodeHandler[0xA5] = std::bind(&CPULR35902::OP_A5, this);
    opcodeHandler[0xA6] = std::bind(&CPULR35902::OP_A6, this);
    opcodeHandler[0xA7] = std::bind(&CPULR35902::OP_A7, this);
    opcodeHandler[0xA8] = std::bind(&CPULR35902::OP_A8, this);
    opcodeHandler[0xA9] = std::bind(&CPULR35902::OP_A9, this);
    opcodeHandler[0xAA] = std::bind(&CPULR35902::OP_AA, this);
    opcodeHandler[0xAB] = std::bind(&CPULR35902::OP_AB, this);
    opcodeHandler[0xAC] = std::bind(&CPULR35902::OP_AC, this);
    opcodeHandler[0xAD] = std::bind(&CPULR35902::OP_AD, this);
    opcodeHandler[0xAE] = std::bind(&CPULR35902::OP_AE, this);
    opcodeHandler[0xAF] = std::bind(&CPULR35902::OP_AF, this);
    opcodeHandler[0xB0] = std::bind(&CPULR35902::OP_B0, this);
    opcodeHandler[0xB1] = std::bind(&CPULR35902::OP_B1, this);
    opcodeHandler[0xB2] = std::bind(&CPULR35902::OP_B2, this);
    opcodeHandler[0xB3] = std::bind(&CPULR35902::OP_B3, this);
    opcodeHandler[0xB4] = std::bind(&CPULR35902::OP_B4, this);
    opcodeHandler[0xB5] = std::bind(&CPULR35902::OP_B5, this);
    opcodeHandler[0xB6] = std::bind(&CPULR35902::OP_B6, this);
    opcodeHandler[0xB7] = std::bind(&CPULR35902::OP_B7, this);
    opcodeHandler[0xB8] = std::bind(&CPULR35902::OP_B8, this);
    opcodeHandler[0xB9] = std::bind(&CPULR35902::OP_B9, this);
    opcodeHandler[0xBA] = std::bind(&CPULR35902::OP_BA, this);
    opcodeHandler[0xBB] = std::bind(&CPULR35902::OP_BB, this);
    opcodeHandler[0xBC] = std::bind(&CPULR35902::OP_BC, this);
    opcodeHandler[0xBD] = std::bind(&CPULR35902::OP_BD, this);
    opcodeHandler[0xBE] = std::bind(&CPULR35902::OP_BE, this);
    opcodeHandler[0xBF] = std::bind(&CPULR35902::OP_BF, this);
    opcodeHandler[0xC0] = std::bind(&CPULR35902::OP_C0, this);
    opcodeHandler[0xC1] = std::bind(&CPULR35902::OP_C1, this);
    opcodeHandler[0xC2] = std::bind(&CPULR35902::OP_C2, this);
    opcodeHandler[0xC3] = std::bind(&CPULR35902::OP_C3, this);
    opcodeHandler[0xC4] = std::bind(&CPULR35902::OP_C4, this);
    opcodeHandler[0xC5] = std::bind(&CPULR35902::OP_C5, this);
    opcodeHandler[0xC6] = std::bind(&CPULR35902::OP_C6, this);
    opcodeHandler[0xC7] = std::bind(&CPULR35902::OP_C7, this);
    opcodeHandler[0xC8] = std::bind(&CPULR35902::OP_C8, this);
    opcodeHandler[0xC9] = std::bind(&CPULR35902::OP_C9, this);
    opcodeHandler[0xCA] = std::bind(&CPULR35902::OP_CA, this);
    opcodeHandler[0xCB] = std::bind(&CPULR35902::OP_CB, this);
    opcodeHandler[0xCC] = std::bind(&CPULR35902::OP_CC, this);
    opcodeHandler[0xCD] = std::bind(&CPULR35902::OP_CD, this);
    opcodeHandler[0xCE] = std::bind(&CPULR35902::OP_CE, this);
    opcodeHandler[0xCF] = std::bind(&CPULR35902::OP_CF, this);
    opcodeHandler[0xD0] = std::bind(&CPULR35902::OP_D0, this);
    opcodeHandler[0xD1] = std::bind(&CPULR35902::OP_D1, this);
    opcodeHandler[0xD2] = std::bind(&CPULR35902::OP_D2, this);
    opcodeHandler[0xD3] = std::bind(&CPULR35902::OP_D3, this);
    opcodeHandler[0xD4] = std::bind(&CPULR35902::OP_D4, this);
    opcodeHandler[0xD5] = std::bind(&CPULR35902::OP_D5, this);
    opcodeHandler[0xD6] = std::bind(&CPULR35902::OP_D6, this);
    opcodeHandler[0xD7] = std::bind(&CPULR35902::OP_D7, this);
    opcodeHandler[0xD8] = std::bind(&CPULR35902::OP_D8, this);
    opcodeHandler[0xD9] = std::bind(&CPULR35902::OP_D9, this);
    opcodeHandler[0xDA] = std::bind(&CPULR35902::OP_DA, this);
    opcodeHandler[0xDB] = std::bind(&CPULR35902::OP_DB, this);
    opcodeHandler[0xDC] = std::bind(&CPULR35902::OP_DC, this);
    opcodeHandler[0xDD] = std::bind(&CPULR35902::OP_DD, this);
    opcodeHandler[0xDE] = std::bind(&CPULR35902::OP_DE, this);
    opcodeHandler[0xDF] = std::bind(&CPULR35902::OP_DF, this);
    opcodeHandler[0xE0] = std::bind(&CPULR35902::OP_E0, this);
    opcodeHandler[0xE1] = std::bind(&CPULR35902::OP_E1, this);
    opcodeHandler[0xE2] = std::bind(&CPULR35902::OP_E2, this);
    opcodeHandler[0xE3] = std::bind(&CPULR35902::OP_E3, this);
    opcodeHandler[0xE4] = std::bind(&CPULR35902::OP_E4, this);
    opcodeHandler[0xE5] = std::bind(&CPULR35902::OP_E5, this);
    opcodeHandler[0xE6] = std::bind(&CPULR35902::OP_E6, this);
    opcodeHandler[0xE7] = std::bind(&CPULR35902::OP_E7, this);
    opcodeHandler[0xE8] = std::bind(&CPULR35902::OP_E8, this);
    opcodeHandler[0xE9] = std::bind(&CPULR35902::OP_E9, this);
    opcodeHandler[0xEA] = std::bind(&CPULR35902::OP_EA, this);
    opcodeHandler[0xEB] = std::bind(&CPULR35902::OP_EB, this);
    opcodeHandler[0xEC] = std::bind(&CPULR35902::OP_EC, this);
    opcodeHandler[0xED] = std::bind(&CPULR35902::OP_ED, this);
    opcodeHandler[0xEE] = std::bind(&CPULR35902::OP_EE, this);
    opcodeHandler[0xEF] = std::bind(&CPULR35902::OP_EF, this);
    opcodeHandler[0xF0] = std::bind(&CPULR35902::OP_F0, this);
    opcodeHandler[0xF1] = std::bind(&CPULR35902::OP_F1, this);
    opcodeHandler[0xF2] = std::bind(&CPULR35902::OP_F2, this);
    opcodeHandler[0xF3] = std::bind(&CPULR35902::OP_F3, this);
    opcodeHandler[0xF4] = std::bind(&CPULR35902::OP_F4, this);
    opcodeHandler[0xF5] = std::bind(&CPULR35902::OP_F5, this);
    opcodeHandler[0xF6] = std::bind(&CPULR35902::OP_F6, this);
    opcodeHandler[0xF7] = std::bind(&CPULR35902::OP_F7, this);
    opcodeHandler[0xF8] = std::bind(&CPULR35902::OP_F8, this);
    opcodeHandler[0xF9] = std::bind(&CPULR35902::OP_F9, this);
    opcodeHandler[0xFA] = std::bind(&CPULR35902::OP_FA, this);
    opcodeHandler[0xFB] = std::bind(&CPULR35902::OP_FB, this);
    opcodeHandler[0xFC] = std::bind(&CPULR35902::OP_FC, this);
    opcodeHandler[0xFD] = std::bind(&CPULR35902::OP_FD, this);
    opcodeHandler[0xFE] = std::bind(&CPULR35902::OP_FE, this);
    opcodeHandler[0xFF] = std::bind(&CPULR35902::OP_FF, this);

    prefixHandler[0x00] = std::bind(&CPULR35902::PR_00, this);
    prefixHandler[0x01] = std::bind(&CPULR35902::PR_01, this);
    prefixHandler[0x02] = std::bind(&CPULR35902::PR_02, this);
    prefixHandler[0x03] = std::bind(&CPULR35902::PR_03, this);
    prefixHandler[0x04] = std::bind(&CPULR35902::PR_04, this);
    prefixHandler[0x05] = std::bind(&CPULR35902::PR_05, this);
    prefixHandler[0x06] = std::bind(&CPULR35902::PR_06, this);
    prefixHandler[0x07] = std::bind(&CPULR35902::PR_07, this);
    prefixHandler[0x08] = std::bind(&CPULR35902::PR_08, this);
    prefixHandler[0x09] = std::bind(&CPULR35902::PR_09, this);
    prefixHandler[0x0A] = std::bind(&CPULR35902::PR_0A, this);
    prefixHandler[0x0B] = std::bind(&CPULR35902::PR_0B, this);
    prefixHandler[0x0C] = std::bind(&CPULR35902::PR_0C, this);
    prefixHandler[0x0D] = std::bind(&CPULR35902::PR_0D, this);
    prefixHandler[0x0E] = std::bind(&CPULR35902::PR_0E, this);
    prefixHandler[0x0F] = std::bind(&CPULR35902::PR_0F, this);
    prefixHandler[0x10] = std::bind(&CPULR35902::PR_10, this);
    prefixHandler[0x11] = std::bind(&CPULR35902::PR_11, this);
    prefixHandler[0x12] = std::bind(&CPULR35902::PR_12, this);
    prefixHandler[0x13] = std::bind(&CPULR35902::PR_13, this);
    prefixHandler[0x14] = std::bind(&CPULR35902::PR_14, this);
    prefixHandler[0x15] = std::bind(&CPULR35902::PR_15, this);
    prefixHandler[0x16] = std::bind(&CPULR35902::PR_16, this);
    prefixHandler[0x17] = std::bind(&CPULR35902::PR_17, this);
    prefixHandler[0x18] = std::bind(&CPULR35902::PR_18, this);
    prefixHandler[0x19] = std::bind(&CPULR35902::PR_19, this);
    prefixHandler[0x1A] = std::bind(&CPULR35902::PR_1A, this);
    prefixHandler[0x1B] = std::bind(&CPULR35902::PR_1B, this);
    prefixHandler[0x1C] = std::bind(&CPULR35902::PR_1C, this);
    prefixHandler[0x1D] = std::bind(&CPULR35902::PR_1D, this);
    prefixHandler[0x1E] = std::bind(&CPULR35902::PR_1E, this);
    prefixHandler[0x1F] = std::bind(&CPULR35902::PR_1F, this);
    prefixHandler[0x20] = std::bind(&CPULR35902::PR_20, this);
    prefixHandler[0x21] = std::bind(&CPULR35902::PR_21, this);
    prefixHandler[0x22] = std::bind(&CPULR35902::PR_22, this);
    prefixHandler[0x23] = std::bind(&CPULR35902::PR_23, this);
    prefixHandler[0x24] = std::bind(&CPULR35902::PR_24, this);
    prefixHandler[0x25] = std::bind(&CPULR35902::PR_25, this);
    prefixHandler[0x26] = std::bind(&CPULR35902::PR_26, this);
    prefixHandler[0x27] = std::bind(&CPULR35902::PR_27, this);
    prefixHandler[0x28] = std::bind(&CPULR35902::PR_28, this);
    prefixHandler[0x29] = std::bind(&CPULR35902::PR_29, this);
    prefixHandler[0x2A] = std::bind(&CPULR35902::PR_2A, this);
    prefixHandler[0x2B] = std::bind(&CPULR35902::PR_2B, this);
    prefixHandler[0x2C] = std::bind(&CPULR35902::PR_2C, this);
    prefixHandler[0x2D] = std::bind(&CPULR35902::PR_2D, this);
    prefixHandler[0x2E] = std::bind(&CPULR35902::PR_2E, this);
    prefixHandler[0x2F] = std::bind(&CPULR35902::PR_2F, this);
    prefixHandler[0x30] = std::bind(&CPULR35902::PR_30, this);
    prefixHandler[0x31] = std::bind(&CPULR35902::PR_31, this);
    prefixHandler[0x32] = std::bind(&CPULR35902::PR_32, this);
    prefixHandler[0x33] = std::bind(&CPULR35902::PR_33, this);
    prefixHandler[0x34] = std::bind(&CPULR35902::PR_34, this);
    prefixHandler[0x35] = std::bind(&CPULR35902::PR_35, this);
    prefixHandler[0x36] = std::bind(&CPULR35902::PR_36, this);
    prefixHandler[0x37] = std::bind(&CPULR35902::PR_37, this);
    prefixHandler[0x38] = std::bind(&CPULR35902::PR_38, this);
    prefixHandler[0x39] = std::bind(&CPULR35902::PR_39, this);
    prefixHandler[0x3A] = std::bind(&CPULR35902::PR_3A, this);
    prefixHandler[0x3B] = std::bind(&CPULR35902::PR_3B, this);
    prefixHandler[0x3C] = std::bind(&CPULR35902::PR_3C, this);
    prefixHandler[0x3D] = std::bind(&CPULR35902::PR_3D, this);
    prefixHandler[0x3E] = std::bind(&CPULR35902::PR_3E, this);
    prefixHandler[0x3F] = std::bind(&CPULR35902::PR_3F, this);
    prefixHandler[0x40] = std::bind(&CPULR35902::PR_40, this);
    prefixHandler[0x41] = std::bind(&CPULR35902::PR_41, this);
    prefixHandler[0x42] = std::bind(&CPULR35902::PR_42, this);
    prefixHandler[0x43] = std::bind(&CPULR35902::PR_43, this);
    prefixHandler[0x44] = std::bind(&CPULR35902::PR_44, this);
    prefixHandler[0x45] = std::bind(&CPULR35902::PR_45, this);
    prefixHandler[0x46] = std::bind(&CPULR35902::PR_46, this);
    prefixHandler[0x47] = std::bind(&CPULR35902::PR_47, this);
    prefixHandler[0x48] = std::bind(&CPULR35902::PR_48, this);
    prefixHandler[0x49] = std::bind(&CPULR35902::PR_49, this);
    prefixHandler[0x4A] = std::bind(&CPULR35902::PR_4A, this);
    prefixHandler[0x4B] = std::bind(&CPULR35902::PR_4B, this);
    prefixHandler[0x4C] = std::bind(&CPULR35902::PR_4C, this);
    prefixHandler[0x4D] = std::bind(&CPULR35902::PR_4D, this);
    prefixHandler[0x4E] = std::bind(&CPULR35902::PR_4E, this);
    prefixHandler[0x4F] = std::bind(&CPULR35902::PR_4F, this);
    prefixHandler[0x50] = std::bind(&CPULR35902::PR_50, this);
    prefixHandler[0x51] = std::bind(&CPULR35902::PR_51, this);
    prefixHandler[0x52] = std::bind(&CPULR35902::PR_52, this);
    prefixHandler[0x53] = std::bind(&CPULR35902::PR_53, this);
    prefixHandler[0x54] = std::bind(&CPULR35902::PR_54, this);
    prefixHandler[0x55] = std::bind(&CPULR35902::PR_55, this);
    prefixHandler[0x56] = std::bind(&CPULR35902::PR_56, this);
    prefixHandler[0x57] = std::bind(&CPULR35902::PR_57, this);
    prefixHandler[0x58] = std::bind(&CPULR35902::PR_58, this);
    prefixHandler[0x59] = std::bind(&CPULR35902::PR_59, this);
    prefixHandler[0x5A] = std::bind(&CPULR35902::PR_5A, this);
    prefixHandler[0x5B] = std::bind(&CPULR35902::PR_5B, this);
    prefixHandler[0x5C] = std::bind(&CPULR35902::PR_5C, this);
    prefixHandler[0x5D] = std::bind(&CPULR35902::PR_5D, this);
    prefixHandler[0x5E] = std::bind(&CPULR35902::PR_5E, this);
    prefixHandler[0x5F] = std::bind(&CPULR35902::PR_5F, this);
    prefixHandler[0x60] = std::bind(&CPULR35902::PR_60, this);
    prefixHandler[0x61] = std::bind(&CPULR35902::PR_61, this);
    prefixHandler[0x62] = std::bind(&CPULR35902::PR_62, this);
    prefixHandler[0x63] = std::bind(&CPULR35902::PR_63, this);
    prefixHandler[0x64] = std::bind(&CPULR35902::PR_64, this);
    prefixHandler[0x65] = std::bind(&CPULR35902::PR_65, this);
    prefixHandler[0x66] = std::bind(&CPULR35902::PR_66, this);
    prefixHandler[0x67] = std::bind(&CPULR35902::PR_67, this);
    prefixHandler[0x68] = std::bind(&CPULR35902::PR_68, this);
    prefixHandler[0x69] = std::bind(&CPULR35902::PR_69, this);
    prefixHandler[0x6A] = std::bind(&CPULR35902::PR_6A, this);
    prefixHandler[0x6B] = std::bind(&CPULR35902::PR_6B, this);
    prefixHandler[0x6C] = std::bind(&CPULR35902::PR_6C, this);
    prefixHandler[0x6D] = std::bind(&CPULR35902::PR_6D, this);
    prefixHandler[0x6E] = std::bind(&CPULR35902::PR_6E, this);
    prefixHandler[0x6F] = std::bind(&CPULR35902::PR_6F, this);
    prefixHandler[0x70] = std::bind(&CPULR35902::PR_70, this);
    prefixHandler[0x71] = std::bind(&CPULR35902::PR_71, this);
    prefixHandler[0x72] = std::bind(&CPULR35902::PR_72, this);
    prefixHandler[0x73] = std::bind(&CPULR35902::PR_73, this);
    prefixHandler[0x74] = std::bind(&CPULR35902::PR_74, this);
    prefixHandler[0x75] = std::bind(&CPULR35902::PR_75, this);
    prefixHandler[0x76] = std::bind(&CPULR35902::PR_76, this);
    prefixHandler[0x77] = std::bind(&CPULR35902::PR_77, this);
    prefixHandler[0x78] = std::bind(&CPULR35902::PR_78, this);
    prefixHandler[0x79] = std::bind(&CPULR35902::PR_79, this);
    prefixHandler[0x7A] = std::bind(&CPULR35902::PR_7A, this);
    prefixHandler[0x7B] = std::bind(&CPULR35902::PR_7B, this);
    prefixHandler[0x7C] = std::bind(&CPULR35902::PR_7C, this);
    prefixHandler[0x7D] = std::bind(&CPULR35902::PR_7D, this);
    prefixHandler[0x7E] = std::bind(&CPULR35902::PR_7E, this);
    prefixHandler[0x7F] = std::bind(&CPULR35902::PR_7F, this);
    prefixHandler[0x80] = std::bind(&CPULR35902::PR_80, this);
    prefixHandler[0x81] = std::bind(&CPULR35902::PR_81, this);
    prefixHandler[0x82] = std::bind(&CPULR35902::PR_82, this);
    prefixHandler[0x83] = std::bind(&CPULR35902::PR_83, this);
    prefixHandler[0x84] = std::bind(&CPULR35902::PR_84, this);
    prefixHandler[0x85] = std::bind(&CPULR35902::PR_85, this);
    prefixHandler[0x86] = std::bind(&CPULR35902::PR_86, this);
    prefixHandler[0x87] = std::bind(&CPULR35902::PR_87, this);
    prefixHandler[0x88] = std::bind(&CPULR35902::PR_88, this);
    prefixHandler[0x89] = std::bind(&CPULR35902::PR_89, this);
    prefixHandler[0x8A] = std::bind(&CPULR35902::PR_8A, this);
    prefixHandler[0x8B] = std::bind(&CPULR35902::PR_8B, this);
    prefixHandler[0x8C] = std::bind(&CPULR35902::PR_8C, this);
    prefixHandler[0x8D] = std::bind(&CPULR35902::PR_8D, this);
    prefixHandler[0x8E] = std::bind(&CPULR35902::PR_8E, this);
    prefixHandler[0x8F] = std::bind(&CPULR35902::PR_8F, this);
    prefixHandler[0x90] = std::bind(&CPULR35902::PR_90, this);
    prefixHandler[0x91] = std::bind(&CPULR35902::PR_91, this);
    prefixHandler[0x92] = std::bind(&CPULR35902::PR_92, this);
    prefixHandler[0x93] = std::bind(&CPULR35902::PR_93, this);
    prefixHandler[0x94] = std::bind(&CPULR35902::PR_94, this);
    prefixHandler[0x95] = std::bind(&CPULR35902::PR_95, this);
    prefixHandler[0x96] = std::bind(&CPULR35902::PR_96, this);
    prefixHandler[0x97] = std::bind(&CPULR35902::PR_97, this);
    prefixHandler[0x98] = std::bind(&CPULR35902::PR_98, this);
    prefixHandler[0x99] = std::bind(&CPULR35902::PR_99, this);
    prefixHandler[0x9A] = std::bind(&CPULR35902::PR_9A, this);
    prefixHandler[0x9B] = std::bind(&CPULR35902::PR_9B, this);
    prefixHandler[0x9C] = std::bind(&CPULR35902::PR_9C, this);
    prefixHandler[0x9D] = std::bind(&CPULR35902::PR_9D, this);
    prefixHandler[0x9E] = std::bind(&CPULR35902::PR_9E, this);
    prefixHandler[0x9F] = std::bind(&CPULR35902::PR_9F, this);
    prefixHandler[0xA0] = std::bind(&CPULR35902::PR_A0, this);
    prefixHandler[0xA1] = std::bind(&CPULR35902::PR_A1, this);
    prefixHandler[0xA2] = std::bind(&CPULR35902::PR_A2, this);
    prefixHandler[0xA3] = std::bind(&CPULR35902::PR_A3, this);
    prefixHandler[0xA4] = std::bind(&CPULR35902::PR_A4, this);
    prefixHandler[0xA5] = std::bind(&CPULR35902::PR_A5, this);
    prefixHandler[0xA6] = std::bind(&CPULR35902::PR_A6, this);
    prefixHandler[0xA7] = std::bind(&CPULR35902::PR_A7, this);
    prefixHandler[0xA8] = std::bind(&CPULR35902::PR_A8, this);
    prefixHandler[0xA9] = std::bind(&CPULR35902::PR_A9, this);
    prefixHandler[0xAA] = std::bind(&CPULR35902::PR_AA, this);
    prefixHandler[0xAB] = std::bind(&CPULR35902::PR_AB, this);
    prefixHandler[0xAC] = std::bind(&CPULR35902::PR_AC, this);
    prefixHandler[0xAD] = std::bind(&CPULR35902::PR_AD, this);
    prefixHandler[0xAE] = std::bind(&CPULR35902::PR_AE, this);
    prefixHandler[0xAF] = std::bind(&CPULR35902::PR_AF, this);
    prefixHandler[0xB0] = std::bind(&CPULR35902::PR_B0, this);
    prefixHandler[0xB1] = std::bind(&CPULR35902::PR_B1, this);
    prefixHandler[0xB2] = std::bind(&CPULR35902::PR_B2, this);
    prefixHandler[0xB3] = std::bind(&CPULR35902::PR_B3, this);
    prefixHandler[0xB4] = std::bind(&CPULR35902::PR_B4, this);
    prefixHandler[0xB5] = std::bind(&CPULR35902::PR_B5, this);
    prefixHandler[0xB6] = std::bind(&CPULR35902::PR_B6, this);
    prefixHandler[0xB7] = std::bind(&CPULR35902::PR_B7, this);
    prefixHandler[0xB8] = std::bind(&CPULR35902::PR_B8, this);
    prefixHandler[0xB9] = std::bind(&CPULR35902::PR_B9, this);
    prefixHandler[0xBA] = std::bind(&CPULR35902::PR_BA, this);
    prefixHandler[0xBB] = std::bind(&CPULR35902::PR_BB, this);
    prefixHandler[0xBC] = std::bind(&CPULR35902::PR_BC, this);
    prefixHandler[0xBD] = std::bind(&CPULR35902::PR_BD, this);
    prefixHandler[0xBE] = std::bind(&CPULR35902::PR_BE, this);
    prefixHandler[0xBF] = std::bind(&CPULR35902::PR_BF, this);
    prefixHandler[0xC0] = std::bind(&CPULR35902::PR_C0, this);
    prefixHandler[0xC1] = std::bind(&CPULR35902::PR_C1, this);
    prefixHandler[0xC2] = std::bind(&CPULR35902::PR_C2, this);
    prefixHandler[0xC3] = std::bind(&CPULR35902::PR_C3, this);
    prefixHandler[0xC4] = std::bind(&CPULR35902::PR_C4, this);
    prefixHandler[0xC5] = std::bind(&CPULR35902::PR_C5, this);
    prefixHandler[0xC6] = std::bind(&CPULR35902::PR_C6, this);
    prefixHandler[0xC7] = std::bind(&CPULR35902::PR_C7, this);
    prefixHandler[0xC8] = std::bind(&CPULR35902::PR_C8, this);
    prefixHandler[0xC9] = std::bind(&CPULR35902::PR_C9, this);
    prefixHandler[0xCA] = std::bind(&CPULR35902::PR_CA, this);
    prefixHandler[0xCB] = std::bind(&CPULR35902::PR_CB, this);
    prefixHandler[0xCC] = std::bind(&CPULR35902::PR_CC, this);
    prefixHandler[0xCD] = std::bind(&CPULR35902::PR_CD, this);
    prefixHandler[0xCE] = std::bind(&CPULR35902::PR_CE, this);
    prefixHandler[0xCF] = std::bind(&CPULR35902::PR_CF, this);
    prefixHandler[0xD0] = std::bind(&CPULR35902::PR_D0, this);
    prefixHandler[0xD1] = std::bind(&CPULR35902::PR_D1, this);
    prefixHandler[0xD2] = std::bind(&CPULR35902::PR_D2, this);
    prefixHandler[0xD3] = std::bind(&CPULR35902::PR_D3, this);
    prefixHandler[0xD4] = std::bind(&CPULR35902::PR_D4, this);
    prefixHandler[0xD5] = std::bind(&CPULR35902::PR_D5, this);
    prefixHandler[0xD6] = std::bind(&CPULR35902::PR_D6, this);
    prefixHandler[0xD7] = std::bind(&CPULR35902::PR_D7, this);
    prefixHandler[0xD8] = std::bind(&CPULR35902::PR_D8, this);
    prefixHandler[0xD9] = std::bind(&CPULR35902::PR_D9, this);
    prefixHandler[0xDA] = std::bind(&CPULR35902::PR_DA, this);
    prefixHandler[0xDB] = std::bind(&CPULR35902::PR_DB, this);
    prefixHandler[0xDC] = std::bind(&CPULR35902::PR_DC, this);
    prefixHandler[0xDD] = std::bind(&CPULR35902::PR_DD, this);
    prefixHandler[0xDE] = std::bind(&CPULR35902::PR_DE, this);
    prefixHandler[0xDF] = std::bind(&CPULR35902::PR_DF, this);
    prefixHandler[0xE0] = std::bind(&CPULR35902::PR_E0, this);
    prefixHandler[0xE1] = std::bind(&CPULR35902::PR_E1, this);
    prefixHandler[0xE2] = std::bind(&CPULR35902::PR_E2, this);
    prefixHandler[0xE3] = std::bind(&CPULR35902::PR_E3, this);
    prefixHandler[0xE4] = std::bind(&CPULR35902::PR_E4, this);
    prefixHandler[0xE5] = std::bind(&CPULR35902::PR_E5, this);
    prefixHandler[0xE6] = std::bind(&CPULR35902::PR_E6, this);
    prefixHandler[0xE7] = std::bind(&CPULR35902::PR_E7, this);
    prefixHandler[0xE8] = std::bind(&CPULR35902::PR_E8, this);
    prefixHandler[0xE9] = std::bind(&CPULR35902::PR_E9, this);
    prefixHandler[0xEA] = std::bind(&CPULR35902::PR_EA, this);
    prefixHandler[0xEB] = std::bind(&CPULR35902::PR_EB, this);
    prefixHandler[0xEC] = std::bind(&CPULR35902::PR_EC, this);
    prefixHandler[0xED] = std::bind(&CPULR35902::PR_ED, this);
    prefixHandler[0xEE] = std::bind(&CPULR35902::PR_EE, this);
    prefixHandler[0xEF] = std::bind(&CPULR35902::PR_EF, this);
    prefixHandler[0xF0] = std::bind(&CPULR35902::PR_F0, this);
    prefixHandler[0xF1] = std::bind(&CPULR35902::PR_F1, this);
    prefixHandler[0xF2] = std::bind(&CPULR35902::PR_F2, this);
    prefixHandler[0xF3] = std::bind(&CPULR35902::PR_F3, this);
    prefixHandler[0xF4] = std::bind(&CPULR35902::PR_F4, this);
    prefixHandler[0xF5] = std::bind(&CPULR35902::PR_F5, this);
    prefixHandler[0xF6] = std::bind(&CPULR35902::PR_F6, this);
    prefixHandler[0xF7] = std::bind(&CPULR35902::PR_F7, this);
    prefixHandler[0xF8] = std::bind(&CPULR35902::PR_F8, this);
    prefixHandler[0xF9] = std::bind(&CPULR35902::PR_F9, this);
    prefixHandler[0xFA] = std::bind(&CPULR35902::PR_FA, this);
    prefixHandler[0xFB] = std::bind(&CPULR35902::PR_FB, this);
    prefixHandler[0xFC] = std::bind(&CPULR35902::PR_FC, this);
    prefixHandler[0xFD] = std::bind(&CPULR35902::PR_FD, this);
    prefixHandler[0xFE] = std::bind(&CPULR35902::PR_FE, this);
    prefixHandler[0xFF] = std::bind(&CPULR35902::PR_FF, this);
}

