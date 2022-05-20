#pragma once

#include "types.hpp"
#include "attributes.hpp"

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