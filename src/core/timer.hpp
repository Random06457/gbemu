#pragma once

#include "attributes.hpp"
#include "device.hpp"
#include "result.hpp"
#include "int_controller.hpp"

namespace gbemu::core
{

class Timer : public Device
{
    static constexpr size_t SYSTEM_FREQUENCY = 4194304; // T-states
    static constexpr size_t DIV_FREQUENCY = 16384; // T-states

    static constexpr size_t TAC_FREQUENCY[] =
    {
        4096, // T-cycles / 1024
        262144, // T-cycles / 16
        65536, // T-cycles / 64
        16384, // T-cycles / 256
    };

public:
    Timer(InterruptController* interrupt);

    Result<void> resetDiv(u8 b);
    void tick(size_t clocks);
    size_t systemClocks() { return m_system_clock; }
    virtual void mapMemory(Memory* mem) override;

private:
    InterruptController* m_interrupt;
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
