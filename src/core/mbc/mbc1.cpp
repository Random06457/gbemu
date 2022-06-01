#include "mbc1.hpp"
#include "core/io.hpp"
#include "core/memory.hpp"

namespace gbemu::core
{

Mbc1::Mbc1(std::vector<u8>& rom) : Mbc(rom), m_extram()
{
    m_extram.resize(header()->ramSize());

    m_mode = 0;
    m_ram_bank = 0;
    m_rom_bank = 1;
}

void Mbc1::map(Memory* mem)
{
    mem->mapWO(
        MmioWrite(0x0000, 0x2000, writeFunc<Mbc1>(&Mbc1::writeRamEnable, mem)));
    mem->mapWO(MmioWrite(0x2000, 0x2000,
                         writeFunc<Mbc1>(&Mbc1::writeRomBankNumber, mem)));
    mem->mapWO(MmioWrite(0x4000, 0x2000,
                         writeFunc<Mbc1>(&Mbc1::writeRamBankNumber, mem)));
    mem->mapWO(MmioWrite(0x6000, 0x2000,
                         writeFunc<Mbc1>(&Mbc1::writeSelectMode, mem)));

    remapBank1(mem);
    remapRAM(mem);
}

Result<void> Mbc1::writeRamEnable(Memory* mem, u16 off, u8 data)
{
    // 4 bits
    data &= 0b1111;

    m_ram_enabled = data == 10;
    remapRAM(mem);

    return {};
}
Result<void> Mbc1::writeRomBankNumber(Memory* mem, u16 off, u8 data)
{
    // 5 bits
    data &= 0b11111;

    size_t bank_count = header()->romSize() / ROM_BANK_SIZE;

    // Bank 0 cannot be mapped twice
    if (data == 0)
        data = 1;

    // if a rom needs less than 5 bits, the higher bits are discarded
    data &= bank_count - 1;

    m_rom_bank_lo = data;

    remapBank1(mem);

    return {};
}
Result<void> Mbc1::writeRamBankNumber(Memory* mem, u16 off, u8 data)
{
    // 2 bits
    data &= 0b11;

    // ROM
    if (m_mode == 0)
    {
        m_rom_bank_hi = data;
        remapBank1(mem);
    }
    // RAM
    else
    {
        m_ram_bank = data;
        remapRAM(mem);
    }

    return {};
}
Result<void> Mbc1::writeSelectMode(Memory* mem, u16 off, u8 data)
{
    // 1 bit
    data &= 0b1;

    m_mode = data;

    return {};
}

void Mbc1::remapBank1(Memory* mem)
{
    // LOG("MBC1 ROM BANK 1 -> {}\n", m_rom_bank);

    auto bank = m_rom.data() + ROM_BANK_SIZE * m_rom_bank;
    mem->remapRO(ROM1_START, bank, ROM_BANK_SIZE);
}

void Mbc1::remapRAM(Memory* mem)
{
    // LOG("MBC1 RAM BANK -> {}\n", m_ram_bank);

    if (m_ram_enabled)
    {
        auto bank = m_extram.data() + RAM_BANK_SIZE * m_ram_bank;
        mem->remapRW(EXTRAM_START, bank, RAM_BANK_SIZE);
    }
    else
    {
        mem->unmapRW(EXTRAM_START);
    }
}

}
