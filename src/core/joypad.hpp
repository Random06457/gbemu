#pragma once

#include "device.hpp"

namespace gbemu::core
{

class Joypad : public Device
{
public:
    Joypad();

    void mapMemory(Memory* mem) override;
private:
    u8 m_p1;
};

}
