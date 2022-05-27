#include "cart.hpp"
#include "unit.hpp"
#include "common/logging.hpp"
#include "memory.hpp"
#include "io.hpp"

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

    if (ram_size <= ARRAY_COUNT(size))
        return size[ram_size];

    UNREACHABLE("Invalid RAM size");
}

const char* CartHeader::cartType(CartridgeType cart_type)
{
    switch (cart_type)
    {
        case CartridgeType_ROM: return "ROM";
        case CartridgeType_MBC1: return "MBC1";
        case CartridgeType_MBC1_RAM: return "MBC1+RAM";
        case CartridgeType_MBC1_RAM_BATTERY: return "MBC1+RAM+BATTERY";
        case CartridgeType_MBC2: return "MBC2";
        case CartridgeType_MBC2_BATTERY: return "MBC2+BATTERY";
        case CartridgeType_ROM_RAM: return "ROM+RAM";
        case CartridgeType_ROM_RAM_BATTERY: return "ROM+RAM+BATTERY";
        case CartridgeType_MMM01: return "MMM01";
        case CartridgeType_MMM01_RAM: return "MMM01+RAM";
        case CartridgeType_MMM01_RAM_BATTERY: return "MMM01+RAM+BATTERY";
        case CartridgeType_MBC3_TIMER_BATTERY: return "MBC3+TIMER+BATTERY";
        case CartridgeType_MBC3_TIMER_RAM_BATTERY: return "MBC3+TIMER+RAM+BATTERY";
        case CartridgeType_MBC3: return "MBC3";
        case CartridgeType_MBC3_RAM: return "MBC3+RAM";
        case CartridgeType_MBC3_RAM_BATTERY: return "MBC3+RAM+BATTERY";
        case CartridgeType_MBC5: return "MBC5";
        case CartridgeType_MBC5_RAM: return "MBC5+RAM";
        case CartridgeType_MBC5_RAM_BATTERY: return "MBC5+RAM+BATTERY";
        case CartridgeType_MBC5_RUMBLE: return "MBC5+RUMBLE";
        case CartridgeType_MBC5_RUMBLE_RAM: return "MBC5+RUMBLE+RAM";
        case CartridgeType_MBC5_RUMBLE_RAM_BATTERY: return "MBC5+RUMBLE+RAM+BATTERY";
        case CartridgeType_MBC6: return "MBC6";
        case CartridgeType_MBC7_SENSOR_RUMBLE_RAM_BATTERY: return "MBC7+SENSOR+RUMBLE+RAM+BATTERY";
        case CartridgeType_POCKET_CAMERA: return "POCKET CAMERA";
        case CartridgeType_BANDAI_TAMA5: return "BANDAI TAMA5";
        case CartridgeType_HuC3: return "HuC3";
        case CartridgeType_HuC1_RAM_BATTERY: return "HuC1+RAM+BATTERY";
        default: UNREACHABLE("Invalid CartridgeType");
    }
}

Cart::Cart(std::vector<u8> rom) :
    m_rom(rom),
    m_external_ram(),
    m_header(data<const CartHeader>())
{
    m_external_ram.resize(m_header->ramSize());
    m_mbc1_mode = 0;
    m_mbc1_ram_bank = 0;
    m_mbc1_rom_bank = 1;
}

