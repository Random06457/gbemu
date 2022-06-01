#include "core/cart.hpp"

namespace gbemu::core
{

class Mbc3 : public Mbc
{
public:
    Mbc3(std::vector<u8>& rom);

    virtual void map(Memory* mem) override;

    Result<void> writeRamTimerEnable(Memory* mem, u16 off, u8 data);
    Result<void> writeRomBank(Memory* mem, u16 off, u8 data);
    Result<void> writeRamRtcBank(Memory* mem, u16 off, u8 data);
    Result<void> writeLatchData(Memory* mem, u16 off, u8 data);
    Result<void> writeRtc(Memory* mem, u16 off, u8 data);
    Result<u8> readRtc(Memory* mem, u16 off);

    void remapRomBank1(Memory* mem);
    void remapRamRtc(Memory* mem);

private:
    std::vector<u8> m_extram;

    bool m_ram_and_timer_enabled;
    u8 m_rom_bank;
    u8 m_ram_rtc_bank;
    bool m_rtc_latched;
    u8 m_latch_data;
    u8 m_rts_s;
    u8 m_rts_m;
    u8 m_rts_h;
    u8 m_rts_dl;
    u8 m_rts_dh;
};

}
