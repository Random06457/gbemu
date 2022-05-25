#include <gtest/gtest.h>
#include "core/cpu.hpp"
#include "core/memory.hpp"
#include "core/timer.hpp"
#include "core/int_controller.hpp"
#include "core/opcode.hpp"

using namespace gbemu::core;

#define REG_A cpu.regs().a
#define REG_B cpu.regs().b
#define REG_C cpu.regs().c
#define REG_D cpu.regs().d
#define REG_E cpu.regs().e
#define REG_H cpu.regs().h
#define REG_L cpu.regs().l
#define REG_BC cpu.regs().bc
#define REG_DE cpu.regs().de
#define REG_HL cpu.regs().hl
#define REG_SP cpu.regs().sp
#define REG_PC cpu.regs().pc
#define Z cpu.regs().flags.z
#define N cpu.regs().flags.n
#define H cpu.regs().flags.h
#define C cpu.regs().flags.c

#define CPU_LOG     false

#define CHECK_FLAGS(z_, n_, h_, c_) \
    ASSERT_EQ(Z, z_); \
    ASSERT_EQ(N, n_); \
    ASSERT_EQ(H, h_); \
    ASSERT_EQ(C, c_);

#define CPU_CREATE(...) \
    u8 code[] = { __VA_ARGS__ }; \
    u8 ram[0x100]; \
    Memory mem; \
    InterruptController ints; \
    Timer timer(&ints); \
    mem.mapBuffer(0x0000, code, sizeof(code)); \
    mem.mapBuffer(0x1000, ram, sizeof(ram)); \
    Cpu cpu(&mem, &timer, &ints); \
    cpu.setLogging(CPU_LOG);

#define CPU_RUN() \
    while (REG_PC != sizeof(code)) \
        cpu.step();


TEST(cpu, read_vregs)
{
    CPU_CREATE(0xFF, 0x03, 0x10, 0x04);

    u8 io[] = { 0x00, 0x00, 0xBB, 0x00, 0xAA };
    mem.mapBuffer(0xFF00, io, sizeof(io));

    REG_A = 0x11;
    REG_B = 0x22;
    REG_C = 0x33;
    REG_D = 0x44;
    REG_E = 0x55;

    ASSERT_EQ(cpu.readReg(Cpu::VREG8_A), 0x11);
    ASSERT_EQ(cpu.readReg(Cpu::VREG8_B), 0x22);
    ASSERT_EQ(cpu.readReg(Cpu::VREG8_C), 0x33);
    ASSERT_EQ(cpu.readReg(Cpu::VREG8_D), 0x44);
    ASSERT_EQ(cpu.readReg(Cpu::VREG8_E), 0x55);

    ram[0] = 0x66;
    ram[1] = 0x77;
    ram[2] = 0x88;
    ram[3] = 0x99;

    REG_BC = 0x1000;
    REG_DE = 0x1001;
    REG_HL = 0x1002;

    ASSERT_EQ(cpu.readReg(Cpu::VREG8_BC8), 0x66);
    ASSERT_EQ(cpu.readReg(Cpu::VREG8_DE8), 0x77);
    ASSERT_EQ(cpu.readReg(Cpu::VREG8_HL8), 0x88);

    REG_C = 2;
    ASSERT_EQ(cpu.readReg(Cpu::VREG8_D8), 0xFF);
    ASSERT_EQ(cpu.readReg(Cpu::VREG8_A16), 0x99);
    ASSERT_EQ(cpu.readReg(Cpu::VREG8_HA8), 0xAA);

    REG_HL = 0x1000;
    ASSERT_EQ(cpu.readReg(Cpu::VREG8_HLI), 0x66);
    ASSERT_EQ(REG_HL, 0x1001);
    ASSERT_EQ(cpu.readReg(Cpu::VREG8_HLD), 0x77);
    ASSERT_EQ(REG_HL, 0x1000);
}

