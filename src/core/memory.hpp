#pragma once

#include <unordered_map>
#include "macro.hpp"
#include "types.hpp"
#include "result.hpp"

namespace gbemu::core
{

using MmioReadFunc = std::function<Result<u8>(u16)>;
using MmioWriteFunc = std::function<Result<void>(u16, u8)>;

struct Mmio
{
public:
    Mmio(u16 addr, size_t size, MmioReadFunc read) :
        m_address(addr),
        m_size(size),
        m_is_read(true),
        m_read(read),
        m_write(nullptr)
    {
    }

    Mmio(u16 addr, size_t size, MmioWriteFunc write) :
        m_address(addr),
        m_size(size),
        m_is_read(false),
        m_read(nullptr),
        m_write(write)
    {
    }

    Mmio() {}

    constexpr bool intersect(u16 addr, size_t size)
    {
        return !(addr + size <= m_address || addr >= m_address + m_size);
    }
    constexpr bool matches(u16 addr, size_t size = 1)
    {
        return addr >= m_address && addr + size <= m_address + m_size;
    }

    constexpr u16 start() const { return m_address; }
    constexpr u16 end() const { return m_address + m_size; }
    constexpr u16 size() const { return m_size; }
    constexpr bool isRead() const { return m_is_read; }
    constexpr bool isWrite() const { return !m_is_read; }

    u16 m_address;
    u16 m_size;
    bool m_is_read;
    // don't make a union to make it a POD type
    MmioReadFunc m_read;
    MmioWriteFunc m_write;
};

// Wrappers around Mmio
struct MmioRead : Mmio
{
private:
    static inline Result<u8> readBuffImpl(const void* buff, u8 mask, u16 off)
    {
        return reinterpret_cast<const u8*>(buff)[off] & mask;
    }

public:
    static inline Result<u8> invalidRead(u16 off)
    {
        return tl::make_unexpected(MemoryError_ReadToWriteOnlyAddress);
    }
    static inline MmioReadFunc readFunc(const void* buff, u8 mask = 0xFF)
    {
        return std::bind(readBuffImpl, buff, mask, std::placeholders::_1);
    }

    MmioRead(u16 addr, size_t size, MmioReadFunc read) : Mmio(addr, size, read)
    {
    }
    MmioRead(u16 addr, size_t size, const void* buff, u8 mask = 0xFF) :
        Mmio(addr, size, readFunc(buff, mask))
    {
    }
};

struct MmioWrite : Mmio
{
private:
    static inline Result<void> writeBuffImpl(void* buff, u8 mask, u16 off,
                                             u8 value)
    {
        u8* ptr = reinterpret_cast<u8*>(buff) + off;
        *ptr = (*ptr & ~mask) | (value & mask);
        return {};
    }

public:
    static inline Result<void> invalidWrite(u16 off, u8 value)
    {
        return tl::make_unexpected(MemoryError_WriteToReadOnlyAddress);
    }
    static inline MmioWriteFunc writeFunc(void* buff, u8 mask = 0xFF)
    {
        return std::bind(writeBuffImpl, buff, mask, std::placeholders::_1,
                         std::placeholders::_2);
    }

    MmioWrite(u16 addr, size_t size, MmioWriteFunc write) :
        Mmio(addr, size, write)
    {
    }
    MmioWrite(u16 addr, size_t size, void* buff, u8 mask = 0xFF) :
        Mmio(addr, size, writeFunc(buff, mask))
    {
    }
};

class Memory
{
private:
    struct Mapper
    {
        Result<void> map(const Mmio& entry);
        Result<void> unmap(u16 addr);
        Result<void> remap(const Mmio& entry);
        Result<Mmio*> findEntry(u16 addr);
        bool isRegionMapped(u16 addr, u16 size);

