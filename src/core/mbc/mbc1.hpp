#include "core/cart.hpp"

namespace gbemu::core
{

class Mbc1 : public Mbc
{
public:
    Mbc1(std::vector<u8>& rom);

    virtual void map(Memory* mem) override;

    Result<void> writeRamEnable(Memory* mem, u16 off, u8 data);
    Result<void> writeRomBankNumber(Memory* mem, u16 off, u8 data);
    Result<void> writeRamBankNumber(Memory* mem, u16 off, u8 data);
    Result<void> writeSelectMode(Memory* mem, u16 off, u8 data);

    void remapBank1(Memory* mem);
    void remapRAM(Memory* mem);

private:
    std::vector<u8> m_extram;

    union
    {
        u8 m_rom_bank;
        struct
        {
            u8 m_rom_bank_lo : 5;
            u8 m_rom_bank_hi : 2;
        };
    } PACKED;
    u8 m_ram_bank;
    u8 m_mode; // 0=rom 1=ram
    bool m_ram_enabled;
};

}
