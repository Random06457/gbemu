#include "gameboy.hpp"
#include "common/logging.hpp"
#include "apu.hpp"
#include "cart.hpp"
#include "cpu.hpp"
#include "int_controller.hpp"
#include "io.hpp"
#include "joypad.hpp"
#include "memory.hpp"
#include "ppu.hpp"
#include "serial.hpp"
#include "timer.hpp"

namespace gbemu::core
{

Gameboy::Gameboy() :
    m_bootrom(std::vector<u8>(BOOTROM_SIZE)),
    m_bootrom_enabled(true),
    m_hram(std::vector<u8>(HRAM_SIZE)),
    m_wram0(std::vector<u8>(WRAM0_SIZE)),
    m_wram1(std::vector<u8>(WRAM1_SIZE)),
    m_memory(std::make_unique<Memory>()),
    m_interrupt_controller(std::make_unique<InterruptController>()),
    m_timer(std::make_unique<Timer>(interrupts())),
    m_cpu(std::make_unique<Cpu>(mem(), timer(), interrupts())),
    m_ppu(std::make_unique<Ppu>(interrupts())),
    m_apu(std::make_unique<Apu>()),
    m_joypad(std::make_unique<Joypad>(interrupts())),
    m_serial(std::make_unique<Serial>(interrupts())),
    m_gb_type(GameboyType_DMG)
{
    // map bootrom
    mem()->mapRO(BOOTROM_START, m_bootrom.data(), m_bootrom.size());

    // map HRAM
    mem()->mapRW(HRAM_START, m_hram.data(), m_hram.size());

    // map WRAM (todo: make WRAM1 switchable for CGB)
    mem()->mapRW(WRAM0_START, m_wram0.data(), m_wram0.size());
    mem()->mapRW(WRAM1_START, m_wram1.data(), m_wram1.size());

    // map bootrom disable register
    mem()->mapWO(
        MmioWrite(BOOT_ADDR, 1,
                  std::bind(&Gameboy::disableBootRom, this,
                            std::placeholders::_1, std::placeholders::_2)));

    // map registers
    m_interrupt_controller->mapMemory(mem());
    m_apu->mapMemory(mem());
    m_ppu->mapMemory(mem());
    m_timer->mapMemory(mem());
    m_joypad->mapMemory(mem());
    m_serial->mapMemory(mem());

    // stub register
    static u8 stub = 0;
    mem()->mapRO(KEY1_ADDR, &stub);
}

Gameboy::~Gameboy()
{
}

Result<void> Gameboy::disableBootRom(u16 off, u8 data)
{
    if (data == 0)
        return {};

    if (m_bootrom_enabled)
    {
        // unmap bootrom
        mem()->unmapRO(BOOTROM_START);
        mem()->unmapRO(BOOTROM_END);
        // map entire cartridge bank 0
        cart()->mapMemory(mem(), false);
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

    m_cart->mapMemory(mem(), m_bootrom_enabled);

    return {};
}

void Gameboy::step()
{
    size_t old_clocks = timer()->systemClocks();

    interrupts()->processInterrupts(cpu());
    cpu()->step();

    size_t new_clocks = timer()->systemClocks();
    size_t clocks_diff = new_clocks - old_clocks;

    joypad()->processInput();
    ppu()->step(mem(), timer()->systemClocks());
    apu()->step(clocks_diff);
}

Result<void> Gameboy::powerOn()
{
    cpu()->reset();
    return {};
}

}
