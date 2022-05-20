#include "int_controller.hpp"
#include "io.hpp"
#include "cpu.hpp"
#include "common/logging.hpp"

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

InterruptController::InterruptController(Memory* memory) :
    m_memory(memory)
{
    m_memory->mapRegister(IE_ADDR, MmioReg::rw(&m_ie));
    m_memory->mapRegister(IF_ADDR, MmioReg::rw(&m_if));

    m_ime = false;
    m_if.raw = 0;
    m_ie.raw = 0;
}


void InterruptController::requestInterrupt(InterruptType type)
{
    m_if.raw |= 1 << type;
}

void InterruptController::processInterrupts(Cpu* cpu)
{
    for (size_t i = 0; i < InterruptType_Count; i++)
    {
        u8 bit = 1 << i;
        u16 addr = 0x40 + 8 * i;
        if ((m_if.raw & bit) && (m_ie.raw & bit))
        {
            m_if.raw &= ~bit;
            m_ime = false;

            LOG("*** INTERRUPT {} ***\n", s_int_names[i]);

            cpu->regs().pc = addr;

            break;
        }
    }
}

}
