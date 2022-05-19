#pragma once

#include "memory.hpp"

namespace gbemu::core
{

class Timer
{
    static constexpr size_t SYSTEM_FREQUENCY = 4194304; // T-states
    static constexpr size_t DIV_FREQUENCY = 16384; // T-states

    static constexpr size_t TAC_FREQUENCY[] =
    {
        4096,
        262144,
        65536,
        16384,
    };

public:
    Timer(Memory* memory);

    Memory* mem() { return m_memory; }

    Result<void> resetDiv(u8 b);
    void tick(size_t clocks);
    size_t systemClocks() { return m_system_clock; }

private:
    Memory* m_memory;
    size_t m_div_start;
    size_t m_system_clock; // T-states

    u8 m_div;
    u8 m_tma;
    u8 m_tima;
    struct
    {
        u8 clock_select : 2;
        u8 timer_enable : 1;
    } m_tac;

};

}
