#pragma once

#include "device.hpp"
#include "int_controller.hpp"

namespace gbemu::core
{

class Serial : public Device
{
public:
    static constexpr size_t CLOCK_FREQUENCY = 8192;

public:
    Serial(InterruptController* interrupts);

    virtual void mapMemory(Memory* mem) override;

    void step(size_t clocks);

    u8 getInputBit();

private:
    InterruptController* m_interrupts;
    size_t m_counter;
    u8 m_input;

    u8 m_sb;
    struct
    {
        u8 shift_clock : 1;
        u8 cgb_clock_speed : 1;
        u8 : 5;
        u8 transfer_flag : 1;
    } PACKED m_sc;
};

}
