#pragma once

#include "result.hpp"
#include "types.hpp"
#include "cart.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
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
    Memory* mem() { return m_memory.get(); }
    Cart* cart() { return m_cart.get(); }
    GameboyType gbType() { return m_gb_type; }
    bool isGb(GameboyType type) { return m_gb_type == type; }

private:
    std::vector<u8> m_bootrom;
    bool m_bootrom_enabled;
    std::vector<u8> m_hram;
    std::unique_ptr<Memory> m_memory;
    std::unique_ptr<Cpu> m_cpu;
    std::unique_ptr<Ppu> m_ppu;
    GameboyType m_gb_type;
    std::unique_ptr<Cart> m_cart;
};

}
