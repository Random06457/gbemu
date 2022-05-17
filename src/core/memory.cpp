#include "memory.hpp"
#include <algorithm>

namespace gbemu::core
{

Result<void> Memory::MmioBuffer::read(u16 addr, void* buff, size_t size)
{
    ERROR_IF(!matches(addr, size), Error_MemoryBufferOOBRead);

    std::memcpy(buff, m_buffer + addr - m_address, size);

    return Result<void>();
}

Result<void> Memory::MmioBuffer::write(u16 addr, void* buff, size_t size)
{
    ERROR_IF(!matches(addr, size), Error_MemoryBufferOOBWrite);

    std::memcpy(m_buffer + addr - m_address, buff, size);

    return Result<void>();
}


bool Memory::isRegionMapped(u16 addr, size_t size)
{
    // check if the region intersects with a buffer
    return std::ranges::any_of(m_buffers,
        [&addr, &size](auto& x) {
            return x.intersect(addr, size);
        }) ||
        // check if the region intersects with a register
        std::ranges::any_of(m_registers,
        [&addr, &size](auto& x) {
            return x.first >= addr && x.first < addr + size;
        });
}

Result<void> Memory::mapBuffer(u16 addr, void* buff, size_t size)
{
    ERROR_IF(isRegionMapped(addr, size), Error_MemoryMapReservedRegion);

    // find position
    auto it = m_buffers.begin();
    while(it != m_buffers.end() && it->start() < addr)
        ++it;

    m_buffers.insert(it, {addr, buff, size});

    return Result<void>();
}

Result<void> Memory::mapRegister(u16 addr, MmioReg reg)
{
    ERROR_IF(isRegionMapped(addr, 1), Error_MemoryMapReservedRegion);

    m_registers[addr] = reg;

    return Result<void>();
}

// Result<void> Memory::read(u16 addr, void* dst, size_t size)
// {
//     u8* ptr = reinterpret_cast<u8*>(dst);
//     u16 off = 0;

//     for (auto& buff : m_buffers)
//     {
//         if (buff.matches(addr))
//         {
//             size_t available_size = buff.end() - addr;
//         }
//     }
// }
// Result<void> write(u16 addr, const void* src, size_t size);


Result<u8> Memory::read8(u16 addr)
{
    for (auto& buff : m_buffers)
        if (buff.matches(addr))
            return buff.read8(addr);

    if (m_registers.contains(addr))
        return m_registers[addr].read();

    return tl::make_unexpected(Error_MemoryReadUnmappedMemory);
}
Result<void> Memory::write8(u16 addr, u8 data)
{
    for (auto& buff : m_buffers)
        if (buff.matches(addr))
            return buff.write8(addr, data);

    if (m_registers.contains(addr))
        return m_registers[addr].write(data);

    return tl::make_unexpected(Error_MemoryWriteUnmappedMemory);
}

}