void Cart::mapMemory(Memory* mem, bool bootrom_enabled)
{
    // map the part of the cartridge that doesn't overlap with the bootrom
    u16 off = bootrom_enabled ? BOOTROM_SIZE : 0;
    mem->remapMemory(Mmio::RO(off, data(off), ROM0_SIZE - off));

    switch (m_header->cart_type)
    {
        case CartridgeType_ROM:
            mem->remapMemory(Mmio::RO(ROM1_START, data(ROM1_START), ROM1_SIZE));
            if (m_external_ram.size() > 0)
                mem->remapMemory(Mmio::RW(EXTRAM_START, m_external_ram.data(), EXTRAM_SIZE));
            break;

        case CartridgeType_MBC1:
        case CartridgeType_MBC1_RAM:
        case CartridgeType_MBC1_RAM_BATTERY:
        {
            auto write0 = writeFunc(&Cart::mbc1WriteRom0, mem);

            mem->getEntry(ROM0_START).value()->m_write_func = write0;
            if (bootrom_enabled)
                mem->getEntry(BOOTROM_END).value()->m_write_func = write0;

            mbc1RemapBank1(mem);
            mbc1RemapRAM(mem);;
            break;
        }

        default:
            LOG("type : {}({})\n", CartHeader::cartType(m_header->cart_type), m_header->cart_type);
            UNIMPLEMENTED("Only MBC1 supported");
    }
}

Result<void> Cart::mbc1WriteRom0(Memory* mem, u16 off, u8 data)
{
    // LOG("mbc1WriteRom0 [{:04X}] = {:02X}\n", off, data);

    // RAM ENABLE
    if (off < 0x2000)
    {
        // LOG("MBC1 WRITE (RAM Enable) [{:04X}] = {:02X}\n", off, data);

        m_mbc1_ram_enabled = data == 10;
        mbc1RemapRAM(mem);
    }

    // ROM Bank Number
    else if (off < 0x4000)
    {
        // LOG("MBC1 WRITE (ROM Bank Number) [{:04X}] = {:02X}\n", off, data);
        size_t bank_count = header()->romSize() / ROM_BANK_SIZE;

        // higher 3 bits are ignored
        data &= 0x1F;

        // Bank 0 cannot be mapped twice
        if (data == 0)
            data = 1;

        // if a rom needs less than 5 bits, the higher bits are discarded
        data &= bank_count - 1;

        // set lower 5 bits
        // m_rom_bank &= ~0x1F;
        // m_rom_bank |= data;
        m_mbc1_rom_bank_lo = data;

        mbc1RemapBank1(mem);
    }
    return {};
}

Result<void> Cart::mbc1WriteRom1(Memory* mem, u16 off, u8 data)
{
    off += ROM1_START;

    // RAM Bank Number / Rom Bank Number High
    if (off < 0x6000)
    {
        // LOG("MBC1 WRITE (RAM Bank Number) [{:04X}] = {:02X}\n", off, data);
        // 2 bit register
        data &= 3;

        // ROM
        if (m_mbc1_mode == 0)
        {
            m_mbc1_rom_bank_hi = data;
            mbc1RemapBank1(mem);
        }
        // RAM
        else
        {
            m_mbc1_ram_bank = data;
            mbc1RemapRAM(mem);
        }
    }

    // Mode Select
    else if (off < 0x8000)
    {
        // LOG("MBC1 WRITE (Mode Select) [{:04X}] = {:02X}\n", off, data);
        m_mbc1_mode = data & 1;
    }
    return {};
}

void Cart::mbc1RemapBank1(Memory* mem)
{
    // LOG("MBC1 ROM BANK 1 -> {}\n", m_mbc1_rom_bank);

    u8* bank = m_rom.data() + ROM_BANK_SIZE * m_mbc1_rom_bank;
    auto read1 = Mmio::readFunc(bank);
    auto write1 = writeFunc(&Cart::mbc1WriteRom1, mem);
    mem->remapMemory(Mmio{ROM1_START, ROM_BANK_SIZE, read1, write1});
}

void Cart::mbc1RemapRAM(Memory* mem)
{
    // LOG("MBC1 RAM BANK -> {}\n", m_mbc1_ram_bank);

    if (m_mbc1_ram_enabled)
    {
        u8* bank = m_external_ram.data() + RAM_BANK_SIZE * m_mbc1_ram_bank;
        mem->remapMemory(Mmio::RW(EXTRAM_START, bank, RAM_BANK_SIZE));
    }
    else
    {
        mem->unmapMemory(EXTRAM_START);
    }
}


};
