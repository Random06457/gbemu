#pragma once

#include "types.hpp"

namespace gbemu::core
{

class Disas
{
public:

    Disas(const void* data, size_t size) :
        m_mem(reinterpret_cast<const u8*>(data)),
        m_size(size),
        m_idx(0)
    {}

    std::string disassemble();

    static std::string disassemble(const void* data, size_t size);

    // todo: move
    static size_t opcodeSize(u8 op);
    static size_t isValidOpcode(u8 op);

private:
    u8 read8();
    u16 read16();

private:
    const u8* m_mem;
    size_t m_size;
    size_t m_idx;
};

}