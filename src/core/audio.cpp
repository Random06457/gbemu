#include "audio.hpp"
#include "io.hpp"
#include "memory.hpp"

namespace gbemu::core
{

Audio::Audio()
{
}

void Audio::mapMemory(Memory* mem)
{
    mem->mapRegister(NR50_ADDR, MmioReg::rw(&m_nr50));
    mem->mapRegister(NR51_ADDR, MmioReg::rw(&m_nr51));
    mem->mapRegister(NR52_ADDR, MmioReg::rw(&m_nr52, 1 << 7));
    mem->mapRegister(NR11_ADDR, MmioReg::rw(&m_nr11)); // todo: read mask
    mem->mapRegister(NR12_ADDR, MmioReg::rw(&m_nr12));
    mem->mapRegister(NR13_ADDR, MmioReg::rw(&m_nr13));
    mem->mapRegister(NR14_ADDR, MmioReg::rw(&m_nr14));

}

}