// 0x_1
TEST(cpu, ld_r16_d16)
{
    CPU_CREATE(
        OP_LD_BC_d16, 0x11, 0x22,
        OP_LD_DE_d16, 0x33, 0x44,
        OP_LD_HL_d16, 0x55, 0x66,
        OP_LD_SP_d16, 0x77, 0x88);

    CPU_RUN()

    ASSERT_EQ(REG_BC, 0x2211);
    ASSERT_EQ(REG_DE, 0x4433);
    ASSERT_EQ(REG_HL, 0x6655);
    ASSERT_EQ(REG_SP, 0x8877);
}

// 0x_2
TEST(cpu, ld_mem_r8)
{
    CPU_CREATE(
        OP_LD_MEM_BC_A,
        OP_LD_MEM_DE_A,
        OP_LD_MEM_HLI_A,
        OP_LD_MEM_HLD_A);

    ram[0] = 0x11;
    ram[1] = 0x22;
    ram[2] = 0x33;
    ram[3] = 0x44;

    REG_A = 0xFF;
    REG_BC = 0x1000;
    REG_DE = 0x1001;
    REG_HL = 0x1002;

    CPU_RUN()

    ASSERT_EQ(ram[0], 0xFF);
    ASSERT_EQ(ram[1], 0xFF);
    ASSERT_EQ(ram[2], 0xFF);
    ASSERT_EQ(ram[3], 0xFF);
    ASSERT_EQ(REG_HL, 0x1002);
}

// 0x_3
TEST(cpu, inc_r16)
{
    CPU_CREATE(
        OP_INC_BC,
        OP_INC_DE,
        OP_INC_HL,
        OP_INC_SP);

    REG_BC = 0x1111;
    REG_DE = 0x2222;
    REG_HL = 0x3333;
    REG_SP = 0x4444;

    CPU_RUN();

    ASSERT_EQ(REG_BC, 0x1112);
    ASSERT_EQ(REG_DE, 0x2223);
    ASSERT_EQ(REG_HL, 0x3334);
    ASSERT_EQ(REG_SP, 0x4445);
}

// 0x_4 / 0x_C
TEST(cpu, inc_r8)
{
    CPU_CREATE(
        OP_INC_B,
        OP_INC_D,
        OP_INC_H,
        OP_INC_MEM_HL,
        OP_INC_C,
        OP_INC_E,
        OP_INC_L,
        OP_INC_A);

    ram[0] = 0xFF;

    REG_B = 0x11;
    REG_D = 0x1F;
    REG_HL = 0x0F00;
    REG_C = 0x11;
    REG_E = 0x1F;
    REG_A = 0x08;

    cpu.step();
    ASSERT_EQ(REG_B, 0x12);
    CHECK_FLAGS(0, 0, 0, 0);

    cpu.step();
    ASSERT_EQ(REG_D, 0x20);
    CHECK_FLAGS(0, 0, 1, 0);

    cpu.step();
    ASSERT_EQ(REG_H, 0x10);
    CHECK_FLAGS(0, 0, 1, 0);

    cpu.step();
    ASSERT_EQ(ram[0], 0x00);
    CHECK_FLAGS(1, 0, 1, 0);

    cpu.step();
    ASSERT_EQ(REG_C, 0x12);
    CHECK_FLAGS(0, 0, 0, 0);

    cpu.step();
    ASSERT_EQ(REG_E, 0x20);
    CHECK_FLAGS(0, 0, 1, 0);

    cpu.step();
    ASSERT_EQ(REG_L, 0x01);
    CHECK_FLAGS(0, 0, 0, 0);

    cpu.step();
    ASSERT_EQ(REG_A, 0x9);
    CHECK_FLAGS(0, 0, 0, 0);
}

