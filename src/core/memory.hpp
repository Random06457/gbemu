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

    static MmioReg ro(u8* reg) { return { std::bind(ptrRead, reg), invalidWrite }; }
    static MmioReg wo(u8* reg) { return { invalidRead, std::bind(ptrWrite, reg, std::placeholders::_1) }; }
    static MmioReg rw(u8* reg) { return { std::bind(ptrRead, reg), std::bind(ptrWrite, reg, std::placeholders::_1) }; }

    MmioReg() {}
    MmioReg(MmioReadFunc read, MmioWriteFunc write) :
        read(read),
        write(write)
    { }

    MmioReadFunc read;
    MmioWriteFunc write;

private:
    static inline Result<u8> ptrRead(u8* ptr) { return *ptr; }
    static inline Result<void> ptrWrite(u8* ptr, u8 value) { *ptr = value; return Result<void>(); }
    static inline Result<u8> invalidRead() { return tl::make_unexpected(Error_MemoryReadToWriteOnlyAddress); }
    static inline Result<void> invalidWrite(u8 value) { return tl::make_unexpected(Error_MemoryWriteToReadOnlyAddress); }
};

class Memory
{
private:

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

        u16 start() { return m_address; }
        u16 end() { return m_address + m_size; }
        u16 size() { return m_size; }

        Result<void> read(u16 addr, void* buff, size_t size);
        Result<void> write(u16 addr, void* buff, size_t size);

        // fast code path
        Result<u8> read8(u16 addr)
        {
            ERROR_IF(!matches(addr), Error_MemoryBufferOOBRead);
            return m_buffer[addr - m_address];
        }
        Result<void> write8(u16 addr, u8 data)
        {
            ERROR_IF(!matches(addr), Error_MemoryBufferOOBWrite);
            m_buffer[addr - m_address] = data;
            return Result<void>();
        }

    private:
        u16 m_address;
        u8* m_buffer;
        size_t m_size;
    };

public:
    bool isRegionMapped(u16 addr, size_t size);
    Result<void> mapBuffer(u16 addr, void* buff, size_t size);
    Result<void> mapRegister(u16 addr, MmioReg reg);

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
