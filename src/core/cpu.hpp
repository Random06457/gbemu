#pragma once

#include "memory.hpp"
#include "macro.hpp"

namespace gbemu::core
{

class Timer;

class Cpu
{
public:
    enum VREG8
    {
        VREG8_B,
        VREG8_C,
        VREG8_D,
        VREG8_E,
        VREG8_H,
        VREG8_L,
        VREG8_HL8,
        VREG8_A,

        VREG8_HLI, // (HI+)
        VREG8_HLD, // (HI-)
        VREG8_BC8, // (BC)
        VREG8_DE8, // (DE)
        VREG8_HA8, // (0xFF00 + a8)
        VREG8_HC, // (0xFF00 + C)
        VREG8_A16, // (a16)
        VREG8_D8, // d8/r8
    };

public:
    Cpu(Memory* memory, Timer* timer) :
        m_memory(memory),
        m_timer(timer)
    {
        reset();
    }

    void reset();
    void step();

    void push16(u16 x);
    u16 pop16();
    u8 fetch8();
    u16 fetch16();
    u8 read8(u16 addr);
    u16 read16(u16 addr);
    void write8(u16 addr, u8 x);
    void write16(u16 addr, u16 x);
    u8 readReg(VREG8 reg);
    void writeReg(VREG8 reg, u8 data);

private:

    void execute(u8 op);
    void executeCB(u8 op);
public:

    auto mem() { return m_memory; }
    auto& regs() { return m_regs; }

private:
    Memory* m_memory;
    Timer* m_timer;

    bool interrupt_master_enable;
    bool m_logging_enable;

// TODO: handle endianness
#define REG_8_16(x, y) \
    union { \
        u16 x##y; \
        struct { u8 y, x; }; \
    };
    struct
    {
        union
        {
            u16 af;
            struct
            {
                union
                {
                    u8 f;
                    struct
                    {
                        u8 : 4;
                        u8 c : 1;
                        u8 h : 1;
                        u8 n : 1;
                        u8 z : 1;
                    } flags;
                };
                u8 a;
            };
        };
        REG_8_16(b, c);
        REG_8_16(d, e);
        REG_8_16(h, l);
        u16 sp;
        u16 pc;
    } m_regs;
#undef REG_8_16
};

}
