#include "CPULR35902.hpp"

#include "Bus.hpp"

#include <iostream>

CPULR35902::CPULR35902(Bus* bus) : bus(bus) {
    initOpcodeHandlers();
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

    if(instruction == 0xCB) {
        const auto prefixInstruction = bus->read<uint8_t>(PC.w);
        PC.w++;
        prefixHandler[prefixInstruction];
    }
    else {
        opcodeHandler[instruction];
    }
}

void CPULR35902::OP_00() {}
void CPULR35902::OP_01() {}
void CPULR35902::OP_02() {}
void CPULR35902::OP_03() {}
void CPULR35902::OP_04() {}
void CPULR35902::OP_05() {}
void CPULR35902::OP_06() {}
void CPULR35902::OP_07() {}
void CPULR35902::OP_08() {}
void CPULR35902::OP_09() {}
void CPULR35902::OP_0A() {}
void CPULR35902::OP_0B() {}
void CPULR35902::OP_0C() {}
void CPULR35902::OP_0D() {}
void CPULR35902::OP_0E() {}
void CPULR35902::OP_0F() {}
void CPULR35902::OP_10() {}
void CPULR35902::OP_11() {}
void CPULR35902::OP_12() {}
void CPULR35902::OP_13() {}
void CPULR35902::OP_14() {}
void CPULR35902::OP_15() {}
void CPULR35902::OP_16() {}
void CPULR35902::OP_17() {}
void CPULR35902::OP_18() {}
void CPULR35902::OP_19() {}
void CPULR35902::OP_1A() {}
void CPULR35902::OP_1B() {}
void CPULR35902::OP_1C() {}
void CPULR35902::OP_1D() {}
void CPULR35902::OP_1E() {}
void CPULR35902::OP_1F() {}
void CPULR35902::OP_20() {}
void CPULR35902::OP_21() {}
void CPULR35902::OP_22() {}
void CPULR35902::OP_23() {}
void CPULR35902::OP_24() {}
void CPULR35902::OP_25() {}
void CPULR35902::OP_26() {}
void CPULR35902::OP_27() {}
void CPULR35902::OP_28() {}
void CPULR35902::OP_29() {}
void CPULR35902::OP_2A() {}
void CPULR35902::OP_2B() {}
void CPULR35902::OP_2C() {}
void CPULR35902::OP_2D() {}
void CPULR35902::OP_2E() {}
void CPULR35902::OP_2F() {}
void CPULR35902::OP_30() {}
void CPULR35902::OP_31() {}
void CPULR35902::OP_32() {}
void CPULR35902::OP_33() {}
void CPULR35902::OP_34() {}
void CPULR35902::OP_35() {}
void CPULR35902::OP_36() {}
void CPULR35902::OP_37() {}
void CPULR35902::OP_38() {}
void CPULR35902::OP_39() {}
void CPULR35902::OP_3A() {}
void CPULR35902::OP_3B() {}
void CPULR35902::OP_3C() {}
void CPULR35902::OP_3D() {}
void CPULR35902::OP_3E() {}
void CPULR35902::OP_3F() {}
void CPULR35902::OP_40() {}
void CPULR35902::OP_41() {}
void CPULR35902::OP_42() {}
void CPULR35902::OP_43() {}
void CPULR35902::OP_44() {}
void CPULR35902::OP_45() {}
void CPULR35902::OP_46() {}
void CPULR35902::OP_47() {}
void CPULR35902::OP_48() {}
void CPULR35902::OP_49() {}
void CPULR35902::OP_4A() {}
void CPULR35902::OP_4B() {}
void CPULR35902::OP_4C() {}
void CPULR35902::OP_4D() {}
void CPULR35902::OP_4E() {}
void CPULR35902::OP_4F() {}
void CPULR35902::OP_50() {}
void CPULR35902::OP_51() {}
void CPULR35902::OP_52() {}
void CPULR35902::OP_53() {}
void CPULR35902::OP_54() {}
void CPULR35902::OP_55() {}
void CPULR35902::OP_56() {}
void CPULR35902::OP_57() {}
void CPULR35902::OP_58() {}
void CPULR35902::OP_59() {}
void CPULR35902::OP_5A() {}
void CPULR35902::OP_5B() {}
void CPULR35902::OP_5C() {}
void CPULR35902::OP_5D() {}
void CPULR35902::OP_5E() {}
void CPULR35902::OP_5F() {}
void CPULR35902::OP_60() {}
void CPULR35902::OP_61() {}
void CPULR35902::OP_62() {}
void CPULR35902::OP_63() {}
void CPULR35902::OP_64() {}
void CPULR35902::OP_65() {}
void CPULR35902::OP_66() {}
void CPULR35902::OP_67() {}
void CPULR35902::OP_68() {}
void CPULR35902::OP_69() {}
void CPULR35902::OP_6A() {}
void CPULR35902::OP_6B() {}
void CPULR35902::OP_6C() {}
void CPULR35902::OP_6D() {}
void CPULR35902::OP_6E() {}
void CPULR35902::OP_6F() {}
void CPULR35902::OP_70() {}
void CPULR35902::OP_71() {}
void CPULR35902::OP_72() {}
void CPULR35902::OP_73() {}
void CPULR35902::OP_74() {}
void CPULR35902::OP_75() {}
void CPULR35902::OP_76() {}
void CPULR35902::OP_77() {}
void CPULR35902::OP_78() {}
void CPULR35902::OP_79() {}
void CPULR35902::OP_7A() {}
void CPULR35902::OP_7B() {}
void CPULR35902::OP_7C() {}
void CPULR35902::OP_7D() {}
void CPULR35902::OP_7E() {}
void CPULR35902::OP_7F() {}
void CPULR35902::OP_80() {}
void CPULR35902::OP_81() {}
void CPULR35902::OP_82() {}
void CPULR35902::OP_83() {}
void CPULR35902::OP_84() {}
void CPULR35902::OP_85() {}
void CPULR35902::OP_86() {}
void CPULR35902::OP_87() {}
void CPULR35902::OP_88() {}
void CPULR35902::OP_89() {}
void CPULR35902::OP_8A() {}
void CPULR35902::OP_8B() {}
void CPULR35902::OP_8C() {}
void CPULR35902::OP_8D() {}
void CPULR35902::OP_8E() {}
void CPULR35902::OP_8F() {}
void CPULR35902::OP_90() {}
void CPULR35902::OP_91() {}
void CPULR35902::OP_92() {}
void CPULR35902::OP_93() {}
void CPULR35902::OP_94() {}
void CPULR35902::OP_95() {}
void CPULR35902::OP_96() {}
void CPULR35902::OP_97() {}
void CPULR35902::OP_98() {}
void CPULR35902::OP_99() {}
void CPULR35902::OP_9A() {}
void CPULR35902::OP_9B() {}
void CPULR35902::OP_9C() {}
void CPULR35902::OP_9D() {}
void CPULR35902::OP_9E() {}
void CPULR35902::OP_9F() {}
void CPULR35902::OP_A0() {}
void CPULR35902::OP_A1() {}
void CPULR35902::OP_A2() {}
void CPULR35902::OP_A3() {}
void CPULR35902::OP_A4() {}
void CPULR35902::OP_A5() {}
void CPULR35902::OP_A6() {}
void CPULR35902::OP_A7() {}
void CPULR35902::OP_A8() {}
void CPULR35902::OP_A9() {}
void CPULR35902::OP_AA() {}
void CPULR35902::OP_AB() {}
void CPULR35902::OP_AC() {}
void CPULR35902::OP_AD() {}
void CPULR35902::OP_AE() {}
void CPULR35902::OP_AF() {}
void CPULR35902::OP_B0() {}
void CPULR35902::OP_B1() {}
void CPULR35902::OP_B2() {}
void CPULR35902::OP_B3() {}
void CPULR35902::OP_B4() {}
void CPULR35902::OP_B5() {}
void CPULR35902::OP_B6() {}
void CPULR35902::OP_B7() {}
void CPULR35902::OP_B8() {}
void CPULR35902::OP_B9() {}
void CPULR35902::OP_BA() {}
void CPULR35902::OP_BB() {}
void CPULR35902::OP_BC() {}
void CPULR35902::OP_BD() {}
void CPULR35902::OP_BE() {}
void CPULR35902::OP_BF() {}
void CPULR35902::OP_C0() {}
void CPULR35902::OP_C1() {}
void CPULR35902::OP_C2() {}
void CPULR35902::OP_C3() {}
void CPULR35902::OP_C4() {}
void CPULR35902::OP_C5() {}
void CPULR35902::OP_C6() {}
void CPULR35902::OP_C7() {}
void CPULR35902::OP_C8() {}
void CPULR35902::OP_C9() {}
void CPULR35902::OP_CA() {}
void CPULR35902::OP_CB() {}
void CPULR35902::OP_CC() {}
void CPULR35902::OP_CD() {}
void CPULR35902::OP_CE() {}
void CPULR35902::OP_CF() {}
void CPULR35902::OP_D0() {}
void CPULR35902::OP_D1() {}
void CPULR35902::OP_D2() {}
void CPULR35902::OP_D3() {}
void CPULR35902::OP_D4() {}
void CPULR35902::OP_D5() {}
void CPULR35902::OP_D6() {}
void CPULR35902::OP_D7() {}
void CPULR35902::OP_D8() {}
void CPULR35902::OP_D9() {}
void CPULR35902::OP_DA() {}
void CPULR35902::OP_DB() {}
void CPULR35902::OP_DC() {}
void CPULR35902::OP_DD() {}
void CPULR35902::OP_DE() {}
void CPULR35902::OP_DF() {}
void CPULR35902::OP_E0() {}
void CPULR35902::OP_E1() {}
void CPULR35902::OP_E2() {}
void CPULR35902::OP_E3() {}
void CPULR35902::OP_E4() {}
void CPULR35902::OP_E5() {}
void CPULR35902::OP_E6() {}
void CPULR35902::OP_E7() {}
void CPULR35902::OP_E8() {}
void CPULR35902::OP_E9() {}
void CPULR35902::OP_EA() {}
void CPULR35902::OP_EB() {}
void CPULR35902::OP_EC() {}
void CPULR35902::OP_ED() {}
void CPULR35902::OP_EE() {}
void CPULR35902::OP_EF() {}
void CPULR35902::OP_F0() {}
void CPULR35902::OP_F1() {}
void CPULR35902::OP_F2() {}
void CPULR35902::OP_F3() {}
void CPULR35902::OP_F4() {}
void CPULR35902::OP_F5() {}
void CPULR35902::OP_F6() {}
void CPULR35902::OP_F7() {}
void CPULR35902::OP_F8() {}
void CPULR35902::OP_F9() {}
void CPULR35902::OP_FA() {}
void CPULR35902::OP_FB() {}
void CPULR35902::OP_FC() {}
void CPULR35902::OP_FD() {}
void CPULR35902::OP_FE() {}
void CPULR35902::OP_FF() {}

