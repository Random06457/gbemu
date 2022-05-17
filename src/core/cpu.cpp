#include "cpu.hpp"

namespace gbemu::core
{


void Cpu::reset()
{
    m_regs.af = 0;
    m_regs.bc = 0;
    m_regs.de = 0;
    m_regs.hl = 0;
    m_regs.pc = 0;
    m_regs.sp = 0xFFFF;
    m_clocks = 0;
}

void Cpu::step()
{

}

}
