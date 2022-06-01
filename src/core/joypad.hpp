#pragma once

#include "device.hpp"

namespace gbemu::core
{

class InterruptController;

class Joypad : public Device
{
public:
    Joypad(InterruptController* interrupts);

    virtual void mapMemory(Memory* mem) override;
    void processInput();

private:
    union
    {
        u8 raw;
        struct
        {
            u8 right_a : 1;
            u8 left_b : 1;
            u8 up_select : 1;
            u8 down_start : 1;
            u8 select_direction : 1;
            u8 select_button : 1;
        };
    } PACKED m_p1;

    InterruptController* m_interrupts;
};

}
