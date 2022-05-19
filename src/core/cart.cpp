#include "cart.hpp"
#include "unit.hpp"
#include "common/logging.hpp"

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

};
