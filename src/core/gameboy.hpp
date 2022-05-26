#pragma once

#include "result.hpp"
#include "types.hpp"
#include "cart.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "audio.hpp"
#include "timer.hpp"
#include "joypad.hpp"
#include "int_controller.hpp"
#include "serial.hpp"
#include "memory.hpp"
#include <vector>
#include <memory>

namespace gbemu::core
{

enum GameboyType
{
    GameboyType_DMG,
    GameboyType_CGB,
    GameboyType_SGB,
};

class Gameboy
{
public:
    Gameboy();
    ~Gameboy() {}

    Result<void> setCartridge(std::unique_ptr<Cart> cart);
    Result<void> setBootrom(std::vector<u8> bootrom);
    Result<void> powerOn();
    void step();

    Result<void> disableBootRom(u8 data);

public:
    Cpu* cpu() { return m_cpu.get(); }
    Ppu* ppu() { return m_ppu.get(); }
    Timer* timer() { return m_timer.get(); }
    Memory* mem() { return m_memory.get(); }
    InterruptController* interrupts() { return m_interrupt_controller.get(); }
    Joypad* joypad() { return m_joypad.get(); }
    Cart* cart() { return m_cart.get(); }
    GameboyType gbType() { return m_gb_type; }
    bool isGb(GameboyType type) { return m_gb_type == type; }

private:
    std::vector<u8> m_bootrom;
    bool m_bootrom_enabled;
    std::vector<u8> m_hram;
    std::vector<u8> m_wram0;
    std::vector<u8> m_wram1; // switchable in cgb mode
    std::unique_ptr<Memory> m_memory;
    std::unique_ptr<InterruptController> m_interrupt_controller;
    std::unique_ptr<Timer> m_timer;
    std::unique_ptr<Cpu> m_cpu;
    std::unique_ptr<Ppu> m_ppu;
    std::unique_ptr<Audio> m_audio;
    std::unique_ptr<Joypad> m_joypad;
    std::unique_ptr<Serial> m_serial;
    GameboyType m_gb_type;
    std::unique_ptr<Cart> m_cart;
};

}