// 0x_5 / 0x_D
TEST(cpu, dec_r8)
{
    CPU_CREATE(
        OP_DEC_B,
        OP_DEC_D,
        OP_DEC_H,
        OP_DEC_MEM_HL,
        OP_DEC_C,
        OP_DEC_E,
        OP_DEC_L,
        OP_DEC_A);

    ram[0] = 0x01;

    REG_B = 0x10;
    REG_D = 0x00;
    REG_HL = 0x1100;
    REG_C = 0x10;
    REG_E = 0x00;
    REG_A = 0x01;

    cpu.step();
    ASSERT_EQ(REG_B, 0x0F);
    CHECK_FLAGS(0, 1, 1, 0);

    cpu.step();
    ASSERT_EQ(REG_D, 0xFF);
    CHECK_FLAGS(0, 1, 1, 0);

    cpu.step();
    ASSERT_EQ(REG_H, 0x10);
    CHECK_FLAGS(0, 1, 0, 0);

    cpu.step();
    ASSERT_EQ(ram[0], 0x00);
    CHECK_FLAGS(1, 1, 0, 0);


    cpu.step();
    ASSERT_EQ(REG_C, 0x0F);
    CHECK_FLAGS(0, 1, 1, 0);

    cpu.step();
    ASSERT_EQ(REG_E, 0xFF);
    CHECK_FLAGS(0, 1, 1, 0);

    cpu.step();
    ASSERT_EQ(REG_L, 0xFF);
    CHECK_FLAGS(0, 1, 1, 0);

    cpu.step();
    ASSERT_EQ(REG_A, 0x00);
    CHECK_FLAGS(1, 1, 0, 0);
}

// 0x_6 / 0x_E
TEST(cpu, ld_r8_d8)
{
    CPU_CREATE(
        OP_LD_B_d8, 0x11,
        OP_LD_D_d8, 0x22,
        OP_LD_H_d8, 0x10,
        OP_LD_MEM_HL_d8, 0x44,
        OP_LD_C_d8, 0x11,
        OP_LD_E_d8, 0x22,
        OP_LD_L_d8, 0x10,
        OP_LD_A_d8, 0x44);

    REG_HL = 0x0000;

    CPU_RUN();

    ASSERT_EQ(REG_B, 0x11);
    ASSERT_EQ(REG_D, 0x22);
    ASSERT_EQ(REG_H, 0x10);
    ASSERT_EQ(ram[0], 0x44);
    ASSERT_EQ(REG_C, 0x11);
    ASSERT_EQ(REG_E, 0x22);
    ASSERT_EQ(REG_L, 0x10);
    ASSERT_EQ(REG_A, 0x44);
    ASSERT_EQ(REG_C, 0x11);
    ASSERT_EQ(REG_E, 0x22);
    ASSERT_EQ(REG_L, 0x10);
    ASSERT_EQ(REG_A, 0x44);
}

// 0x_7 / 0x_F
TEST(cpu, rotate_a)
{
    CPU_CREATE(
        OP_RLCA,
        OP_RLA,
        OP_RRCA,
        OP_RRA,
    );

    // RLCA
    C = 0;
    REG_A = 0b11101000;

    cpu.step();

    ASSERT_EQ(REG_A, 0b11010001);
    ASSERT_EQ(C, 1);

    // RLA
    C = 1;
    REG_A = 0b01101000;

    cpu.step();

    ASSERT_EQ(REG_A, 0b11010001);
    ASSERT_EQ(C, 0);

    // RRCA
    C = 0;
    REG_A = 0b00010111;

    cpu.step();

    ASSERT_EQ(REG_A, 0b10001011);
    ASSERT_EQ(C, 1);

    // RRA
    C = 1;
    REG_A = 0b00010110;

    cpu.step();

    ASSERT_EQ(REG_A, 0b10001011);
    ASSERT_EQ(C, 0);
}

// 0x_08
TEST(cpu, ld_mem_a16_sp)
{
    CPU_CREATE(OP_LD_MEM_a16_SP, 0x00, 0x10);

    REG_SP = 0xAABB;
    CPU_RUN();

    ASSERT_EQ(ram[0], 0xBB);
    ASSERT_EQ(ram[1], 0xAA);
}

