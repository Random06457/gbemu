#include <cassert>
#include "cart.hpp"
#include "unit.hpp"

namespace gbemu::core
{

size_t CartHeader::romSize(u8 rom_size)
{
    if (rom_size <= 9)
        return 32_kb << rom_size;

    UNREACHABLE("Invalid ROM size");
}

size_t CartHeader::ramSize(u8 ram_size)
{
    size_t size[] = { 0, 0, 8_kb, 32_kb, 128_kb, 64_kb, };

    if (ram_size <= ARRAY_SIZE(size))
        return size[ram_size];

    UNREACHABLE("Invalid RAM size");
}

};