        std::vector<Mmio> m_entries;
        std::unordered_map<u16, Mmio> m_fast_entries;
    };

public:
    bool isRegionReadable(u16 addr, u16 size)
    {
        return m_read_map.isRegionMapped(addr, size);
    }
    bool isRegionWritable(u16 addr, u16 size)
    {
        return m_write_map.isRegionMapped(addr, size);
    }
    bool isRegionReadAndWritable(u16 addr, u16 size)
    {
        return isRegionReadable(addr, size) && isRegionWritable(addr, size);
    }
    bool isRegionReadOrWritable(u16 addr, u16 size)
    {
        return isRegionReadable(addr, size) || isRegionWritable(addr, size);
    }

    Result<void> mapRO(const MmioRead& read) { return m_read_map.map(read); }
    Result<void> mapWO(const MmioWrite& write)
    {
        return m_write_map.map(write);
    }
    Result<void> mapRW(const MmioRead& read, const MmioWrite& write)
    {
        auto res1 = mapRO(read);
        auto res2 = mapWO(write);
        return res1 ? res1 : res2;
    }

    Result<void> unmapRO(u16 addr) { return m_read_map.unmap(addr); }
    Result<void> unmapWO(u16 addr) { return m_write_map.unmap(addr); }
    Result<void> unmapRW(u16 addr)
    {
        auto res1 = unmapRO(addr);
        auto res2 = unmapWO(addr);
        return res1 ? res1 : res2;
    }

    Result<void> remapRO(const MmioRead& read)
    {
        return m_read_map.remap(read);
    }
    Result<void> remapWO(const MmioWrite& write)
    {
        return m_write_map.remap(write);
    }
    Result<void> remapRW(const MmioRead& read, const MmioWrite& write)
    {
        auto res1 = remapRO(read);
        auto res2 = remapWO(write);
        return res1 ? res1 : res2;
    }

    // Helper functions
    Result<void> mapRW(u16 addr, void* buff, u16 size = 1, u8 wmask = 0xFF,
                       u8 rmask = 0xFF)
    {
        return mapRW(MmioRead(addr, size, buff, rmask),
                     MmioWrite(addr, size, buff, wmask));
    }
    Result<void> mapRO(u16 addr, const void* buff, u16 size = 1, u8 mask = 0xFF)
    {
        return mapRO(MmioRead(addr, size, buff, mask));
    }
    Result<void> mapWO(u16 addr, void* buff, u16 size = 1, u8 mask = 0xFF)
    {
        return mapWO(MmioWrite(addr, size, buff, mask));
    }

    Result<void> remapRW(u16 addr, void* buff, u16 size = 1, u8 wmask = 0xFF,
                         u8 rmask = 0xFF)
    {
        return remapRW(MmioRead(addr, size, buff, rmask),
                       MmioWrite(addr, size, buff, wmask));
    }
    Result<void> remapRO(u16 addr, const void* buff, u16 size = 1,
                         u8 mask = 0xFF)
    {
        return remapRO(MmioRead(addr, size, buff, mask));
    }
    Result<void> remapWO(u16 addr, void* buff, u16 size = 1, u8 mask = 0xFF)
    {
        return remapWO(MmioWrite(addr, size, buff, mask));
    }

    Result<void> mapRW(u16 addr, MmioReadFunc read, MmioWriteFunc write,
                       u16 size = 1)
    {
        return mapRW(MmioRead(addr, size, read), MmioWrite(addr, size, write));
    }
    Result<void> remapRW(u16 addr, MmioReadFunc read, MmioWriteFunc write,
                         u16 size = 1)
    {
        return remapRW(MmioRead(addr, size, read),
                       MmioWrite(addr, size, write));
    }

    // TODO:
    // Result<void> read(u16 addr, void* dst, size_t size);
    // Result<void> write(u16 addr, const void* src, size_t size);

    // fast code path
    Result<u8> read8(u16 addr);
    Result<void> write8(u16 addr, u8 data);

private:
    Mapper m_read_map;
    Mapper m_write_map;
};

}
