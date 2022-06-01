#pragma once

#include <bits/unique_ptr.h>
#include <vector>
#include "attributes.hpp"
#include "result.hpp"
#include "types.hpp"
#include "unit.hpp"

namespace gbemu::core
{

class Memory;

static constexpr size_t ROM_BANK_SIZE = 16_kb;
static constexpr size_t RAM_BANK_SIZE = 8_kb;

enum CartridgeType : u8
{
    CartridgeType_ROM = 0x00,
    CartridgeType_MBC1 = 0x01,
    CartridgeType_MBC1_RAM = 0x02,
    CartridgeType_MBC1_RAM_BATTERY = 0x03,
    CartridgeType_MBC2 = 0x05,
    CartridgeType_MBC2_BATTERY = 0x06,
    CartridgeType_ROM_RAM = 0x08,
    CartridgeType_ROM_RAM_BATTERY = 0x09,
    CartridgeType_MMM01 = 0x0B,
    CartridgeType_MMM01_RAM = 0x0C,
    CartridgeType_MMM01_RAM_BATTERY = 0x0D,
    CartridgeType_MBC3_TIMER_BATTERY = 0x0F,
    CartridgeType_MBC3_TIMER_RAM_BATTERY = 0x10,
    CartridgeType_MBC3 = 0x11,
    CartridgeType_MBC3_RAM = 0x12,
    CartridgeType_MBC3_RAM_BATTERY = 0x13,
    CartridgeType_MBC5 = 0x19,
    CartridgeType_MBC5_RAM = 0x1A,
    CartridgeType_MBC5_RAM_BATTERY = 0x1B,
    CartridgeType_MBC5_RUMBLE = 0x1C,
    CartridgeType_MBC5_RUMBLE_RAM = 0x1D,
    CartridgeType_MBC5_RUMBLE_RAM_BATTERY = 0x1E,
    CartridgeType_MBC6 = 0x20,
    CartridgeType_MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
    CartridgeType_POCKET_CAMERA = 0xFC,
    CartridgeType_BANDAI_TAMA5 = 0xFD,
    CartridgeType_HuC3 = 0xFE,
    CartridgeType_HuC1_RAM_BATTERY = 0xFF,
};

enum DestinationCode : u8
{
    DestinationCode_Japanese = 0x00,
    DestinationCode_NonJapanese = 0x01,
};

struct CartHeader
{
    /* 0x000 */ u8 jump_vector_table[0x100];
    /* 0x100 */ u8 entrypoint[4];
    /* 0x104 */ u8 nintendo_logo[0x30];
    union
    {
        /* 0x134 */ char title_old[0x10];
        struct
        {
            /* 0x134 */ char title[0xB];
            /* 0x13F */ char manufacturer_code[4];
            /* 0x143 */ u8 cgb_flag;
        };
    };
    /* 0x144 */ u8 new_licensee_code[2];
    /* 0x146 */ u8 sgb_flag;
    /* 0x147 */ CartridgeType cart_type;
    /* 0x148 */ u8 rom_size;
    /* 0x149 */ u8 ram_size;
    /* 0x14A */ DestinationCode destination_code;
    /* 0x14B */ u8 old_licensee_code;
    /* 0x14C */ u8 rom_version;
    /* 0x14D */ u8 checksum;
    /* 0x14E */ u8 global_checksum[2];

    static size_t romSize(u8 rom_size);
    size_t romSize() const { return romSize(rom_size); }

    static size_t ramSize(u8 rom_size);
    size_t ramSize() const { return ramSize(ram_size); }

    static const char* cartType(CartridgeType cart_type);
    const char* cartType() const { return cartType(cart_type); }
} PACKED;
static_assert(sizeof(CartHeader) == 0x150);

class Mbc
{
public:
    Mbc(const std::vector<u8>& rom) : m_rom(rom) {}
    virtual ~Mbc(){};

    virtual void map(Memory* mem) = 0;

    auto header() const
    {
        return reinterpret_cast<const CartHeader*>(m_rom.data());
    }

protected:
    template<typename T, typename... TArgs>
    auto writeFunc(auto func, TArgs... args)
    {
        return std::bind(func, reinterpret_cast<T*>(this),
                         std::forward<TArgs>(args)..., std::placeholders::_1,
                         std::placeholders::_2);
    }
    template<typename T, typename... TArgs>
    auto readFunc(auto func, TArgs... args)
    {
        return std::bind(func, reinterpret_cast<T*>(this),
                         std::forward<TArgs>(args)..., std::placeholders::_1);
    }

protected:
    const std::vector<u8>& m_rom;
};

class Cart
{
public:
    Cart(std::vector<u8> rom);

public:
    template<typename T = void>
    T* data(size_t off = 0)
    {
        return reinterpret_cast<T*>(m_rom.data() + off);
    }

    void mapMemory(Memory* mem, bool bootrom_enabled);

    auto header() const { return m_header; }
    auto& rom() { return m_rom; };

private:
    std::vector<u8> m_rom;
    const CartHeader* m_header;
    std::unique_ptr<Mbc> m_mbc;
};

}
