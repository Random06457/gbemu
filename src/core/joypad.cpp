#include "joypad.hpp"
#include "io.hpp"
#include "memory.hpp"

namespace gbemu::core
{

Joypad::Joypad(InterruptController* interrupts) :
    m_p1({0}),
    m_interrupts(interrupts)
{

}

void Joypad::mapMemory(Memory* mem)
{
    mem->mapRegister(P1_ADDR, MmioReg::rw(&m_p1, 0b00110000));
}

}
