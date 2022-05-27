#include "int_controller.hpp"
#include "io.hpp"
#include "cpu.hpp"
#include "common/logging.hpp"
#include "memory.hpp"

namespace gbemu::core
{

static const char* s_int_names[] =
{
    "Vblank",
    "Lcd",
    "Timer",
    "Serial",
    "Joypad",
};

InterruptController::InterruptController()
{
    m_ime = false;
    m_if.raw = 0;
    m_ie.raw = 0;
}

void InterruptController::mapMemory(Memory* mem)
{
    mem->mapMemory(Mmio::RW(IE_ADDR, &m_ie, 1));
    mem->mapMemory(Mmio::RW(IF_ADDR, &m_if, 1));
}


void InterruptController::requestInterrupt(InterruptType type)
{
    m_if.raw |= 1 << type;
}

void InterruptController::processInterrupts(Cpu* cpu)
{
    if (!m_ime && !cpu->isHalted())
        return;

    for (size_t i = 0; i < InterruptType_Count; i++)
    {
        u8 bit = 1 << i;
        if ((m_if.raw & bit) && (m_ie.raw & bit))
        {
            // LOG("*** INTERRUPT {} ***\n", s_int_names[i]);

            cpu->unhalt();

            if (m_ime)
            {
                u16 addr = 0x40 + 8 * i;
                m_if.raw &= ~bit;
                m_ime = false;
                cpu->push16(cpu->regs().pc);
                cpu->regs().pc = addr;
            }

            break;
        }
    }
}

}