void CPULR35902::PR_00() {}
void CPULR35902::PR_01() {}
void CPULR35902::PR_02() {}
void CPULR35902::PR_03() {}
void CPULR35902::PR_04() {}
void CPULR35902::PR_05() {}
void CPULR35902::PR_06() {}
void CPULR35902::PR_07() {}
void CPULR35902::PR_08() {}
void CPULR35902::PR_09() {}
void CPULR35902::PR_0A() {}
void CPULR35902::PR_0B() {}
void CPULR35902::PR_0C() {}
void CPULR35902::PR_0D() {}
void CPULR35902::PR_0E() {}
void CPULR35902::PR_0F() {}
void CPULR35902::PR_10() {}
void CPULR35902::PR_11() {}
void CPULR35902::PR_12() {}
void CPULR35902::PR_13() {}
void CPULR35902::PR_14() {}
void CPULR35902::PR_15() {}
void CPULR35902::PR_16() {}
void CPULR35902::PR_17() {}
void CPULR35902::PR_18() {}
void CPULR35902::PR_19() {}
void CPULR35902::PR_1A() {}
void CPULR35902::PR_1B() {}
void CPULR35902::PR_1C() {}
void CPULR35902::PR_1D() {}
void CPULR35902::PR_1E() {}
void CPULR35902::PR_1F() {}
void CPULR35902::PR_20() {}
void CPULR35902::PR_21() {}
void CPULR35902::PR_22() {}
void CPULR35902::PR_23() {}
void CPULR35902::PR_24() {}
void CPULR35902::PR_25() {}
void CPULR35902::PR_26() {}
void CPULR35902::PR_27() {}
void CPULR35902::PR_28() {}
void CPULR35902::PR_29() {}
void CPULR35902::PR_2A() {}
void CPULR35902::PR_2B() {}
void CPULR35902::PR_2C() {}
void CPULR35902::PR_2D() {}
void CPULR35902::PR_2E() {}
void CPULR35902::PR_2F() {}
void CPULR35902::PR_30() {}
void CPULR35902::PR_31() {}
void CPULR35902::PR_32() {}
void CPULR35902::PR_33() {}
void CPULR35902::PR_34() {}
void CPULR35902::PR_35() {}
void CPULR35902::PR_36() {}
void CPULR35902::PR_37() {}
void CPULR35902::PR_38() {}
void CPULR35902::PR_39() {}
void CPULR35902::PR_3A() {}
void CPULR35902::PR_3B() {}
void CPULR35902::PR_3C() {}
void CPULR35902::PR_3D() {}
void CPULR35902::PR_3E() {}
void CPULR35902::PR_3F() {}
void CPULR35902::PR_40() {}
void CPULR35902::PR_41() {}
void CPULR35902::PR_42() {}
void CPULR35902::PR_43() {}
void CPULR35902::PR_44() {}
void CPULR35902::PR_45() {}
void CPULR35902::PR_46() {}
void CPULR35902::PR_47() {}
void CPULR35902::PR_48() {}
void CPULR35902::PR_49() {}
void CPULR35902::PR_4A() {}
void CPULR35902::PR_4B() {}
void CPULR35902::PR_4C() {}
void CPULR35902::PR_4D() {}
void CPULR35902::PR_4E() {}
void CPULR35902::PR_4F() {}
void CPULR35902::PR_50() {}
void CPULR35902::PR_51() {}
void CPULR35902::PR_52() {}
void CPULR35902::PR_53() {}
void CPULR35902::PR_54() {}
void CPULR35902::PR_55() {}
void CPULR35902::PR_56() {}
void CPULR35902::PR_57() {}
void CPULR35902::PR_58() {}
void CPULR35902::PR_59() {}
void CPULR35902::PR_5A() {}
void CPULR35902::PR_5B() {}
void CPULR35902::PR_5C() {}
void CPULR35902::PR_5D() {}
void CPULR35902::PR_5E() {}
void CPULR35902::PR_5F() {}
void CPULR35902::PR_60() {}
void CPULR35902::PR_61() {}
void CPULR35902::PR_62() {}
void CPULR35902::PR_63() {}
void CPULR35902::PR_64() {}
void CPULR35902::PR_65() {}
void CPULR35902::PR_66() {}
void CPULR35902::PR_67() {}
void CPULR35902::PR_68() {}
void CPULR35902::PR_69() {}
void CPULR35902::PR_6A() {}
void CPULR35902::PR_6B() {}
void CPULR35902::PR_6C() {}
void CPULR35902::PR_6D() {}
void CPULR35902::PR_6E() {}
void CPULR35902::PR_6F() {}
void CPULR35902::PR_70() {}
void CPULR35902::PR_71() {}
void CPULR35902::PR_72() {}
void CPULR35902::PR_73() {}
void CPULR35902::PR_74() {}
void CPULR35902::PR_75() {}
void CPULR35902::PR_76() {}
void CPULR35902::PR_77() {}
void CPULR35902::PR_78() {}
void CPULR35902::PR_79() {}
void CPULR35902::PR_7A() {}
void CPULR35902::PR_7B() {}
void CPULR35902::PR_7C() {}
void CPULR35902::PR_7D() {}
void CPULR35902::PR_7E() {}
void CPULR35902::PR_7F() {}
void CPULR35902::PR_80() {}
void CPULR35902::PR_81() {}
void CPULR35902::PR_82() {}
void CPULR35902::PR_83() {}
void CPULR35902::PR_84() {}
void CPULR35902::PR_85() {}
void CPULR35902::PR_86() {}
void CPULR35902::PR_87() {}
void CPULR35902::PR_88() {}
void CPULR35902::PR_89() {}
void CPULR35902::PR_8A() {}
void CPULR35902::PR_8B() {}
void CPULR35902::PR_8C() {}
void CPULR35902::PR_8D() {}
void CPULR35902::PR_8E() {}
void CPULR35902::PR_8F() {}
void CPULR35902::PR_90() {}
void CPULR35902::PR_91() {}
void CPULR35902::PR_92() {}
void CPULR35902::PR_93() {}
void CPULR35902::PR_94() {}
void CPULR35902::PR_95() {}
void CPULR35902::PR_96() {}
void CPULR35902::PR_97() {}
void CPULR35902::PR_98() {}
void CPULR35902::PR_99() {}
void CPULR35902::PR_9A() {}
void CPULR35902::PR_9B() {}
void CPULR35902::PR_9C() {}
void CPULR35902::PR_9D() {}
void CPULR35902::PR_9E() {}
void CPULR35902::PR_9F() {}
void CPULR35902::PR_A0() {}
void CPULR35902::PR_A1() {}
void CPULR35902::PR_A2() {}
void CPULR35902::PR_A3() {}
void CPULR35902::PR_A4() {}
void CPULR35902::PR_A5() {}
void CPULR35902::PR_A6() {}
void CPULR35902::PR_A7() {}
void CPULR35902::PR_A8() {}
void CPULR35902::PR_A9() {}
void CPULR35902::PR_AA() {}
void CPULR35902::PR_AB() {}
void CPULR35902::PR_AC() {}
void CPULR35902::PR_AD() {}
void CPULR35902::PR_AE() {}
void CPULR35902::PR_AF() {}
void CPULR35902::PR_B0() {}
void CPULR35902::PR_B1() {}
void CPULR35902::PR_B2() {}
void CPULR35902::PR_B3() {}
void CPULR35902::PR_B4() {}
void CPULR35902::PR_B5() {}
void CPULR35902::PR_B6() {}
void CPULR35902::PR_B7() {}
void CPULR35902::PR_B8() {}
void CPULR35902::PR_B9() {}
void CPULR35902::PR_BA() {}
void CPULR35902::PR_BB() {}
void CPULR35902::PR_BC() {}
void CPULR35902::PR_BD() {}
void CPULR35902::PR_BE() {}
void CPULR35902::PR_BF() {}
void CPULR35902::PR_C0() {}
void CPULR35902::PR_C1() {}
void CPULR35902::PR_C2() {}
void CPULR35902::PR_C3() {}
void CPULR35902::PR_C4() {}
void CPULR35902::PR_C5() {}
void CPULR35902::PR_C6() {}
void CPULR35902::PR_C7() {}
void CPULR35902::PR_C8() {}
void CPULR35902::PR_C9() {}
void CPULR35902::PR_CA() {}
void CPULR35902::PR_CB() {}
void CPULR35902::PR_CC() {}
void CPULR35902::PR_CD() {}
void CPULR35902::PR_CE() {}
void CPULR35902::PR_CF() {}
void CPULR35902::PR_D0() {}
void CPULR35902::PR_D1() {}
void CPULR35902::PR_D2() {}
void CPULR35902::PR_D3() {}
void CPULR35902::PR_D4() {}
void CPULR35902::PR_D5() {}
void CPULR35902::PR_D6() {}
void CPULR35902::PR_D7() {}
void CPULR35902::PR_D8() {}
void CPULR35902::PR_D9() {}
void CPULR35902::PR_DA() {}
void CPULR35902::PR_DB() {}
void CPULR35902::PR_DC() {}
void CPULR35902::PR_DD() {}
void CPULR35902::PR_DE() {}
void CPULR35902::PR_DF() {}
void CPULR35902::PR_E0() {}
void CPULR35902::PR_E1() {}
void CPULR35902::PR_E2() {}
void CPULR35902::PR_E3() {}
void CPULR35902::PR_E4() {}
void CPULR35902::PR_E5() {}
void CPULR35902::PR_E6() {}
void CPULR35902::PR_E7() {}
void CPULR35902::PR_E8() {}
void CPULR35902::PR_E9() {}
void CPULR35902::PR_EA() {}
void CPULR35902::PR_EB() {}
void CPULR35902::PR_EC() {}
void CPULR35902::PR_ED() {}
void CPULR35902::PR_EE() {}
void CPULR35902::PR_EF() {}
void CPULR35902::PR_F0() {}
void CPULR35902::PR_F1() {}
void CPULR35902::PR_F2() {}
void CPULR35902::PR_F3() {}
void CPULR35902::PR_F4() {}
void CPULR35902::PR_F5() {}
void CPULR35902::PR_F6() {}
void CPULR35902::PR_F7() {}
void CPULR35902::PR_F8() {}
void CPULR35902::PR_F9() {}
void CPULR35902::PR_FA() {}
void CPULR35902::PR_FB() {}
void CPULR35902::PR_FC() {}
void CPULR35902::PR_FD() {}
void CPULR35902::PR_FE() {}
void CPULR35902::PR_FF() {}

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

