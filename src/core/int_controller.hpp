#pragma once

#include "device.hpp"

namespace gbemu::core
{

enum InterruptType : u8
{
    InterruptType_Vblank,
    InterruptType_LCDSTA,
    InterruptType_Timer,
    InterruptType_Serial,
    InterruptType_Joypad,

    InterruptType_Count,
    InterruptType_None = 0xFF,
};

class Cpu;

class InterruptController : public Device
{
public:
    InterruptController();

    virtual void mapMemory(Memory* mem) override;

    void requestInterrupt(InterruptType type);
    void processInterrupts(Cpu* cpu);
    void setIME(bool enable) { m_ime = enable; }

private:
    bool m_ime; // interrupt master flag
    union
    {
        struct
        {
            u8 vblank_int_req : 1;
            u8 lcd_int_req : 1;
            u8 timer_int_req : 1;
            u8 serial_int_req : 1;
            u8 joypad_int_req : 1;
        };
        u8 raw;
    } PACKED m_if; // Interrupt Flag (R/W)
    union
    {
        struct
        {
            u8 vblank_int_enable : 1;
            u8 lcd_int_enable : 1;
            u8 timer_int_enable : 1;
            u8 serial_int_enable : 1;
            u8 joypad_int_enable : 1;
        };
        u8 raw;
    } PACKED m_ie; // Interrupt Enable (R/W)
};

}
