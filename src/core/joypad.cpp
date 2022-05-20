#include "joypad.hpp"
#include "io.hpp"
#include "memory.hpp"

namespace gbemu::core
{

Joypad::Joypad() :
    m_p1(0)
{

}

void Joypad::mapMemory(Memory* mem)
{
    // todo:
    mem->mapRegister(P1_ADDR, MmioReg::rw(&m_p1, 0b00001100));
}

}
