#include "int_controller.hpp"
#include "io.hpp"
#include "memory.hpp"
#include "serial.hpp"
#include "timer.hpp"

namespace gbemu::core
{

Serial::Serial(InterruptController* interrupts) :
    m_interrupts(interrupts),
    m_counter(0)
{
}

void Serial::mapMemory(Memory* mem)
{
    mem->mapRW(SB_ADDR, &m_sb);
    mem->mapRW(SC_ADDR, &m_sc);
}

u8 Serial::getInputBit()
{
    return (m_input >> m_counter) & 1;
}

void Serial::step(size_t clocks)
{
    constexpr size_t freq = Timer::SYSTEM_FREQUENCY / CLOCK_FREQUENCY;

    if (clocks % freq == 0 && m_sc.transfer_flag)
    {
        m_sb <<= 1;
        m_sb |= getInputBit();
        m_counter++;

        if (m_counter == 8)
        {
            m_counter = 0;
            m_interrupts->requestInterrupt(InterruptType_Serial);
            m_sc.transfer_flag = 0;
        }
    }
}

}
