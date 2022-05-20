#pragma once

#include "attributes.hpp"
#include "device.hpp"
#include "result.hpp"

namespace gbemu::core
{

class Timer : public Device
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
    Timer();

    Result<void> resetDiv(u8 b);
    void tick(size_t clocks);
    size_t systemClocks() { return m_system_clock; }
    virtual void mapMemory(Memory* mem) override;

private:
    size_t m_div_start;
    size_t m_system_clock; // T-states

    u8 m_div;
    u8 m_tma;
    u8 m_tima;
    struct
    {
        u8 clock_select : 2;
        u8 timer_enable : 1;
    } PACKED m_tac;

};

}
