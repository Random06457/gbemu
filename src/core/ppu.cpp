#include "ppu.hpp"
#include "memory.hpp"
#include "common/fs.hpp"
#include "common/logging.hpp"

namespace gbemu::core
{

void Ppu::drawTile(u32* dst, u8* tile)
{
    for (size_t y = 0; y < 8; y++)
    {
        u8 b0 = *tile++;
        u8 b1 = *tile++;

        for (size_t x = 0; x < 8; x++)
        {
            u8 bit0 = (b0 >> (7 - x)) & 1;
            u8 bit1 = (b1 >> (7 - x)) & 1;

            dst[y*32*8+x] = getColor(m_dmg_bgp, (bit0 << 1) | bit1);
        }
    }
}

void Ppu::drawTiles(bool bg)
{
    u8* map = bg ? bgMap() : windowMap();
    u32* dst = bg ? m_bg_texture : m_window_texture;
    u8* tiles = bgTiles();

    for (size_t i = 0; i < 32*32; i++)
    {
        u8 tile_off = m_lcdc.bg_tile_area ? map[i] : map[i] ^ 0x80;

        u8* src = tiles + tile_off * 0x10;

        size_t x = (i % 32) * 8;
        size_t y = (i / 32) * 8;

        drawTile(dst + y * 32*8 + x, src);
    }
}

Ppu::Ppu(InterruptController* interrupt) :
    m_interrupt(interrupt),
    m_vram_bank(0)
{
    m_dmg_colors[0] = 0xFFFFFFFF;
    m_dmg_colors[2] = 0xFFAAAAAA;
    m_dmg_colors[1] = 0xFF555555;
    m_dmg_colors[3] = 0xFF000000;
}

void Ppu::mapMemory(Memory* mem)
{
    mem->mapBuffer(OAM_START, oam(), OAM_SIZE);

    mem->mapRegister(BGP_ADDR, MmioReg::rw(&m_dmg_bgp));
    mem->mapRegister(OBP0_ADDR, MmioReg::rw(&m_dmg_obp[0]));
    mem->mapRegister(OBP1_ADDR, MmioReg::rw(&m_dmg_obp[1]));
    mem->mapRegister(SCX_ADDR, MmioReg::rw(&m_scx));
    mem->mapRegister(SCY_ADDR, MmioReg::rw(&m_scy));
    mem->mapRegister(LCDC_ADDR, MmioReg::rw(&m_lcdc));
    mem->mapRegister(STAT_ADDR, MmioReg::rw(&m_stat, 0b01111100));
    mem->mapRegister(LY_ADDR, MmioReg::ro(&m_ly));
    mem->mapRegister(LYC_ADDR, MmioReg::ro(&m_lyc));

    switchBank(mem, 0);
}

void Ppu::switchBank(Memory* mem, size_t bank)
{
    m_vram_bank = bank;
    mem->remapBuffer(VRAM_START, vram(), VRAM_SIZE);
}

u32 Ppu::getColor(u8 palette, u8 idx)
{
    return m_dmg_colors[(palette >> idx*2) & 3];
}


void Ppu::step(size_t clocks)
{
    constexpr size_t oam_cycles = 20 * 4;
    constexpr size_t transfer_cycles = 43 * 4;
    constexpr size_t hblank_cycles = 41 * 4;
    constexpr size_t transfer_off = oam_cycles;
    constexpr size_t hblank_off = transfer_off + transfer_cycles;
    constexpr size_t line_cycles = oam_cycles + transfer_cycles + hblank_cycles;
    constexpr size_t vblank_cycles = line_cycles * 10;
    constexpr size_t screen_cycles = line_cycles * (SCREEN_HEIGHT + 10);
    size_t line_off = clocks % line_cycles;

    m_ly = (clocks % screen_cycles) / line_cycles;

    m_stat.lyc_eq_lc = m_ly == m_lyc;
    if (m_stat.lyc_eq_lc && m_stat.lyc_int_enable)
        m_interrupt->requestInterrupt(InterruptType_LCDSTA);

    PpuMode prev_mode = m_stat.mode;

    if (m_ly > SCREEN_HEIGHT)
    {
        if (prev_mode != PpuMode_VBlank)
        {
            m_stat.mode = PpuMode_VBlank;
            if (m_stat.vblank_int_enable)
                m_interrupt->requestInterrupt(InterruptType_LCDSTA);
        }
    }
    else if (line_off >= hblank_off)
    {
        if (prev_mode != PpuMode_HBlank)
        {
            m_stat.mode = PpuMode_HBlank;
            if (m_stat.hblank_int_enable)
                m_interrupt->requestInterrupt(InterruptType_LCDSTA);
        }
    }
    else if (line_off >= transfer_off)
    {
        if (prev_mode != PpuMode_PixelTransfer)
            m_stat.mode = PpuMode_PixelTransfer;
    }
    else if (prev_mode != PpuMode_OamSearch)
    {
        m_stat.mode = PpuMode_OamSearch;
        if (m_stat.oam_int_enable)
            m_interrupt->requestInterrupt(InterruptType_LCDSTA);
    }

    if (prev_mode == PpuMode_HBlank && m_stat.mode == PpuMode_VBlank)
    {
        m_interrupt->requestInterrupt(InterruptType_Vblank);
        drawTiles(true);
        drawTiles(false);
        m_new_frame_available = true;
    }
}


void Ppu::dumpBg()
{
    drawTiles(true);
    fmt::print("Dumping background\n");
    fmt::print("palette : 0x{:02X}\n", m_dmg_bgp);
    File::writeAllBytes("bg.raw", m_bg_texture, sizeof(m_bg_texture));
    File::writeAllBytes("bg_map.bin", bgMap(), 32*32*0x10);
    File::writeAllBytes("window_map.bin", windowMap(), 32*32*0x10);
    File::writeAllBytes("bg_tiles.bin", bgTiles(), 0x100*0x10);
}

}
