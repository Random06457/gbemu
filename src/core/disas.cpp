#include <string>
#include "disas.hpp"
#include "common/logging.hpp"
#include "opcode.hpp"

namespace gbemu::core
{


std::string Disas::disassemble(const void* data, size_t size)
{
    Disas disas{data, size};
    return disas.disassemble();
}

std::string Disas::disassemble()
{
    u8 op = read8();

    // auto generated, see optable_gen.py
    switch(op)
    {
        case OP_NOP: return "NOP ";
        case OP_LD_BC_d16: return fmt::format("LD BC, ${:04X}", read16());
        case OP_LD_MEM_BC_A: return "LD (BC), A";
        case OP_INC_BC: return "INC BC";
        case OP_INC_B: return "INC B";
        case OP_DEC_B: return "DEC B";
        case OP_LD_B_d8: return fmt::format("LD B, {:02X}", read8());
        case OP_RLCA: return "RLCA ";
        case OP_LD_MEM_a16_SP: return fmt::format("LD (${:04X}), SP", read16());
        case OP_ADD_HL_BC: return "ADD HL, BC";
        case OP_LD_A_MEM_BC: return "LD A, (BC)";
        case OP_DEC_BC: return "DEC BC";
        case OP_INC_C: return "INC C";
        case OP_DEC_C: return "DEC C";
        case OP_LD_C_d8: return fmt::format("LD C, {:02X}", read8());
        case OP_RRCA: return "RRCA ";
        case OP_STOP_d8: return fmt::format("STOP {:02X}", read8());
        case OP_LD_DE_d16: return fmt::format("LD DE, ${:04X}", read16());
        case OP_LD_MEM_DE_A: return "LD (DE), A";
        case OP_INC_DE: return "INC DE";
        case OP_INC_D: return "INC D";
        case OP_DEC_D: return "DEC D";
        case OP_LD_D_d8: return fmt::format("LD D, {:02X}", read8());
        case OP_RLA: return "RLA ";
        case OP_JR_r8: return fmt::format("JR {:02X}", read8());
        case OP_ADD_HL_DE: return "ADD HL, DE";
        case OP_LD_A_MEM_DE: return "LD A, (DE)";
        case OP_DEC_DE: return "DEC DE";
        case OP_INC_E: return "INC E";
        case OP_DEC_E: return "DEC E";
        case OP_LD_E_d8: return fmt::format("LD E, {:02X}", read8());
        case OP_RRA: return "RRA ";
        case OP_JR_NZ_r8: return fmt::format("JR NZ, {:02X}", read8());
        case OP_LD_HL_d16: return fmt::format("LD HL, ${:04X}", read16());
        case OP_LD_MEM_HLI_A: return "LD (HL+), A";
        case OP_INC_HL: return "INC HL";
        case OP_INC_H: return "INC H";
        case OP_DEC_H: return "DEC H";
        case OP_LD_H_d8: return fmt::format("LD H, {:02X}", read8());
        case OP_DAA: return "DAA ";
        case OP_JR_Z_r8: return fmt::format("JR Z, {:02X}", read8());
        case OP_ADD_HL_HL: return "ADD HL, HL";
        case OP_LD_A_MEM_HLI: return "LD A, (HL+)";
        case OP_DEC_HL: return "DEC HL";
        case OP_INC_L: return "INC L";
        case OP_DEC_L: return "DEC L";
        case OP_LD_L_d8: return fmt::format("LD L, {:02X}", read8());
        case OP_CPL: return "CPL ";
        case OP_JR_NC_r8: return fmt::format("JR NC, {:02X}", read8());
        case OP_LD_SP_d16: return fmt::format("LD SP, ${:04X}", read16());
        case OP_LD_MEM_HLD_A: return "LD (HL-), A";
        case OP_INC_SP: return "INC SP";
        case OP_INC_MEM_HL: return "INC (HL)";
        case OP_DEC_MEM_HL: return "DEC (HL)";
        case OP_LD_MEM_HL_d8: return fmt::format("LD (HL), {:02X}", read8());
        case OP_SCF: return "SCF ";
        case OP_JR_C_r8: return fmt::format("JR C, {:02X}", read8());
        case OP_ADD_HL_SP: return "ADD HL, SP";
        case OP_LD_A_MEM_HLD: return "LD A, (HL-)";
        case OP_DEC_SP: return "DEC SP";
        case OP_INC_A: return "INC A";
        case OP_DEC_A: return "DEC A";
        case OP_LD_A_d8: return fmt::format("LD A, {:02X}", read8());
        case OP_CCF: return "CCF ";
        case OP_LD_B_B: return "LD B, B";
        case OP_LD_B_C: return "LD B, C";
        case OP_LD_B_D: return "LD B, D";
        case OP_LD_B_E: return "LD B, E";
        case OP_LD_B_H: return "LD B, H";
        case OP_LD_B_L: return "LD B, L";
        case OP_LD_B_MEM_HL: return "LD B, (HL)";
        case OP_LD_B_A: return "LD B, A";
        case OP_LD_C_B: return "LD C, B";
        case OP_LD_C_C: return "LD C, C";
        case OP_LD_C_D: return "LD C, D";
        case OP_LD_C_E: return "LD C, E";
        case OP_LD_C_H: return "LD C, H";
        case OP_LD_C_L: return "LD C, L";
        case OP_LD_C_MEM_HL: return "LD C, (HL)";
        case OP_LD_C_A: return "LD C, A";
        case OP_LD_D_B: return "LD D, B";
        case OP_LD_D_C: return "LD D, C";
        case OP_LD_D_D: return "LD D, D";
        case OP_LD_D_E: return "LD D, E";
        case OP_LD_D_H: return "LD D, H";
        case OP_LD_D_L: return "LD D, L";
        case OP_LD_D_MEM_HL: return "LD D, (HL)";
        case OP_LD_D_A: return "LD D, A";
        case OP_LD_E_B: return "LD E, B";
        case OP_LD_E_C: return "LD E, C";
        case OP_LD_E_D: return "LD E, D";
        case OP_LD_E_E: return "LD E, E";
        case OP_LD_E_H: return "LD E, H";
        case OP_LD_E_L: return "LD E, L";
        case OP_LD_E_MEM_HL: return "LD E, (HL)";
        case OP_LD_E_A: return "LD E, A";
        case OP_LD_H_B: return "LD H, B";
        case OP_LD_H_C: return "LD H, C";
        case OP_LD_H_D: return "LD H, D";
        case OP_LD_H_E: return "LD H, E";
        case OP_LD_H_H: return "LD H, H";
        case OP_LD_H_L: return "LD H, L";
        case OP_LD_H_MEM_HL: return "LD H, (HL)";
        case OP_LD_H_A: return "LD H, A";
        case OP_LD_L_B: return "LD L, B";
        case OP_LD_L_C: return "LD L, C";
        case OP_LD_L_D: return "LD L, D";
        case OP_LD_L_E: return "LD L, E";
        case OP_LD_L_H: return "LD L, H";
        case OP_LD_L_L: return "LD L, L";
        case OP_LD_L_MEM_HL: return "LD L, (HL)";
        case OP_LD_L_A: return "LD L, A";
        case OP_LD_MEM_HL_B: return "LD (HL), B";
        case OP_LD_MEM_HL_C: return "LD (HL), C";
        case OP_LD_MEM_HL_D: return "LD (HL), D";
        case OP_LD_MEM_HL_E: return "LD (HL), E";
        case OP_LD_MEM_HL_H: return "LD (HL), H";
        case OP_LD_MEM_HL_L: return "LD (HL), L";
        case OP_HALT: return "HALT ";
        case OP_LD_MEM_HL_A: return "LD (HL), A";
        case OP_LD_A_B: return "LD A, B";
        case OP_LD_A_C: return "LD A, C";
        case OP_LD_A_D: return "LD A, D";
        case OP_LD_A_E: return "LD A, E";
        case OP_LD_A_H: return "LD A, H";
        case OP_LD_A_L: return "LD A, L";
        case OP_LD_A_MEM_HL: return "LD A, (HL)";
        case OP_LD_A_A: return "LD A, A";
        case OP_ADD_A_B: return "ADD A, B";
        case OP_ADD_A_C: return "ADD A, C";
        case OP_ADD_A_D: return "ADD A, D";
        case OP_ADD_A_E: return "ADD A, E";
        case OP_ADD_A_H: return "ADD A, H";
        case OP_ADD_A_L: return "ADD A, L";
        case OP_ADD_A_MEM_HL: return "ADD A, (HL)";
        case OP_ADD_A_A: return "ADD A, A";
        case OP_ADC_A_B: return "ADC A, B";
        case OP_ADC_A_C: return "ADC A, C";
        case OP_ADC_A_D: return "ADC A, D";
        case OP_ADC_A_E: return "ADC A, E";
        case OP_ADC_A_H: return "ADC A, H";
        case OP_ADC_A_L: return "ADC A, L";
        case OP_ADC_A_MEM_HL: return "ADC A, (HL)";
        case OP_ADC_A_A: return "ADC A, A";
        case OP_SUB_B: return "SUB B";
        case OP_SUB_C: return "SUB C";
        case OP_SUB_D: return "SUB D";
        case OP_SUB_E: return "SUB E";
        case OP_SUB_H: return "SUB H";
        case OP_SUB_L: return "SUB L";
        case OP_SUB_MEM_HL: return "SUB (HL)";
        case OP_SUB_A: return "SUB A";
        case OP_SBC_A_B: return "SBC A, B";
        case OP_SBC_A_C: return "SBC A, C";
        case OP_SBC_A_D: return "SBC A, D";
        case OP_SBC_A_E: return "SBC A, E";
        case OP_SBC_A_H: return "SBC A, H";
        case OP_SBC_A_L: return "SBC A, L";
        case OP_SBC_A_MEM_HL: return "SBC A, (HL)";
        case OP_SBC_A_A: return "SBC A, A";
        case OP_AND_B: return "AND B";
        case OP_AND_C: return "AND C";
        case OP_AND_D: return "AND D";
        case OP_AND_E: return "AND E";
        case OP_AND_H: return "AND H";
        case OP_AND_L: return "AND L";
        case OP_AND_MEM_HL: return "AND (HL)";
        case OP_AND_A: return "AND A";
        case OP_XOR_B: return "XOR B";
        case OP_XOR_C: return "XOR C";
        case OP_XOR_D: return "XOR D";
        case OP_XOR_E: return "XOR E";
        case OP_XOR_H: return "XOR H";
        case OP_XOR_L: return "XOR L";
        case OP_XOR_MEM_HL: return "XOR (HL)";
        case OP_XOR_A: return "XOR A";
        case OP_OR_B: return "OR B";
        case OP_OR_C: return "OR C";
        case OP_OR_D: return "OR D";
        case OP_OR_E: return "OR E";
        case OP_OR_H: return "OR H";
        case OP_OR_L: return "OR L";
        case OP_OR_MEM_HL: return "OR (HL)";
        case OP_OR_A: return "OR A";
        case OP_CP_B: return "CP B";
        case OP_CP_C: return "CP C";
        case OP_CP_D: return "CP D";
        case OP_CP_E: return "CP E";
        case OP_CP_H: return "CP H";
        case OP_CP_L: return "CP L";
        case OP_CP_MEM_HL: return "CP (HL)";
        case OP_CP_A: return "CP A";
        case OP_RET_NZ: return "RET NZ";
        case OP_POP_BC: return "POP BC";
        case OP_JP_NZ_a16: return fmt::format("JP NZ, ${:04X}", read16());
        case OP_JP_a16: return fmt::format("JP ${:04X}", read16());
        case OP_CALL_NZ_a16: return fmt::format("CALL NZ, ${:04X}", read16());
        case OP_PUSH_BC: return "PUSH BC";
        case OP_ADD_A_d8: return fmt::format("ADD A, {:02X}", read8());
        case OP_RST_00H: return "RST 00H";
        case OP_RET_Z: return "RET Z";
        case OP_RET: return "RET ";
        case OP_JP_Z_a16: return fmt::format("JP Z, ${:04X}", read16());
        case OP_PREFIX: return "PREFIX ";
        case OP_CALL_Z_a16: return fmt::format("CALL Z, ${:04X}", read16());
        case OP_CALL_a16: return fmt::format("CALL ${:04X}", read16());
        case OP_ADC_A_d8: return fmt::format("ADC A, {:02X}", read8());
        case OP_RST_08H: return "RST 08H";
        case OP_RET_NC: return "RET NC";
        case OP_POP_DE: return "POP DE";
        case OP_JP_NC_a16: return fmt::format("JP NC, ${:04X}", read16());
        case OP_CALL_NC_a16: return fmt::format("CALL NC, ${:04X}", read16());
        case OP_PUSH_DE: return "PUSH DE";
        case OP_SUB_d8: return fmt::format("SUB {:02X}", read8());
        case OP_RST_10H: return "RST 10H";
        case OP_RET_C: return "RET C";
        case OP_RETI: return "RETI ";
        case OP_JP_C_a16: return fmt::format("JP C, ${:04X}", read16());
        case OP_CALL_C_a16: return fmt::format("CALL C, ${:04X}", read16());
        case OP_SBC_A_d8: return fmt::format("SBC A, {:02X}", read8());
        case OP_RST_18H: return "RST 18H";
        case OP_LDH_MEM_a8_A: return fmt::format("LDH ({:02X}), A", read8());
        case OP_POP_HL: return "POP HL";
        case OP_LD_MEM_C_A: return "LD (C), A";
        case OP_PUSH_HL: return "PUSH HL";
        case OP_AND_d8: return fmt::format("AND {:02X}", read8());
        case OP_RST_20H: return "RST 20H";
        case OP_ADD_SP_r8: return fmt::format("ADD SP, {:02X}", read8());
        case OP_JP_HL: return "JP HL";
        case OP_LD_MEM_a16_A: return fmt::format("LD (${:04X}), A", read16());
        case OP_XOR_d8: return fmt::format("XOR {:02X}", read8());
        case OP_RST_28H: return "RST 28H";
        case OP_LDH_A_MEM_a8: return fmt::format("LDH A, ({:02X})", read8());
        case OP_POP_AF: return "POP AF";
        case OP_LD_A_MEM_C: return "LD A, (C)";
        case OP_DI: return "DI ";
        case OP_PUSH_AF: return "PUSH AF";
        case OP_OR_d8: return fmt::format("OR {:02X}", read8());
        case OP_RST_30H: return "RST 30H";
        case OP_LD_HL_SPI_r8: return fmt::format("LD HL, SP+{:02X}", read8());
        case OP_LD_SP_HL: return "LD SP, HL";
        case OP_LD_A_MEM_a16: return fmt::format("LD A, (${:04X})", read16());
        case OP_EI: return "EI ";
        case OP_CP_d8: return fmt::format("CP {:02X}", read8());
        case OP_RST_38H: return "RST 38H";

        default:
            UNIMPLEMENTED("Unimplemented or invalid opcode (0x{:02X})", op);
    }

}

u8 Disas::read8()
{
    if (m_idx >= m_size)
        UNREACHABLE("Invalid size");

    return m_mem[m_idx++];
}

u16 Disas::read16()
{
    u8 b0 = read8();
    u8 b1 = read8();
    return b0 << 8 | b1;
}

}
