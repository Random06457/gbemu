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
    mem->mapRW(NR50_ADDR, &m_nr50);
    mem->mapRW(NR51_ADDR, &m_nr51);
    mem->mapRW(NR52_ADDR, &m_nr52, 1, 1 << 7);
    mem->mapRW(NR11_ADDR, &m_nr11); // todo: read mask
    mem->mapRW(NR12_ADDR, &m_nr12);
    mem->mapRW(NR13_ADDR, &m_nr13);
    mem->mapRW(NR14_ADDR, &m_nr14);
}

}
