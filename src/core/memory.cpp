#include "memory.hpp"
#include <algorithm>

namespace gbemu::core
{


Result<void> Memory::mapMemory(Mmio entry)
{
    ERROR_IF(isRegionMapped(entry.start(), entry.size()), MemoryError_MapReservedRegion);

    if (entry.size() == 1)
    {
        m_fast_entries[entry.start()] = entry;
        return {};
    }

    // find position
    auto it = m_entries.begin();
    while(it != m_entries.end() && it->start() < entry.start())
        ++it;

    m_entries.insert(it, entry);

    return {};
}

Result<void> Memory::remapMemory(Mmio entry)
{
    auto old_entry = getEntry(entry.start());

    ERROR_IF(!old_entry && isRegionMapped(entry.start(), entry.size()), MemoryError_RemapWithDifferentAddr);

    ERROR_IF(old_entry && old_entry.value()->size() != entry.size(), MemoryError_RemapWithDifferentSize);

    if (old_entry)
    {
        Result<void> ret = unmapMemory(old_entry.value()->start());
        if (!ret)
            return ret;
    }

    return mapMemory(entry);
}

Result<void> Memory::unmapMemory(u16 addr)
{
    if (m_fast_entries.contains(addr))
    {
        m_fast_entries.erase(addr);
        return {};
    }

    auto it = std::ranges::find_if(m_entries, [&addr](auto& buf) { return buf.start() == addr; });
    if (it != m_entries.end())
    {
        m_entries.erase(it);
        return {};
    }

    return tl::make_unexpected(MemoryError_UnmapUnmappedAddress);
}


bool Memory::isRegionMapped(u16 addr, size_t size)
{
    // check if the region intersects with a buffer
    return std::ranges::any_of(m_entries,
        [&addr, &size](auto& x) {
            return x.intersect(addr, size);
        }) ||
        // check if the region intersects with a register
        std::ranges::any_of(m_fast_entries,
        [&addr, &size](auto& x) {
            return x.first >= addr && x.first < addr + size;
        });
}


Result<Mmio*> Memory::getEntry(u16 addr)
{
    if (m_fast_entries.contains(addr))
        return &m_fast_entries[addr];

    auto it = std::ranges::find_if(m_entries, [&addr](auto& buf) { return buf.start() == addr; });
    if (it != m_entries.end())
        return &*it;

    return tl::make_unexpected(MemoryError_CannotFindMapped);
}

Result<u8> Memory::read8(u16 addr)
{
    if (m_fast_entries.contains(addr))
        return m_fast_entries[addr].m_read_func(0);

    for (auto& entry : m_entries)
        if (entry.matches(addr))
            return entry.m_read_func(addr - entry.start());


    return tl::make_unexpected(MemoryError_ReadUnmappedMemory);
}
Result<void> Memory::write8(u16 addr, u8 data)
{
    if (m_fast_entries.contains(addr))
        return m_fast_entries[addr].m_write_func(0, data);

    for (auto& entry : m_entries)
        if (entry.matches(addr))
            return entry.m_write_func(addr - entry.start(), data);

    return tl::make_unexpected(MemoryError_WriteUnmappedMemory);
}

}
