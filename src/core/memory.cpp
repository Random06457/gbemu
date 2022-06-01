#include "memory.hpp"
#include <algorithm>

namespace gbemu::core
{

bool Memory::Mapper::isRegionMapped(u16 addr, u16 size)
{
    // check if the region intersects with a buffer
    return std::ranges::any_of(m_entries, [&addr, &size](auto& x)
                               { return x.intersect(addr, size); }) ||
           // check if the region intersects with a register
           std::ranges::any_of(
               m_fast_entries, [&addr, &size](auto& x)
               { return x.first >= addr && x.first < addr + size; });
}

Result<Mmio*> Memory::Mapper::findEntry(u16 addr)
{
    if (m_fast_entries.contains(addr))
        return &m_fast_entries[addr];

    auto it = std::ranges::find_if(m_entries, [&addr](auto& buf)
                                   { return buf.matches(addr); });
    if (it != m_entries.end())
        return &*it;

    return tl::make_unexpected(MemoryError_CannotFindMapped);
}

Result<void> Memory::Mapper::map(const Mmio& entry)
{
    ERROR_IF(isRegionMapped(entry.start(), entry.size()),
             MemoryError_MapReservedRegion);

    if (entry.size() == 1)
    {
        m_fast_entries[entry.start()] = entry;
        return {};
    }

    // find position
    auto it = m_entries.begin();
    while (it != m_entries.end() && it->start() < entry.start())
        ++it;

    m_entries.insert(it, entry);

    return {};
}

Result<void> Memory::Mapper::unmap(u16 addr)
{
    if (m_fast_entries.contains(addr))
    {
        m_fast_entries.erase(addr);
        return {};
    }

    auto it = std::ranges::find_if(m_entries, [&addr](auto& buf)
                                   { return buf.start() == addr; });
    if (it != m_entries.end())
    {
        m_entries.erase(it);
        return {};
    }

    return tl::make_unexpected(MemoryError_UnmapUnmappedAddress);
}

Result<void> Memory::Mapper::remap(const Mmio& entry)
{
    auto old_entry = findEntry(entry.start());

    ERROR_IF(old_entry && old_entry.value()->start() != entry.start(),
             MemoryError_RemapWithDifferentAddr);

    ERROR_IF(old_entry && old_entry.value()->size() != entry.size(),
             MemoryError_RemapWithDifferentSize);

    if (old_entry)
    {
        Result<void> ret = unmap(old_entry.value()->start());
        if (!ret)
            return ret;
    }

    return map(entry);
}

Result<u8> Memory::read8(u16 addr)
{
    auto entry = m_read_map.findEntry(addr);
    if (entry)
        return entry.value()->m_read(addr - entry.value()->start());

    return tl::make_unexpected(MemoryError_ReadUnmappedMemory);
}

Result<void> Memory::write8(u16 addr, u8 data)
{
    auto entry = m_write_map.findEntry(addr);
    if (entry)
        return entry.value()->m_write(addr - entry.value()->start(), data);

    return tl::make_unexpected(MemoryError_WriteUnmappedMemory);
}

}
