#include <cstring>
#include "common/fs.hpp"
#include "common/logging.hpp"
#include "int_controller.hpp"
#include "memory.hpp"
#include "ppu.hpp"

static constexpr size_t TILE_WIDTH = 8;
static constexpr size_t TILE_HEIGHT = 8;
static constexpr size_t BG_TILES_X = 32;
static constexpr size_t BG_TILES_Y = 32;
static constexpr size_t BG_WIDTH = BG_TILES_X * TILE_WIDTH;
static constexpr size_t BG_HEIGHT = BG_TILES_Y * TILE_HEIGHT;

namespace gbemu::core
{

void Ppu::oamSearch(size_t screen_y)
{
    // TODO: timing: supposedly 2 machine cycles per sprite

    // TODO: is this necessary?
    if (!m_lcdc.obj_enable)
        return;

    size_t sprite_height = m_lcdc.obj_size == 0 ? 8 : 16;


    m_line_oam_count = 0;
    for (size_t i = 0; i < ARRAY_COUNT(m_oam) && m_line_oam_count < 10; i++)
    {
        // LOG("fetchSpriteLayerColor {} vs {}\n", screen_y, m_oam[i].y);
        if (m_oam[i].x != 0 && screen_y + 16 >= m_oam[i].y && screen_y + 16 < m_oam[i].y + sprite_height)
        {
            size_t j = 0;
            for (j = 0; j < m_line_oam_count; j++)
            {
                if (m_oam[i].x < m_oam[m_line_oam[j]].x)
                    break;
            }
            // push back
            if (j >= m_line_oam_count)
            {
                m_line_oam[m_line_oam_count++] = i;
            }
            // insert
            else
            {
                std::memmove(m_line_oam + j + 1, m_line_oam + j, m_line_oam_count - j);
                m_line_oam[j] = i;
                m_line_oam_count++;
            }
        }
    }
}

void Ppu::drawLine(size_t screen_y)
{
    auto fetchTileColor = [] (u8* tile_data, size_t tile_idx, size_t tile_x, size_t tile_y) ALWAYS_INLINE -> u8
    {
        u8 b0 = tile_data[tile_idx * 0x10 + tile_y*2 + 0];
        u8 b1 = tile_data[tile_idx * 0x10 + tile_y*2 + 1];
        u8 bit0 = (b0 >> (7 - tile_x)) & 1;
        u8 bit1 = (b1 >> (7 - tile_x)) & 1;
        u8 color_code =  (bit0 << 1) | bit1;

        return color_code;
    };

    auto fetchBgColorFromBgCoord = [this, fetchTileColor] (u8* tile_data, u8* tile_map, size_t bg_x, size_t bg_y) ALWAYS_INLINE -> u8
    {
        size_t tile_x = bg_x / TILE_WIDTH;
        size_t tile_y = bg_y / TILE_HEIGHT;

        size_t tile_off_x = bg_x % TILE_WIDTH;
        size_t tile_off_y = bg_y % TILE_HEIGHT;

        u8 tile_idx = tile_map[tile_y * BG_TILES_X + tile_x];
        if (!m_lcdc.bg_tile_area)
            tile_idx ^= 0x80;

        return fetchTileColor(tile_data, tile_idx, tile_off_x, tile_off_y);
    };

    auto fetchBgColor = [this, fetchBgColorFromBgCoord] (u8* tile_data, u8* tile_map, size_t screen_x, size_t screen_y) ALWAYS_INLINE -> u8
    {
        size_t bg_x = (screen_x + m_scx) % BG_WIDTH;
        size_t bg_y = (screen_y + m_scy) % BG_HEIGHT;

        return fetchBgColorFromBgCoord(tile_data, tile_map, bg_x, bg_y);
    };

    auto fetchWinColor = [this, fetchBgColorFromBgCoord] (u8* tile_data, u8* tile_map, size_t screen_x, size_t screen_y) ALWAYS_INLINE -> u8
    {
        size_t win_x = screen_x - m_wx + 7;
        size_t win_y = screen_y - m_wy;

        return fetchBgColorFromBgCoord(tile_data, tile_map, win_x, win_y);
    };

    auto fetchSpriteColor = [this, fetchTileColor] (OamEntry& oam, size_t sprite_x, size_t sprite_y, u8 bg_color) ALWAYS_INLINE -> std::tuple<u8, u8>
    {
        u8 tile_idx = oam.tile;
        if (m_lcdc.obj_size == 1)
            tile_idx &= ~1;
        u8 sprite_color = fetchTileColor(spriteTiles(), tile_idx, sprite_x, sprite_y);
        u8 palette = oam.dmg_palette;

        if (sprite_color == 0)
            return {0, 0};
        if (oam.bg_and_window_over_obj == 1 && bg_color != 0)
            return {0, 0};
        return {sprite_color, palette};
    };

    auto fetchSpriteLayerColor = [this, fetchSpriteColor] (u8* sprites, size_t sprite_count, size_t screen_x, size_t screen_y, u8 bg_color) ALWAYS_INLINE -> std::tuple<u8, u8>
    {
        for (size_t i = 0; i < sprite_count; i++)
        {
            auto& oam = m_oam[sprites[i]];

            if (screen_x + 8 >= oam.x && screen_x < oam.x)
            {
                size_t sprite_x = screen_x + 8 - oam.x;
                size_t sprite_y = screen_y + 16 - oam.y;
                size_t sprite_height = m_lcdc.obj_size == 0 ? 8 : 16;

                if (oam.flip_x)
                    sprite_x = 7 - sprite_x;
                if (oam.flip_y)
                    sprite_y = sprite_height - 1 - sprite_y;

                auto[color, palette] = fetchSpriteColor(oam, sprite_x, sprite_y, bg_color);
                if (color != 0)
                    return {color, palette};
            }
        }
        return {0, 0};
    };

    u32* dst = m_screen_texture + SCREEN_WIDTH * screen_y;
    u8* bg_map = bgMap();
    u8* win_map = windowMap();
    u8* tiles = bgTiles();

    for (size_t screen_x = 0; screen_x < SCREEN_WIDTH; screen_x++)
    {
        bool debug_win = false;

        u8 bg_color = 0;
        if (m_lcdc.bg_and_window_enable)
        {
            bg_color = (m_lcdc.window_enable && screen_x+7 >= m_wx && screen_y >= m_wy)
                ? fetchWinColor(tiles, win_map, screen_x, screen_y)
                : fetchBgColor(tiles, bg_map, screen_x, screen_y);

            // if (m_lcdc.window_enable)
            // if (m_lcdc.window_enable && screen_x+7 >= m_wx && screen_y >= m_wy)
            //     debug_win = true;
        }

        auto [sprite_color, sprite_palette] = m_lcdc.obj_enable
            ? fetchSpriteLayerColor(m_line_oam, m_line_oam_count, screen_x, screen_y, bg_color)
            : std::tuple<u8, u8>{0, 0};

        *dst++ = sprite_color == 0
            ? getColor(m_dmg_bgp, bg_color, false)
            : getColor(m_dmg_obp[sprite_palette], sprite_color, false);
        // if (debug_win)
        //     dst[-1] = 0xFFFF0000;
    }
}

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

