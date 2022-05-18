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

using MmioReadFunc = std::function<Result<u8>()>;
using MmioWriteFunc = std::function<Result<void>(u8)>;

struct MmioReg
{
    static MmioReg ro(MmioReadFunc read) { return { read, invalidWrite }; }
    static MmioReg wo(MmioWriteFunc write) { return { invalidRead, write }; }
    static MmioReg rw(MmioReadFunc read, MmioWriteFunc write) { return { read, write }; }

    static MmioReg ro(void* reg)
    {
        return { std::bind(ptrRead, reinterpret_cast<u8*>(reg)), invalidWrite };
    }
    static MmioReg wo(void* reg, u8 wmask = 0xFF)
    {
        return {
            invalidRead,
            std::bind(ptrWrite, reinterpret_cast<u8*>(reg), wmask, std::placeholders::_1)
        };
    }
    static MmioReg rw(void* reg, u8 wmask = 0xFF)
    {
        return {
            std::bind(ptrRead, reinterpret_cast<u8*>(reg)),
            std::bind(ptrWrite, reinterpret_cast<u8*>(reg), wmask, std::placeholders::_1)
        };
    }

    MmioReg() {}
    MmioReg(MmioReadFunc read, MmioWriteFunc write) :
        read(read),
        write(write)
    { }

    MmioReadFunc read;
    MmioWriteFunc write;

private:
    static inline Result<u8> ptrRead(u8* ptr) { return *ptr; }
    static inline Result<void> ptrWrite(u8* ptr, u8 mask, u8 value) { *ptr = (*ptr & ~mask) | (value & mask); return {}; }
    static inline Result<u8> invalidRead() { return tl::make_unexpected(MemoryError_ReadToWriteOnlyAddress); }
    static inline Result<void> invalidWrite(u8 value) { return tl::make_unexpected(MemoryError_WriteToReadOnlyAddress); }
};

class MmioBuffer
{
public:
    MmioBuffer(u16 addr, void* buff, size_t size) :
        m_address(addr),
        m_buffer(reinterpret_cast<u8*>(buff)),
        m_size(size)
    {}

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
    u16 size() const { return m_size; }
    void setBuffer(void* buff) { m_buffer = reinterpret_cast<u8*>(buff); };

    Result<void> read(u16 addr, void* buff, size_t size);
    Result<void> write(u16 addr, void* buff, size_t size);

    // fast code path
    Result<u8> read8(u16 addr)
    {
        ERROR_IF(!matches(addr), MemoryError_BufferOOBRead);
        return m_buffer[addr - m_address];
    }
    Result<void> write8(u16 addr, u8 data)
    {
        ERROR_IF(!matches(addr), MemoryError_BufferOOBWrite);
        m_buffer[addr - m_address] = data;
        return {};
    }

private:
    u16 m_address;
    u8* m_buffer;
    size_t m_size;
};

class Memory
{
public:
    bool isRegionMapped(u16 addr, size_t size);
    Result<void> mapBuffer(u16 addr, void* buff, size_t size);
    Result<void> mapRegister(u16 addr, MmioReg reg);
    Result<void> unmapAddress(u16 addr);
    Result<MmioBuffer*> getMappedBuffer(u16 addr);
    Result<MmioReg*> getMappedReg(u16 addr);
    Result<void> remapBuffer(u16 addr, void* buff);
    Result<void> remapBuffer(u16 addr, void* buff, size_t size);

    // Result<void> read(u16 addr, void* dst, size_t size);
    // Result<void> write(u16 addr, const void* src, size_t size);

    // fast code path
    Result<u8> read8(u16 addr);
    Result<void> write8(u16 addr, u8 data);

private:

    std::vector<MmioBuffer> m_buffers;
    std::unordered_map<u16, MmioReg> m_registers;
};

}