// 0x_09
TEST(cpu, add_r16_r16)
{
    CPU_CREATE(
        OP_ADD_HL_BC,
        OP_ADD_HL_DE,
        OP_ADD_HL_HL,
        OP_ADD_HL_SP,
        );

    Z = 0;
    REG_HL = 0x00FE;
    REG_BC = 0x0002;
    cpu.step();
    ASSERT_EQ(REG_HL, 0x100);
    CHECK_FLAGS(0, 0, 0, 0);

    REG_HL = 0xF000;
    REG_DE = 0x2000;
    cpu.step();
    ASSERT_EQ(REG_HL, 0x1000);
    CHECK_FLAGS(0, 0, 0, 1);

    REG_HL = 0x0811;
    cpu.step();
    ASSERT_EQ(REG_HL, 0x1022);
    CHECK_FLAGS(0, 0, 1, 0);

    REG_HL = 0x0111;
    REG_SP = 0x1234;
    cpu.step();
    ASSERT_EQ(REG_HL, 0x1345);
    CHECK_FLAGS(0, 0, 0, 0);

}

// 0x_A
TEST(cpu, ld_r8_mem)
{
    CPU_CREATE(
        OP_LD_A_MEM_BC,
        OP_LD_A_MEM_DE,
        OP_LD_A_MEM_HLI,
        OP_LD_A_MEM_HLD);

    ram[0] = 0x11;
    ram[1] = 0x22;
    ram[2] = 0x33;
    ram[3] = 0x44;

    REG_BC = 0x1000;
    REG_DE = 0x1001;
    REG_HL = 0x1002;

    cpu.step();
    ASSERT_EQ(REG_A, 0x11);
    cpu.step();
    ASSERT_EQ(REG_A, 0x22);
    cpu.step();
    ASSERT_EQ(REG_A, 0x33);
    cpu.step();
    ASSERT_EQ(REG_A, 0x44);

    ASSERT_EQ(REG_HL, 0x1002);
}

// 0x_B
TEST(cpu, dec_r16)
{
    CPU_CREATE(
        OP_DEC_BC,
        OP_DEC_DE,
        OP_DEC_HL,
        OP_DEC_SP);

    REG_BC = 0x1111;
    REG_DE = 0x2222;
    REG_HL = 0x3333;
    REG_SP = 0x4444;

    CPU_RUN();

    ASSERT_EQ(REG_BC, 0x1110);
    ASSERT_EQ(REG_DE, 0x2221);
    ASSERT_EQ(REG_HL, 0x3332);
    ASSERT_EQ(REG_SP, 0x4443);
}

// 0x_0 / 0x_8
TEST(cpu, jr_r8)
{
    CPU_CREATE(
        OP_JR_NZ_r8, 0x06,  // 0x0000
        OP_JR_NC_r8, 0x70,  // 0x0002
        OP_JR_Z_r8,  0x70,  // 0x0004
        OP_JR_r8,    0xFA,  // 0x0006
        OP_JR_C_r8,  0xFA,  // 0x0008
    );

    Z = 0;
    cpu.step();
    ASSERT_EQ(REG_PC, 0x0008);

    C = 1;
    cpu.step();
    ASSERT_EQ(REG_PC, 0x0004);

    Z = 0;
    cpu.step();
    ASSERT_EQ(REG_PC, 0x0006);

    cpu.step();
    ASSERT_EQ(REG_PC, 0x0002);

    C = 0;
    cpu.step();
    ASSERT_EQ(REG_PC, 0x0074);
}


