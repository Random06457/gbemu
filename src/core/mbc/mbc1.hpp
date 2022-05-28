#include "core/cart.hpp"

namespace gbemu::core
{

class Mbc1 : public Mbc
{
public:
    Mbc1(std::vector<u8>& rom);

    virtual void map(Memory* mem) override;


    Result<void> mbc1WriteRamEnable(Memory* mem, u16 off, u8 data);
    Result<void> mbc1WriteRomBankNumber(Memory* mem, u16 off, u8 data);
    Result<void> mbc1WriteRamBankNumber(Memory* mem, u16 off, u8 data);
    Result<void> mbc1WriteSelectMode(Memory* mem, u16 off, u8 data);

    void mbc1RemapBank1(Memory* mem);
    void mbc1RemapRAM(Memory* mem);

private:
    std::vector<u8> m_extram;

    union
    {
        u8 m_mbc1_rom_bank;
        struct
        {
            u8 m_mbc1_rom_bank_lo : 5;
            u8 m_mbc1_rom_bank_hi : 2;
        };
    } PACKED;
    u8 m_mbc1_ram_bank;
    u8 m_mbc1_mode; // 0=rom 1=ram
    bool m_mbc1_ram_enabled;
};

}
