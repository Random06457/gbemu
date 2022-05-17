#pragma once

#include "memory.hpp"
#include "macro.hpp"

namespace gbemu::core
{

class Cpu
{
public:
    Cpu(Memory* memory) :
        m_memory(memory),
        m_clocks(0)
    {}


    void reset();
    void step();

    GETTER(auto, memory);
    void tick(size_t x) { m_clocks += x; }

private:
    Memory* m_memory;
    size_t m_clocks;

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
