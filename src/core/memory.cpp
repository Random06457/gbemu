#include "memory.hpp"
#include <algorithm>

namespace gbemu::core
{

Result<void> MmioBuffer::read(u16 addr, void* buff, size_t size)
{
    ERROR_IF(!matches(addr, size), MemoryError_BufferOOBRead);

    std::memcpy(buff, m_buffer + addr - m_address, size);

    return {};
}

Result<void> MmioBuffer::write(u16 addr, void* buff, size_t size)
{
    ERROR_IF(!matches(addr, size), MemoryError_BufferOOBWrite);

    std::memcpy(m_buffer + addr - m_address, buff, size);

    return {};
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
    ERROR_IF(isRegionMapped(addr, size), MemoryError_MapReservedRegion);

    // find position
    auto it = m_buffers.begin();
    while(it != m_buffers.end() && it->start() < addr)
        ++it;

    m_buffers.insert(it, {addr, buff, size});

    return {};
}

Result<void> Memory::mapRegister(u16 addr, MmioReg reg)
{
    ERROR_IF(isRegionMapped(addr, 1), MemoryError_MapReservedRegion);

    m_registers[addr] = reg;

    return {};
}

Result<MmioBuffer*> Memory::getMappedBuffer(u16 addr)
{
    auto it = std::ranges::find_if(m_buffers, [&addr](auto& buf) { return buf.start() == addr; });
    if (it != m_buffers.end())
        return &*it;

    return tl::make_unexpected(MemoryError_CannotFindMapped);
}
Result<MmioReg*> Memory::getMappedReg(u16 addr)
{
    if (m_registers.contains(addr))
        return &m_registers[addr];

    return tl::make_unexpected(MemoryError_CannotFindMapped);
}

Result<void> Memory::unmapAddress(u16 addr)
{
    auto it = std::ranges::find_if(m_buffers, [&addr](auto& buf) { return buf.start() == addr; });
    if (it != m_buffers.end())
    {
        m_buffers.erase(it);
        return {};
    }

    if (m_registers.contains(addr))
    {
        m_registers.erase(addr);
        return {};
    }

    return tl::make_unexpected(MemoryError_UnmapUnmappedAddress);
}

Result<void> Memory::remapBuffer(u16 addr, void* buff)
{
    auto b = PROPAGATE_ERROR(getMappedBuffer(addr));
    b->setBuffer(buff);
    return {};
}

Result<void> Memory::remapBuffer(u16 addr, void* buff, size_t size)
{
    auto b = getMappedBuffer(addr);
    if (!b)
        return mapBuffer(addr, buff, size);

    ERROR_IF(b.value()->start() != addr || b.value()->size() != size, MemoryError_RemapBufferWithDifferentSize);

    return remapBuffer(addr, buff);
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

    return tl::make_unexpected(MemoryError_ReadUnmappedMemory);
}
Result<void> Memory::write8(u16 addr, u8 data)
{
    for (auto& buff : m_buffers)
        if (buff.matches(addr))
            return buff.write8(addr, data);

    if (m_registers.contains(addr))
        return m_registers[addr].write(data);

    return tl::make_unexpected(MemoryError_WriteUnmappedMemory);
}

}
