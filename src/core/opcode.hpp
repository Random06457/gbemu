#pragma once

#include "types.hpp"

#define MAKE_OP(i, name) OP_##name = i,
enum
{

MAKE_OP(0x00, NOP)                      // NOP 
MAKE_OP(0x01, LD_BC_d16)                // LD BC, d16
MAKE_OP(0x02, LD_MEM_BC_A)              // LD (BC), A
MAKE_OP(0x03, INC_BC)                   // INC BC
MAKE_OP(0x04, INC_B)                    // INC B
MAKE_OP(0x05, DEC_B)                    // DEC B
MAKE_OP(0x06, LD_B_d8)                  // LD B, d8
MAKE_OP(0x07, RLCA)                     // RLCA 
MAKE_OP(0x08, LD_MEM_a16_SP)            // LD (a16), SP
MAKE_OP(0x09, ADD_HL_BC)                // ADD HL, BC
MAKE_OP(0x0A, LD_A_MEM_BC)              // LD A, (BC)
MAKE_OP(0x0B, DEC_BC)                   // DEC BC
MAKE_OP(0x0C, INC_C)                    // INC C
MAKE_OP(0x0D, DEC_C)                    // DEC C
MAKE_OP(0x0E, LD_C_d8)                  // LD C, d8
MAKE_OP(0x0F, RRCA)                     // RRCA 
MAKE_OP(0x10, STOP_d8)                  // STOP d8
MAKE_OP(0x11, LD_DE_d16)                // LD DE, d16
MAKE_OP(0x12, LD_MEM_DE_A)              // LD (DE), A
MAKE_OP(0x13, INC_DE)                   // INC DE
MAKE_OP(0x14, INC_D)                    // INC D
MAKE_OP(0x15, DEC_D)                    // DEC D
MAKE_OP(0x16, LD_D_d8)                  // LD D, d8
MAKE_OP(0x17, RLA)                      // RLA 
MAKE_OP(0x18, JR_r8)                    // JR r8
MAKE_OP(0x19, ADD_HL_DE)                // ADD HL, DE
MAKE_OP(0x1A, LD_A_MEM_DE)              // LD A, (DE)
MAKE_OP(0x1B, DEC_DE)                   // DEC DE
MAKE_OP(0x1C, INC_E)                    // INC E
MAKE_OP(0x1D, DEC_E)                    // DEC E
MAKE_OP(0x1E, LD_E_d8)                  // LD E, d8
MAKE_OP(0x1F, RRA)                      // RRA 
MAKE_OP(0x20, JR_NZ_r8)                 // JR NZ, r8
MAKE_OP(0x21, LD_HL_d16)                // LD HL, d16
MAKE_OP(0x22, LD_MEM_HLI_A)             // LD (HL+), A
MAKE_OP(0x23, INC_HL)                   // INC HL
MAKE_OP(0x24, INC_H)                    // INC H
MAKE_OP(0x25, DEC_H)                    // DEC H
MAKE_OP(0x26, LD_H_d8)                  // LD H, d8
MAKE_OP(0x27, DAA)                      // DAA 
MAKE_OP(0x28, JR_Z_r8)                  // JR Z, r8
MAKE_OP(0x29, ADD_HL_HL)                // ADD HL, HL
MAKE_OP(0x2A, LD_A_MEM_HLI)             // LD A, (HL+)
MAKE_OP(0x2B, DEC_HL)                   // DEC HL
MAKE_OP(0x2C, INC_L)                    // INC L
MAKE_OP(0x2D, DEC_L)                    // DEC L
MAKE_OP(0x2E, LD_L_d8)                  // LD L, d8
MAKE_OP(0x2F, CPL)                      // CPL 
MAKE_OP(0x30, JR_NC_r8)                 // JR NC, r8
MAKE_OP(0x31, LD_SP_d16)                // LD SP, d16
MAKE_OP(0x32, LD_MEM_HLD_A)             // LD (HL-), A
MAKE_OP(0x33, INC_SP)                   // INC SP
MAKE_OP(0x34, INC_MEM_HL)               // INC (HL)
MAKE_OP(0x35, DEC_MEM_HL)               // DEC (HL)
MAKE_OP(0x36, LD_MEM_HL_d8)             // LD (HL), d8
MAKE_OP(0x37, SCF)                      // SCF 
MAKE_OP(0x38, JR_C_r8)                  // JR C, r8
MAKE_OP(0x39, ADD_HL_SP)                // ADD HL, SP
MAKE_OP(0x3A, LD_A_MEM_HLD)             // LD A, (HL-)
MAKE_OP(0x3B, DEC_SP)                   // DEC SP
MAKE_OP(0x3C, INC_A)                    // INC A
MAKE_OP(0x3D, DEC_A)                    // DEC A
MAKE_OP(0x3E, LD_A_d8)                  // LD A, d8
MAKE_OP(0x3F, CCF)                      // CCF 
MAKE_OP(0x40, LD_B_B)                   // LD B, B
MAKE_OP(0x41, LD_B_C)                   // LD B, C
MAKE_OP(0x42, LD_B_D)                   // LD B, D
MAKE_OP(0x43, LD_B_E)                   // LD B, E
MAKE_OP(0x44, LD_B_H)                   // LD B, H
MAKE_OP(0x45, LD_B_L)                   // LD B, L
MAKE_OP(0x46, LD_B_MEM_HL)              // LD B, (HL)
MAKE_OP(0x47, LD_B_A)                   // LD B, A
MAKE_OP(0x48, LD_C_B)                   // LD C, B
MAKE_OP(0x49, LD_C_C)                   // LD C, C
MAKE_OP(0x4A, LD_C_D)                   // LD C, D
MAKE_OP(0x4B, LD_C_E)                   // LD C, E
MAKE_OP(0x4C, LD_C_H)                   // LD C, H
MAKE_OP(0x4D, LD_C_L)                   // LD C, L
MAKE_OP(0x4E, LD_C_MEM_HL)              // LD C, (HL)
MAKE_OP(0x4F, LD_C_A)                   // LD C, A
MAKE_OP(0x50, LD_D_B)                   // LD D, B
MAKE_OP(0x51, LD_D_C)                   // LD D, C
MAKE_OP(0x52, LD_D_D)                   // LD D, D
MAKE_OP(0x53, LD_D_E)                   // LD D, E
MAKE_OP(0x54, LD_D_H)                   // LD D, H
MAKE_OP(0x55, LD_D_L)                   // LD D, L
MAKE_OP(0x56, LD_D_MEM_HL)              // LD D, (HL)
MAKE_OP(0x57, LD_D_A)                   // LD D, A
MAKE_OP(0x58, LD_E_B)                   // LD E, B
MAKE_OP(0x59, LD_E_C)                   // LD E, C
MAKE_OP(0x5A, LD_E_D)                   // LD E, D
MAKE_OP(0x5B, LD_E_E)                   // LD E, E
MAKE_OP(0x5C, LD_E_H)                   // LD E, H
MAKE_OP(0x5D, LD_E_L)                   // LD E, L
MAKE_OP(0x5E, LD_E_MEM_HL)              // LD E, (HL)
MAKE_OP(0x5F, LD_E_A)                   // LD E, A
MAKE_OP(0x60, LD_H_B)                   // LD H, B
MAKE_OP(0x61, LD_H_C)                   // LD H, C
MAKE_OP(0x62, LD_H_D)                   // LD H, D
MAKE_OP(0x63, LD_H_E)                   // LD H, E
MAKE_OP(0x64, LD_H_H)                   // LD H, H
MAKE_OP(0x65, LD_H_L)                   // LD H, L
MAKE_OP(0x66, LD_H_MEM_HL)              // LD H, (HL)
MAKE_OP(0x67, LD_H_A)                   // LD H, A
MAKE_OP(0x68, LD_L_B)                   // LD L, B
MAKE_OP(0x69, LD_L_C)                   // LD L, C
MAKE_OP(0x6A, LD_L_D)                   // LD L, D
MAKE_OP(0x6B, LD_L_E)                   // LD L, E
MAKE_OP(0x6C, LD_L_H)                   // LD L, H
MAKE_OP(0x6D, LD_L_L)                   // LD L, L
MAKE_OP(0x6E, LD_L_MEM_HL)              // LD L, (HL)
MAKE_OP(0x6F, LD_L_A)                   // LD L, A
MAKE_OP(0x70, LD_MEM_HL_B)              // LD (HL), B
MAKE_OP(0x71, LD_MEM_HL_C)              // LD (HL), C
MAKE_OP(0x72, LD_MEM_HL_D)              // LD (HL), D
MAKE_OP(0x73, LD_MEM_HL_E)              // LD (HL), E
MAKE_OP(0x74, LD_MEM_HL_H)              // LD (HL), H
MAKE_OP(0x75, LD_MEM_HL_L)              // LD (HL), L
MAKE_OP(0x76, HALT)                     // HALT 
MAKE_OP(0x77, LD_MEM_HL_A)              // LD (HL), A
MAKE_OP(0x78, LD_A_B)                   // LD A, B
MAKE_OP(0x79, LD_A_C)                   // LD A, C
MAKE_OP(0x7A, LD_A_D)                   // LD A, D
MAKE_OP(0x7B, LD_A_E)                   // LD A, E
MAKE_OP(0x7C, LD_A_H)                   // LD A, H
MAKE_OP(0x7D, LD_A_L)                   // LD A, L
MAKE_OP(0x7E, LD_A_MEM_HL)              // LD A, (HL)
MAKE_OP(0x7F, LD_A_A)                   // LD A, A
MAKE_OP(0x80, ADD_A_B)                  // ADD A, B
MAKE_OP(0x81, ADD_A_C)                  // ADD A, C
MAKE_OP(0x82, ADD_A_D)                  // ADD A, D
MAKE_OP(0x83, ADD_A_E)                  // ADD A, E
MAKE_OP(0x84, ADD_A_H)                  // ADD A, H
MAKE_OP(0x85, ADD_A_L)                  // ADD A, L
MAKE_OP(0x86, ADD_A_MEM_HL)             // ADD A, (HL)
MAKE_OP(0x87, ADD_A_A)                  // ADD A, A
MAKE_OP(0x88, ADC_A_B)                  // ADC A, B
MAKE_OP(0x89, ADC_A_C)                  // ADC A, C
MAKE_OP(0x8A, ADC_A_D)                  // ADC A, D
MAKE_OP(0x8B, ADC_A_E)                  // ADC A, E
MAKE_OP(0x8C, ADC_A_H)                  // ADC A, H
MAKE_OP(0x8D, ADC_A_L)                  // ADC A, L
MAKE_OP(0x8E, ADC_A_MEM_HL)             // ADC A, (HL)
MAKE_OP(0x8F, ADC_A_A)                  // ADC A, A
MAKE_OP(0x90, SUB_B)                    // SUB B
MAKE_OP(0x91, SUB_C)                    // SUB C
MAKE_OP(0x92, SUB_D)                    // SUB D
MAKE_OP(0x93, SUB_E)                    // SUB E
MAKE_OP(0x94, SUB_H)                    // SUB H
MAKE_OP(0x95, SUB_L)                    // SUB L
MAKE_OP(0x96, SUB_MEM_HL)               // SUB (HL)
MAKE_OP(0x97, SUB_A)                    // SUB A
MAKE_OP(0x98, SBC_A_B)                  // SBC A, B
MAKE_OP(0x99, SBC_A_C)                  // SBC A, C
MAKE_OP(0x9A, SBC_A_D)                  // SBC A, D
MAKE_OP(0x9B, SBC_A_E)                  // SBC A, E
MAKE_OP(0x9C, SBC_A_H)                  // SBC A, H
MAKE_OP(0x9D, SBC_A_L)                  // SBC A, L
MAKE_OP(0x9E, SBC_A_MEM_HL)             // SBC A, (HL)
MAKE_OP(0x9F, SBC_A_A)                  // SBC A, A
MAKE_OP(0xA0, AND_B)                    // AND B
MAKE_OP(0xA1, AND_C)                    // AND C
MAKE_OP(0xA2, AND_D)                    // AND D
MAKE_OP(0xA3, AND_E)                    // AND E
MAKE_OP(0xA4, AND_H)                    // AND H
MAKE_OP(0xA5, AND_L)                    // AND L
MAKE_OP(0xA6, AND_MEM_HL)               // AND (HL)
MAKE_OP(0xA7, AND_A)                    // AND A
MAKE_OP(0xA8, XOR_B)                    // XOR B
MAKE_OP(0xA9, XOR_C)                    // XOR C
MAKE_OP(0xAA, XOR_D)                    // XOR D
MAKE_OP(0xAB, XOR_E)                    // XOR E
MAKE_OP(0xAC, XOR_H)                    // XOR H
MAKE_OP(0xAD, XOR_L)                    // XOR L
MAKE_OP(0xAE, XOR_MEM_HL)               // XOR (HL)
MAKE_OP(0xAF, XOR_A)                    // XOR A
MAKE_OP(0xB0, OR_B)                     // OR B
MAKE_OP(0xB1, OR_C)                     // OR C
MAKE_OP(0xB2, OR_D)                     // OR D
MAKE_OP(0xB3, OR_E)                     // OR E
MAKE_OP(0xB4, OR_H)                     // OR H
MAKE_OP(0xB5, OR_L)                     // OR L
MAKE_OP(0xB6, OR_MEM_HL)                // OR (HL)
MAKE_OP(0xB7, OR_A)                     // OR A
MAKE_OP(0xB8, CP_B)                     // CP B
MAKE_OP(0xB9, CP_C)                     // CP C
MAKE_OP(0xBA, CP_D)                     // CP D
MAKE_OP(0xBB, CP_E)                     // CP E
MAKE_OP(0xBC, CP_H)                     // CP H
MAKE_OP(0xBD, CP_L)                     // CP L
MAKE_OP(0xBE, CP_MEM_HL)                // CP (HL)
MAKE_OP(0xBF, CP_A)                     // CP A
MAKE_OP(0xC0, RET_NZ)                   // RET NZ
MAKE_OP(0xC1, POP_BC)                   // POP BC
MAKE_OP(0xC2, JP_NZ_a16)                // JP NZ, a16
MAKE_OP(0xC3, JP_a16)                   // JP a16
MAKE_OP(0xC4, CALL_NZ_a16)              // CALL NZ, a16
MAKE_OP(0xC5, PUSH_BC)                  // PUSH BC
MAKE_OP(0xC6, ADD_A_d8)                 // ADD A, d8
MAKE_OP(0xC7, RST_00H)                  // RST 00H
MAKE_OP(0xC8, RET_Z)                    // RET Z
MAKE_OP(0xC9, RET)                      // RET 
MAKE_OP(0xCA, JP_Z_a16)                 // JP Z, a16
MAKE_OP(0xCB, PREFIX)                   // PREFIX 
MAKE_OP(0xCC, CALL_Z_a16)               // CALL Z, a16
MAKE_OP(0xCD, CALL_a16)                 // CALL a16
MAKE_OP(0xCE, ADC_A_d8)                 // ADC A, d8
MAKE_OP(0xCF, RST_08H)                  // RST 08H
MAKE_OP(0xD0, RET_NC)                   // RET NC
MAKE_OP(0xD1, POP_DE)                   // POP DE
MAKE_OP(0xD2, JP_NC_a16)                // JP NC, a16
MAKE_OP(0xD4, CALL_NC_a16)              // CALL NC, a16
MAKE_OP(0xD5, PUSH_DE)                  // PUSH DE
MAKE_OP(0xD6, SUB_d8)                   // SUB d8
MAKE_OP(0xD7, RST_10H)                  // RST 10H
MAKE_OP(0xD8, RET_C)                    // RET C
MAKE_OP(0xD9, RETI)                     // RETI 
MAKE_OP(0xDA, JP_C_a16)                 // JP C, a16
MAKE_OP(0xDC, CALL_C_a16)               // CALL C, a16
MAKE_OP(0xDE, SBC_A_d8)                 // SBC A, d8
MAKE_OP(0xDF, RST_18H)                  // RST 18H
MAKE_OP(0xE0, LDH_MEM_a8_A)             // LDH (a8), A
MAKE_OP(0xE1, POP_HL)                   // POP HL
MAKE_OP(0xE2, LD_MEM_C_A)               // LD (C), A
MAKE_OP(0xE5, PUSH_HL)                  // PUSH HL
MAKE_OP(0xE6, AND_d8)                   // AND d8
MAKE_OP(0xE7, RST_20H)                  // RST 20H
MAKE_OP(0xE8, ADD_SP_r8)                // ADD SP, r8
MAKE_OP(0xE9, JP_HL)                    // JP HL
MAKE_OP(0xEA, LD_MEM_a16_A)             // LD (a16), A
MAKE_OP(0xEE, XOR_d8)                   // XOR d8
MAKE_OP(0xEF, RST_28H)                  // RST 28H
MAKE_OP(0xF0, LDH_A_MEM_a8)             // LDH A, (a8)
MAKE_OP(0xF1, POP_AF)                   // POP AF
MAKE_OP(0xF2, LD_A_MEM_C)               // LD A, (C)
MAKE_OP(0xF3, DI)                       // DI 
MAKE_OP(0xF5, PUSH_AF)                  // PUSH AF
MAKE_OP(0xF6, OR_d8)                    // OR d8
MAKE_OP(0xF7, RST_30H)                  // RST 30H
MAKE_OP(0xF8, LD_HL_SPI_r8)             // LD HL, SP+r8
MAKE_OP(0xF9, LD_SP_HL)                 // LD SP, HL
MAKE_OP(0xFA, LD_A_MEM_a16)             // LD A, (a16)
MAKE_OP(0xFB, EI)                       // EI 
MAKE_OP(0xFE, CP_d8)                    // CP d8
MAKE_OP(0xFF, RST_38H)                  // RST 38H

};

#undef MAKE_OP
