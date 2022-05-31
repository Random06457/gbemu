#pragma once

#include "attributes.hpp"
#include "types.hpp"

namespace gbemu::core
{

class Memory;
class Device
{
public:
    virtual ~Device() {}
    virtual void mapMemory(Memory* mem) = 0;
};

}