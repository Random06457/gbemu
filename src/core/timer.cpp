#include "timer.hpp"
#include "common/logging.hpp"
#include "int_controller.hpp"
#include "io.hpp"
#include "memory.hpp"

namespace gbemu::core
{

Timer::Timer(InterruptController* interrupt) :
    m_interrupt(interrupt),
    m_system_clock(0)
{
}

void Timer::mapMemory(Memory* mem)
{
    mem->mapRW(DIV_ADDR, MmioRead::readFunc(&m_div),
               std::bind(&Timer::resetDiv, this, std::placeholders::_1));
    mem->mapRW(TIMA_ADDR, &m_tima);
    mem->mapRW(TMA_ADDR, &m_tma);
    mem->mapRW(TAC_ADDR, &m_tac);
}

Result<void> Timer::resetDiv(u8 b)
{
    m_div = 0;
    m_div_start = m_system_clock;
    return {};
}

void Timer::tick(size_t clocks)
{
    m_system_clock += clocks;
    m_div = (m_div_start - m_system_clock) / (SYSTEM_FREQUENCY / DIV_FREQUENCY);

    size_t freq = SYSTEM_FREQUENCY / TAC_FREQUENCY[m_tac.clock_select];
    bool should_tick = m_system_clock % freq == 0;

    if (m_tac.timer_enable && should_tick)
    {
        m_tima++;
        // overflow
        if (m_tima == 0)
        {
            m_tima = m_tma;
            m_interrupt->requestInterrupt(InterruptType_Timer);
        }
    }
}

}