// 0x4_ / 0x5_ / 0x6_ / 0x7_
TEST(cpu, ld_r8_r8)
{
#define TEST_LD(src) { \
    CPU_CREATE( \
        OP_LD_##src##_d8, 0x11, OP_LD_HL_d16, 0x00, 0x10, \
        OP_LD_B_##src, OP_LD_C_##src, OP_LD_D_##src, OP_LD_E_##src, OP_LD_MEM_HL_##src, OP_LD_A_##src, OP_LD_H_##src, OP_LD_L_##src \
    ); \
    CPU_RUN(); \
    for (size_t i = 0; i < 8; i++) \
    { \
        u8 b = i == 6 ? ram[0] : cpu.readReg((Cpu::VREG8)i); \
        ASSERT_EQ(b, 0x11); \
    } \
}

    TEST_LD(A);
    TEST_LD(B);
    TEST_LD(C);
    TEST_LD(D);
    TEST_LD(E);

    // HL
    {
        CPU_CREATE(
            OP_LD_H_MEM_HL,
            OP_LD_L_MEM_HL,

            OP_LD_HL_d16, 0x00, 0x10,
            OP_LD_B_MEM_HL, OP_LD_C_MEM_HL, OP_LD_D_MEM_HL, OP_LD_E_MEM_HL, OP_LD_A_MEM_HL,
        );

        ram[0] = 0x11;


        REG_HL = 0x1000;
        cpu.step();
        ASSERT_EQ(REG_H, 0x11);
        REG_HL = 0x1000;
        cpu.step();
        ASSERT_EQ(REG_L, 0x11);

        CPU_RUN();
        for (size_t i = 0; i < 8; i++)
        {
            Cpu::VREG8 reg = (Cpu::VREG8)i;
            if (i == Cpu::VREG8_H || i == Cpu::VREG8_L || i == Cpu::VREG8_HL8)
                continue;
            ASSERT_EQ(cpu.readReg(reg), 0x11);
        }
    }
    // L
    {
        CPU_CREATE(
            OP_LD_MEM_HL_L,
            OP_LD_B_L, OP_LD_C_L, OP_LD_D_L, OP_LD_E_L, OP_LD_H_L, OP_LD_L_L, OP_LD_A_L
        );

        ram[0] = 0x11;
        REG_HL = 0x1000;
        cpu.step();
        ASSERT_EQ(ram[0], 0x00);

        REG_L = 0x11;
        CPU_RUN();
        for (size_t i = 0; i < 8; i++)
        {
            Cpu::VREG8 reg = (Cpu::VREG8)i;
            if (i == Cpu::VREG8_HL8)
                continue;
            ASSERT_EQ(cpu.readReg(reg), 0x11);
        }
    }
    // H
    {
        CPU_CREATE(
            OP_LD_MEM_HL_H,
            OP_LD_B_H, OP_LD_C_H, OP_LD_D_H, OP_LD_E_H, OP_LD_H_H, OP_LD_L_H, OP_LD_A_H
        );

        ram[0] = 0x11;
        REG_HL = 0x1000;
        cpu.step();
        ASSERT_EQ(ram[0], 0x10);

        REG_H = 0x11;
        CPU_RUN();
        for (size_t i = 0; i < 8; i++)
        {
            Cpu::VREG8 reg = (Cpu::VREG8)i;
            if (i == Cpu::VREG8_HL8)
                continue;
            ASSERT_EQ(cpu.readReg(reg), 0x11);
        }
    }

}

#define TEST_OP_R8_R8(reg_y, x, y, res, res_z, res_n, res_h, res_c) \
    REG_PC = 0; \
    REG_A = x; \
    cpu.writeReg(reg_y, y); \
    cpu.step(); \
    ASSERT_EQ(REG_A, res); \
    ASSERT_EQ(Z, res_z); \
    ASSERT_EQ(N, res_n); \
    ASSERT_EQ(H, res_h); \
    ASSERT_EQ(C, res_c);

