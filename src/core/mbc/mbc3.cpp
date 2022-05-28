#include "mbc3.hpp"
#include "core/io.hpp"
#include "core/memory.hpp"
#include "common/logging.hpp"

namespace gbemu::core
{

Mbc3::Mbc3(std::vector<u8>& rom) :
    Mbc(rom),
    m_extram()
{
    m_extram.resize(header()->ramSize());

    m_ram_and_timer_enabled = false;
    m_rom_bank = 1;
    m_ram_rtc_bank = 0;
    m_rtc_latched = false;
    m_latch_data = 0xFF;
    m_rts_s = 0;
    m_rts_m = 0;
    m_rts_h = 0;
    m_rts_dl = 0;
    m_rts_dh = 0;
}

void Mbc3::map(Memory* mem)
{
    mem->mapWO(MmioWrite(0x0000, 0x2000, writeFunc<Mbc3>(&Mbc3::writeRamTimerEnable, mem)));
    mem->mapWO(MmioWrite(0x2000, 0x2000, writeFunc<Mbc3>(&Mbc3::writeRomBank, mem)));
    mem->mapWO(MmioWrite(0x4000, 0x2000, writeFunc<Mbc3>(&Mbc3::writeRamRtcBank, mem)));
    mem->mapWO(MmioWrite(0x6000, 0x2000, writeFunc<Mbc3>(&Mbc3::writeLatchData, mem)));

    remapRomBank1(mem);
    remapRamRtc(mem);
}



Result<void> Mbc3::writeRamTimerEnable(Memory* mem, u16 off, u8 data)
{
    // 4 bits
    data &= 0b1111;

    bool enabled = data == 10;

    if (enabled != m_ram_and_timer_enabled)
    {
        m_ram_and_timer_enabled = data == 10;
        remapRamRtc(mem);
    }

    return {};
}

Result<void> Mbc3::writeRomBank(Memory* mem, u16 off, u8 data)
{
    // 7 bits
    data &= 0b1111111;

    if (data == 0)
        data = 1;

    if (data != m_rom_bank)
    {
        // TODO: does this happen on MBC3 as well?
        // if a rom needs less than 5 bits, the higher bits are discarded
        // size_t bank_count = header()->romSize() / ROM_BANK_SIZE;
        // data &= bank_count - 1;

        m_rom_bank = data;
        remapRomBank1(mem);
    }


    return {};
}

Result<void> Mbc3::writeRamRtcBank(Memory* mem, u16 off, u8 data)
{
    if ((data < 3 || (data >= 0x8 && data <= 0xC)) && data != m_ram_rtc_bank)
    {
        m_ram_rtc_bank = data;
        remapRamRtc(mem);
    }

    return {};
}

Result<void> Mbc3::writeLatchData(Memory* mem, u16 off, u8 data)
{
    bool latch = m_latch_data == 0 && data == 1;
    m_latch_data = data;

    m_rtc_latched ^= latch;

    // todo: write rtc clock and freeze
    return {};
}

Result<void> Mbc3::writeRtc(Memory* mem, u16 off, u8 data)
{
    switch (m_ram_rtc_bank)
    {
        case 0x8: m_rts_s = data; break;
        case 0x9: m_rts_m = data; break;
        case 0xA: m_rts_h = data; break;
        case 0xB: m_rts_dl = data; break;
        case 0xC: m_rts_dh = data; break;

        default: UNREACHABLE("Invalid RAM bank / RTC register select");
    }
    return {};
}

Result<u8> Mbc3::readRtc(Memory* mem, u16 off)
{
    switch (m_ram_rtc_bank)
    {
        case 0x8: return m_rts_s;
        case 0x9: return m_rts_m;
        case 0xA: return m_rts_h;
        case 0xB: return m_rts_dl;
        case 0xC: return m_rts_dh;

        default: UNREACHABLE("Invalid RAM bank / RTC register select");
    }
}


void Mbc3::remapRomBank1(Memory* mem)
{
    mem->remapRO(ROM1_START, m_rom.data() + m_rom_bank * ROM_BANK_SIZE, ROM_BANK_SIZE);
}

void Mbc3::remapRamRtc(Memory* mem)
{
    LOG("remapRamRtc\n");
    if (m_ram_and_timer_enabled)
    {
        if (m_ram_rtc_bank < 3)
            mem->remapRW(EXTRAM_START, m_extram.data() + m_ram_rtc_bank * RAM_BANK_SIZE, RAM_BANK_SIZE);
        else
            mem->remapRW(EXTRAM_START, readFunc<Mbc3>(&Mbc3::readRtc, mem), writeFunc<Mbc3>(&Mbc3::writeRtc, mem));
    }
    else
    {
        mem->unmapRW(EXTRAM_START);
    }
}

}
