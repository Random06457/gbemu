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
    mem->mapMemory(Mmio::RW(NR50_ADDR, &m_nr50, 1));
    mem->mapMemory(Mmio::RW(NR51_ADDR, &m_nr51, 1));
    mem->mapMemory(Mmio::RW(NR52_ADDR, &m_nr52, 1, 1 << 7));
    mem->mapMemory(Mmio::RW(NR11_ADDR, &m_nr11, 1)); // todo: read mask
    mem->mapMemory(Mmio::RW(NR12_ADDR, &m_nr12, 1));
    mem->mapMemory(Mmio::RW(NR13_ADDR, &m_nr13, 1));
    mem->mapMemory(Mmio::RW(NR14_ADDR, &m_nr14, 1));

}

}