// 0x8_
TEST(cpu, add_r8_r8)
{
    CPU_CREATE(OP_ADD_A_B);

    for (size_t i = 0; i < 7; i++)
    {
        REG_HL = 0x1000;

        TEST_OP_R8_R8((Cpu::VREG8)i, 0xF, 1, 0x10, 0, 0, 1, 0);

        TEST_OP_R8_R8((Cpu::VREG8)i, 0xF2, 0xE, 0x00, 1, 0, 1, 1);

        code[0]++;
    }

    TEST_OP_R8_R8(Cpu::VREG8_A, 0x88, 0x88, 0x10, 0, 0, 1, 1);

    TEST_OP_R8_R8(Cpu::VREG8_A, 0x80, 0x80, 0x00, 1, 0, 0, 1);

    TEST_OP_R8_R8(Cpu::VREG8_A, 0x11, 0x11, 0x22, 0, 0, 0, 0);
}
// 0x8_
TEST(cpu, addc_r8_r8)
{
    CPU_CREATE(OP_ADC_A_B);

    for (size_t i = 0; i < 7; i++)
    {
        REG_HL = 0x1000;

        C = 0;
        TEST_OP_R8_R8((Cpu::VREG8)i, 0xE, 1, 0x0F, 0, 0, 0, 0);
        C = 1;
        TEST_OP_R8_R8((Cpu::VREG8)i, 0xE, 1, 0x10, 0, 0, 1, 0);

        C = 0;
        TEST_OP_R8_R8((Cpu::VREG8)i, 0xF2, 0xE, 0x00, 1, 0, 1, 1);
        C = 1;
        TEST_OP_R8_R8((Cpu::VREG8)i, 0xF2, 0xE, 0x01, 0, 0, 1, 1);

        code[0]++;
    }

    C = 0;
    TEST_OP_R8_R8(Cpu::VREG8_A, 0x88, 0x88, 0x10, 0, 0, 1, 1);
    C = 1;
    TEST_OP_R8_R8(Cpu::VREG8_A, 0x88, 0x88, 0x11, 0, 0, 1, 1);

    C = 0;
    TEST_OP_R8_R8(Cpu::VREG8_A, 0x80, 0x80, 0x00, 1, 0, 0, 1);
    C = 1;
    TEST_OP_R8_R8(Cpu::VREG8_A, 0x80, 0x80, 0x01, 0, 0, 0, 1);

    C = 0;
    TEST_OP_R8_R8(Cpu::VREG8_A, 0x11, 0x11, 0x22, 0, 0, 0, 0);
    C = 1;
    TEST_OP_R8_R8(Cpu::VREG8_A, 0x11, 0x11, 0x23, 0, 0, 0, 0);
}


// 0x9_
TEST(cpu, sub_r8_r8)
{
    CPU_CREATE(OP_SUB_B);

    for (size_t i = 0; i < 7; i++)
    {
        REG_HL = 0x1000;

        TEST_OP_R8_R8((Cpu::VREG8)i, 0, 1, 0xFF, 0, 1, 1, 1);

        TEST_OP_R8_R8((Cpu::VREG8)i, 0x10, 0x10, 0x00, 1, 1, 0, 0);

        TEST_OP_R8_R8((Cpu::VREG8)i, 0x10, 0x1, 0x0F, 0, 1, 1, 0);

        code[0]++;
    }

    TEST_OP_R8_R8(Cpu::VREG8_A, 0x88, 0x88, 0x00, 1, 1, 0, 0);

    TEST_OP_R8_R8(Cpu::VREG8_A, 0x80, 0x80, 0x00, 1, 1, 0, 0);

    TEST_OP_R8_R8(Cpu::VREG8_A, 0x11, 0x11, 0x00, 1, 1, 0, 0);
}
// 0x9_
TEST(cpu, sbc_r8_r8)
{
    CPU_CREATE(OP_SBC_A_B);

    for (size_t i = 0; i < 7; i++)
    {
        REG_HL = 0x1000;

        C = 0;
        TEST_OP_R8_R8((Cpu::VREG8)i, 0, 1, 0xFF, 0, 1, 1, 1);
        C = 1;
        TEST_OP_R8_R8((Cpu::VREG8)i, 0, 1, 0xFE, 0, 1, 1, 1);

        C = 0;
        TEST_OP_R8_R8((Cpu::VREG8)i, 0x10, 0x10, 0x00, 1, 1, 0, 0);
        C = 1;
        TEST_OP_R8_R8((Cpu::VREG8)i, 0x10, 0x10, 0xFF, 0, 1, 1, 1);

        C = 0;
        TEST_OP_R8_R8((Cpu::VREG8)i, 0x10, 0x1, 0x0F, 0, 1, 1, 0);
        C = 1;
        TEST_OP_R8_R8((Cpu::VREG8)i, 0x10, 0x1, 0x0E, 0, 1, 1, 0);

        code[0]++;
    }

    C = 0;
    TEST_OP_R8_R8(Cpu::VREG8_A, 0x88, 0x88, 0x00, 1, 1, 0, 0);
    C = 1;
    TEST_OP_R8_R8(Cpu::VREG8_A, 0x88, 0x88, 0xFF, 0, 1, 1, 1);

    C = 0;
    TEST_OP_R8_R8(Cpu::VREG8_A, 0x80, 0x80, 0x00, 1, 1, 0, 0);
    C = 1;
    TEST_OP_R8_R8(Cpu::VREG8_A, 0x80, 0x80, 0xFF, 0, 1, 1, 1);

    C = 0;
    TEST_OP_R8_R8(Cpu::VREG8_A, 0x11, 0x11, 0x00, 1, 1, 0, 0);
    C = 1;
    TEST_OP_R8_R8(Cpu::VREG8_A, 0x11, 0x11, 0xFF, 0, 1, 1, 1);
}

