#include "CPULR35902.hpp"

#include "Bus.hpp"

#include <iostream>

CPULR35902::CPULR35902(Bus* bus) : bus(bus) {
    reset();  
}

CPULR35902::~CPULR35902() {

}

void CPULR35902::reset() {
    AF.w = 0;
    BC.w = 0;
    DE.w = 0;
    HL.w = 0;
    SP.w = 0;
    PC.w = 0;
}

void CPULR35902::clearFlags() {
    AF.lsb &= 0x0F;
}

void CPULR35902::setFlag(Flag flag) {
    switch(flag) {
	case Flag::Z : { AF.lsb = AF.lsb | 0b10000000; break; }
	case Flag::N : { AF.lsb = AF.lsb | 0b01000000; break; }
	case Flag::H : { AF.lsb = AF.lsb | 0b00100000; break; }
	case Flag::C : { AF.lsb = AF.lsb | 0b00010000; break; }
        default : { throw std::runtime_error("Invalid flag set!"); }
    }
}

bool CPULR35902::getFlag(Flag flag) {
    switch(flag) {
	case Flag::Z : { return static_cast<bool>(AF.lsb & 0b10000000); }
	case Flag::N : { return static_cast<bool>(AF.lsb & 0b01000000); }
	case Flag::H : { return static_cast<bool>(AF.lsb & 0b00100000); }
	case Flag::C : { return static_cast<bool>(AF.lsb & 0b00010000); }
        default : { throw std::runtime_error("Invalid flag get!"); }
    }
}


void CPULR35902::fetchDecodeExecute() {
    const auto instruction = bus->read<uint8_t>(PC.w);
    PC.w++;

    std::cout << static_cast<int>(instruction) << std::endl;
}


