#pragma once

#include <span>
#include <unordered_map>
#include <functional>
#include <cstring>
#include "types.hpp"
#include "result.hpp"
#include "macro.hpp"

namespace gbemu::core
{

using MmioReadFunc = std::function<Result<u8>(u16)>;
using MmioWriteFunc = std::function<Result<void>(u16, u8)>;

class Mmio
{
public:
    Mmio(u16 addr, size_t size, MmioReadFunc read, MmioWriteFunc write) :
        m_address(addr),
        m_size(size),
        m_read_func(read),
        m_write_func(write)
    {}

    Mmio(){}

private:
    static inline Result<u8> readBuffImpl(const void* buff, u8 mask, u16 off)
    {
        return reinterpret_cast<const u8*>(buff)[off] & mask;
    }

    static inline Result<void> writeBuffImpl(void* buff, u8 mask, u16 off, u8 value)
    {
        u8* ptr = reinterpret_cast<u8*>(buff) + off;
        *ptr = (*ptr & ~mask) | (value & mask);
        return {};
    }

public:
    static inline Result<u8> invalidRead(u16 off) { return tl::make_unexpected(MemoryError_ReadToWriteOnlyAddress); }
    static inline Result<void> invalidWrite(u16 off, u8 value) { return tl::make_unexpected(MemoryError_WriteToReadOnlyAddress); }

    static inline MmioReadFunc readFunc(const void* buff, u8 mask = 0xFF)
    {
        return std::bind(readBuffImpl, buff, mask, std::placeholders::_1);
    }
    static inline MmioWriteFunc writeFunc(void* buff, u8 mask = 0xFF)
    {
        return std::bind(writeBuffImpl, buff, mask, std::placeholders::_1, std::placeholders::_2);
    }

    static inline Mmio RO(u16 addr, size_t size, MmioReadFunc read) { return { addr, size, read, invalidWrite }; }
    static inline Mmio WO(u16 addr, size_t size, MmioWriteFunc write) { return { addr, size, invalidRead, write }; }
    static inline Mmio RW(u16 addr, void* buff, size_t size, u8 wmask = 0xFF, u8 rmask = 0xFF)
    { return { addr, size, readFunc(buff, rmask), writeFunc(buff, wmask) }; }
    static inline Mmio RO(u16 addr, void* buff, size_t size, u8 mask = 0xFF) { return { addr, size, readFunc(buff, mask), invalidWrite }; }
    static inline Mmio WO(u16 addr, void* buff, size_t size, u8 mask = 0xFF) { return { addr, size, invalidRead, writeFunc(buff, mask) }; }


    bool intersect(u16 addr, size_t size)
    {
        return !(addr + size <= m_address || addr >= m_address + m_size);
    }

    bool matches(u16 addr, size_t size=1)
    {
        return addr >= m_address && addr + size <= m_address + m_size;
    }

    u16 start() const { return m_address; }
    u16 end() const { return m_address + m_size; }
    size_t size() const { return m_size; }

public:
    u16 m_address;
    size_t m_size;
    MmioReadFunc m_read_func;
    MmioWriteFunc m_write_func;
};

class Memory
{
public:
    Result<void> mapMemory(Mmio entry);
    Result<void> mapMemory(u16 addr, size_t size, MmioReadFunc read, MmioWriteFunc write) { return mapMemory({addr, size, read, write}); }
    Result<void> remapMemory(Mmio entry);
    Result<void> unmapMemory(u16 addr);
    Result<Mmio*> getEntry(u16 addr);

    bool isRegionMapped(u16 addr, size_t size);

    // TODO:
    // Result<void> read(u16 addr, void* dst, size_t size);
    // Result<void> write(u16 addr, const void* src, size_t size);

    // fast code path
    Result<u8> read8(u16 addr);
    Result<void> write8(u16 addr, u8 data);

private:
    std::vector<Mmio> m_entries;
    std::unordered_map<u16, Mmio> m_fast_entries;
};

}
