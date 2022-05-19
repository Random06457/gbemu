#include "gameboy.hpp"
#include "io.hpp"
#include <assert.h>

namespace gbemu::core
{


Gameboy::Gameboy() :
    m_bootrom(std::vector<u8>(BOOTROM_SIZE)),
    m_bootrom_enabled(true),
    m_hram(std::vector<u8>(HRAM_SIZE)),
    m_memory(std::make_unique<Memory>()),
    m_cpu(std::make_unique<Cpu>(mem())),
    m_ppu(std::make_unique<Ppu>(mem())),
    m_gb_type(GameboyType_DMG)
{
    // map bootrom
    mem()->mapBuffer(BOOTROM_START, m_bootrom.data(), m_bootrom.size());

    // map HRAM
    mem()->mapBuffer(HRAM_START, m_hram.data(), m_hram.size());

    // map bootrom disable register
    mem()->mapRegister(BOOT_ADDR, MmioReg::wo(std::bind(&Gameboy::disableBootRom, this, std::placeholders::_1)));
}


Result<void> Gameboy::disableBootRom(u8 data)
{
    if (data == 0)
        return {};

    if (m_bootrom_enabled)
    {
        // unmap bootrom
        mem()->unmapAddress(BOOTROM_START);
        mem()->unmapAddress(BOOTROM_END);
        // map entire cartridge bank 0
        mem()->mapBuffer(ROM0_START, cart()->data(), ROM0_SIZE);
    }
    m_bootrom_enabled = false;
    return {};
}

Result<void> Gameboy::setBootrom(std::vector<u8> bootrom)
{
    ERROR_IF(bootrom.size() != BOOTROM_SIZE, Errro_InvalidBootromSize);

    m_bootrom = bootrom;

    return {};
}

Result<void> Gameboy::setCartridge(std::unique_ptr<Cart> cart)
{
    m_cart = std::move(cart);

    // TODO: change m_gb_type

    // map the part of the cartridge that doesn't overlap with the bootrom
    mem()->remapBuffer(BOOTROM_END, m_cart->data(BOOTROM_SIZE), ROM0_SIZE - BOOTROM_SIZE);

    switch (m_cart->header()->cart_type)
    {
        // TODO: map rest of cartridge
        case CartridgeType_ROM:
        case CartridgeType_MBC1:
            mem()->remapBuffer(ROM1_START, m_cart->data(ROM0_SIZE), ROM1_SIZE);
            break;
        default:
            printf("type : %d\n", m_cart->header()->cart_type);
            UNIMPLEMENTED("Only MBC1 supported");
    }

    return {};
}

void Gameboy::step()
{
    cpu()->step();
    ppu()->step(cpu()->clocks());
}

Result<void> Gameboy::powerOn()
{
    cpu()->reset();
    return {};
}

}
