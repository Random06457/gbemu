#pragma once

#include "device.hpp"
#include "io.hpp"
#include "int_controller.hpp"
#include "result.hpp"

namespace gbemu::core
{

static constexpr size_t SCREEN_WIDTH = 160;
static constexpr size_t SCREEN_HEIGHT = 144;

struct OamEntry
{
    u8 y;
    u8 x;
    u8 tile;
    u8 cgb_palette : 2;
    u8 vram_bank : 1;
    u8 dmg_palette : 1;
    // TODO: is this order correct? this doesn't match pandocs
    u8 bg_and_window_over_obj : 1;
    u8 flip_x : 1;
    u8 flip_y : 1;
} PACKED;
static_assert(sizeof(OamEntry) == 4);

enum PpuMode : u8
{
    PpuMode_HBlank,
    PpuMode_VBlank,
    PpuMode_OamSearch,
    PpuMode_PixelTransfer,
};

class Ppu : public Device
{
public:
    Ppu(InterruptController* interrupt);

public:

    virtual void mapMemory(Memory* mem) override;

    auto vram() { return  m_vram[m_vram_bank]; }
    auto oam() { return  m_oam; }
    void switchBank(Memory* mem, size_t bank);

    void step(Memory* mem, size_t clocks);

    u32 getColor(u8 palette, u8 idx, bool transparency);

    u8* spriteTiles() { return vram(); }
    u8* bgTiles() { return vram() + (m_lcdc.bg_tile_area ? 0 : 0x800); }
    u8* bgMap() { return vram() + (m_lcdc.bg_map_area ? 0x1C00 : 0x1800); }
    u8* windowMap() { return vram() + (m_lcdc.window_map_area ? 0x1C00 : 0x1800); }

    Result<void> startDMA(u8 addr);

    void drawLine(size_t screen_y);
    void oamSearch(size_t screen_y);

    void drawTiles(bool bg);
    void drawTile(u32* dst, u8* tile);

    void dumpBg();

    void render();

    bool newFrameAvailable() const { return m_new_frame_available; }

    auto lcdc() { return m_lcdc; }

private:
    InterruptController* m_interrupt;
    size_t m_vram_bank;
    u8 m_vram[2][VRAM_SIZE]; // switchable bank in CGB mode
    OamEntry m_oam[40];

    u8 m_dmg_bgp; // non-CGB
    u8 m_dmg_obp[2]; // non-CGB
    bool m_new_frame_available;

    union
    {
        struct
        {
            u8 bg_and_window_enable : 1;
            u8 obj_enable : 1;
            u8 obj_size : 1; // 0=8x8, 1=8x16
            u8 bg_map_area : 1; // 0=9800-9BFF, 1=9C00-9FFF
            u8 bg_tile_area : 1; // 0=8800-97FF, 1=8000-8FFF
            u8 window_enable : 1;
            u8 window_map_area : 1; // 0=9800-9BFF, 1=9C00-9FFF
            u8 lcd_enable : 1;
        };
        u8 raw;
    } PACKED m_lcdc;

    struct
    {
        PpuMode mode : 2; //0=HBlank, 1=VBlank, 2=OAM, 3=Transfer
        u8 lyc_eq_lc : 1;
        u8 hblank_int_enable : 1;
        u8 vblank_int_enable : 1;
        u8 oam_int_enable : 1;
        u8 lyc_int_enable : 1;
    } PACKED m_stat;

    u8 m_dma;
    size_t m_dma_transfered;

    u8 m_line_oam[10];
    size_t m_line_oam_count;

    u8 m_scy;
    u8 m_scx;

    u8 m_wy;
    u8 m_wx;

    u8 m_ly;
    u8 m_lyc;

    u32 m_dmg_colors[4];

    u32 m_screen_texture[SCREEN_WIDTH*SCREEN_HEIGHT];
    u32 m_bg_texture[32*32*8*8];
    u32 m_window_texture[32*32*8*8];
};

}
