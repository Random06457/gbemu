#include "rom.hpp"
#include "core/io.hpp"
#include "core/memory.hpp"

namespace gbemu::core
{


Rom::Rom(std::vector<u8>& rom) :
    Mbc(rom)
{

}

void Rom::map(Memory* mem)
{
    mem->remapRO(ROM1_START, m_rom.data() + ROM1_START, ROM1_SIZE);
    // if (m_external_ram.size() > 0)
    //     mem->remapRW(EXTRAM_START, m_external_ram.data(), EXTRAM_SIZE);
}

}