            dst[y*32*8+x] = getColor(m_dmg_bgp, (bit0 << 1) | bit1, false);
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

    m_dma_transfered = 160;
}

void Ppu::mapMemory(Memory* mem)
{
    mem->mapRW(OAM_START, oam(), OAM_SIZE);

    mem->mapRW(BGP_ADDR, &m_dmg_bgp, 1);
    mem->mapRW(OBP0_ADDR, &m_dmg_obp[0]);
    mem->mapRW(OBP1_ADDR, &m_dmg_obp[1]);
    mem->mapRW(SCX_ADDR, &m_scx);
    mem->mapRW(SCY_ADDR, &m_scy);
    mem->mapRW(LCDC_ADDR, &m_lcdc);
    mem->mapRW(STAT_ADDR, &m_stat, 1, 0b01111100);
    mem->mapRO(LY_ADDR, &m_ly);
    mem->mapRW(LYC_ADDR, &m_lyc);
    mem->mapRW(WX_ADDR, &m_wx);
    mem->mapRW(WY_ADDR, &m_wy);

    mem->mapRW(DMA_ADDR, MmioRead::readFunc(&m_dma), std::bind(&Ppu::startDMA, this, std::placeholders::_1, std::placeholders::_2));

    switchBank(mem, 0);
}

Result<void> Ppu::startDMA(u16 off, u8 addr)
{
    m_dma = addr;
    m_dma_transfered = 0;
    return {};
}

void Ppu::switchBank(Memory* mem, size_t bank)
{
    m_vram_bank = bank;
    mem->mapRW(VRAM_START, vram(), VRAM_SIZE);
}

u32 Ppu::getColor(u8 palette, u8 idx, bool transparency)
{
    u8 code = (palette >> idx*2) & 3;

    if (transparency && code == 0)
        return 0x00000000;

    return m_dmg_colors[code];
}

void Ppu::step(Memory* mem, size_t clocks)
{
    // todo: cleanup
    static size_t last_clocks = 0;
    static size_t clocks_diff = clocks - last_clocks;

    if (m_dma_transfered < 160)
    {
        u8* dst = reinterpret_cast<u8*>(m_oam) + m_dma_transfered;
        u16 src_addr = (m_dma << 8) + m_dma_transfered;

        for (size_t i = 0; i < clocks_diff / 4 && m_dma_transfered < 160; i++)
        {
            *dst++ = mem->read8(src_addr++).value_or(0);
            m_dma_transfered++;
        }
    }

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

            drawLine(m_ly);
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

        oamSearch(m_ly);
    }

    if (prev_mode == PpuMode_HBlank && m_stat.mode == PpuMode_VBlank)
    {
        m_interrupt->requestInterrupt(InterruptType_Vblank);
        // drawTiles(true);
        // drawTiles(false);
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