#define TEST_RET2(op, flag_expr, exp_pc, exp_sp) \
    REG_SP = 0x1000; \
    REG_PC = 0x0000; \
    code[0] = op; \
    flag_expr; \
    cpu.step(); \
    ASSERT_EQ(REG_PC, exp_pc); \
    ASSERT_EQ(REG_SP, exp_sp);

// 0x[C/D]0 / 0x[C/D]8 / 0x[C/D]9
TEST(cpu, ret)
{
    CPU_CREATE(OP_RET_NZ);

    ram[0] = 0xCD;
    ram[1] = 0xAB;

    TEST_RET2(OP_RET_NZ, Z=1, 0x0001, 0x1000);
    TEST_RET2(OP_RET_NZ, Z=0, 0xABCD, 0x1002);
    TEST_RET2(OP_RET_Z, Z=1, 0xABCD, 0x1002);
    TEST_RET2(OP_RET_Z, Z=0, 0x0001, 0x1000);

    TEST_RET2(OP_RET_NC, C=1, 0x0001, 0x1000);
    TEST_RET2(OP_RET_NC, C=0, 0xABCD, 0x1002);
    TEST_RET2(OP_RET_C, C=1, 0xABCD, 0x1002);
    TEST_RET2(OP_RET_C, C=0, 0x0001, 0x1000);
}

#define TEST_OP_SP_S8(x, y, res, h, c) \
    REG_PC = 0; \
    REG_SP = x; \
    code[1] = code[3] = y; \
    cpu.step(); \
    ASSERT_EQ(REG_HL, res); \
    CHECK_FLAGS(0, 0, h, c); \
    REG_SP = x; \
    cpu.step(); \
    ASSERT_EQ(REG_SP, res); \
    CHECK_FLAGS(0, 0, h, c);


TEST(cpu, op_sp_s8)
{
    CPU_CREATE(OP_LD_HL_SPI_r8, 0xFF, OP_ADD_SP_r8, 0xFF);

    TEST_OP_SP_S8(0x0000, 0xFF, 0xFFFF, 0, 0);

    TEST_OP_SP_S8(0x0001, 0xFF, 0x0000, 1, 1);

    TEST_OP_SP_S8(0x0001, 0x1, 0x0002, 0, 0);

    TEST_OP_SP_S8(0x0001, 0xF, 0x0010, 1, 0);

    TEST_OP_SP_S8(0x00F1, 0x10, 0x0101, 0, 1);

}
