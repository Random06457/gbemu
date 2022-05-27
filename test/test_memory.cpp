#include <gtest/gtest.h>
#include "core/memory.hpp"

using namespace gbemu::core;


TEST(memory, intersection)
{
    u8 buff[]
    {
        1, 2, 3, 4,
    };
    u8 reg = 8;
    Memory mem;
    ASSERT_TRUE(mem.mapMemory(Mmio::RW(0x100, buff, sizeof(buff))));
    ASSERT_TRUE(mem.mapMemory(Mmio::RW(0x200, &reg, 1)));

    ASSERT_FALSE(mem.isRegionMapped(0x0, 0x100));
    ASSERT_FALSE(mem.isRegionMapped(0x104, 0x200 - 0x104));
    ASSERT_FALSE(mem.isRegionMapped(0x201, 0x10000 - 0x201));

    ASSERT_TRUE(mem.isRegionMapped(0x100, 2));
    ASSERT_TRUE(mem.isRegionMapped(0x102, 4));
    ASSERT_TRUE(mem.isRegionMapped(0x101, 5));
    ASSERT_TRUE(mem.isRegionMapped(0xFF, 2));
    ASSERT_TRUE(mem.isRegionMapped(0xFF, 10));

    ASSERT_TRUE(mem.isRegionMapped(0x200, 1));
    ASSERT_TRUE(mem.isRegionMapped(0x200, 2));
    ASSERT_TRUE(mem.isRegionMapped(0x1FF, 2));
    ASSERT_TRUE(mem.isRegionMapped(0x1FF, 3));

    ASSERT_FALSE(mem.mapMemory(Mmio::RW(0x102, buff, sizeof(buff))));
    ASSERT_FALSE(mem.mapMemory(Mmio::RW(0x102, &reg, 1)));

    ASSERT_FALSE(mem.mapMemory(Mmio::RW(0x1FF, buff, sizeof(buff))));
    ASSERT_FALSE(mem.mapMemory(Mmio::RW(0x200, &reg, 1)));
}

TEST(memory, map_buffer)
{
    u8 buff[]
    {
        1, 2, 3, 4,
    };
    Memory mem;
    ASSERT_TRUE(mem.mapMemory(Mmio::RW(0x100, buff, sizeof(buff))));
    auto ret0 = mem.read8(0x100);
    auto ret1 = mem.read8(0x101);
    auto ret2 = mem.read8(0x102);
    auto ret3 = mem.read8(0x103);
    auto ret4 = mem.read8(0x104);
    ASSERT_TRUE(ret0);
    ASSERT_TRUE(ret1);
    ASSERT_TRUE(ret2);
    ASSERT_TRUE(ret3);
    ASSERT_FALSE(ret4);
    ASSERT_EQ(ret0.value(), 1);
    ASSERT_EQ(ret1.value(), 2);
    ASSERT_EQ(ret2.value(), 3);
    ASSERT_EQ(ret3.value(), 4);
}

TEST(memory, map_register)
{
    u8 reg0 = 8;
    u8 reg1 = 7;
    u8 reg2 = 0x80;
    Memory mem;

    // RO
    ASSERT_TRUE(mem.mapMemory(Mmio::RO(0x100, &reg0, 1)));
    ASSERT_FALSE(mem.write8(0x100, 1));
    ASSERT_TRUE(mem.read8(0x100));
    ASSERT_EQ(mem.read8(0x100).value(), 8);

    // R/W
    ASSERT_TRUE(mem.mapMemory(Mmio::RW(0x101, &reg1, 1)));
    ASSERT_TRUE(mem.read8(0x101));
    ASSERT_EQ(mem.read8(0x101), 7);
    ASSERT_TRUE(mem.write8(0x101, 9));
    ASSERT_EQ(mem.read8(0x101), 9);

    // WO
    ASSERT_TRUE(mem.mapMemory(Mmio::WO(0x102, &reg1, 1)));
    ASSERT_FALSE(mem.read8(0x102));
    ASSERT_TRUE(mem.write8(0x102, 9));
    ASSERT_EQ(reg1, 9);

    // partial W
    ASSERT_TRUE(mem.mapMemory(Mmio::WO(0x103, &reg2, 1, 0b01111110)));
    ASSERT_FALSE(mem.read8(0x103));
    ASSERT_TRUE(mem.write8(0x103, 0x7F));
    ASSERT_EQ(reg2, 0xFE);
}

TEST(memory, unmap_register)
{
    Memory mem;
    u8 reg;

    ASSERT_FALSE(mem.unmapMemory(0x100));

    ASSERT_TRUE(mem.mapMemory(Mmio::RW(0x100, &reg, 1)));
    ASSERT_TRUE(mem.isRegionMapped(0x100, 1));
    ASSERT_FALSE(mem.mapMemory(Mmio::RW(0x100, &reg, 1)));

    ASSERT_TRUE(mem.unmapMemory(0x100));
    ASSERT_FALSE(mem.isRegionMapped(0x100, 1));
    ASSERT_TRUE(mem.mapMemory(Mmio::RW(0x100, &reg, 1)));
}

TEST(memory, unmap_buffer)
{
    Memory mem;
    u8 reg;

    ASSERT_FALSE(mem.unmapMemory(0x100));

    ASSERT_TRUE(mem.mapMemory(Mmio::RW(0x100, &reg, 1)));
    ASSERT_TRUE(mem.isRegionMapped(0x100, 1));
    ASSERT_FALSE(mem.mapMemory(Mmio::RW(0x100, &reg, 1)));

    ASSERT_TRUE(mem.unmapMemory(0x100));
    ASSERT_FALSE(mem.isRegionMapped(0x100, 1));
    ASSERT_TRUE(mem.mapMemory(Mmio::RW(0x100, &reg, 1)));
}

TEST(memory, get_mapped)
{
    Memory mem;
    u8 buff0[] = { 1, 2, 3, 4, };
    u8 reg;

    ASSERT_TRUE(mem.mapMemory(Mmio::RW(0x100, buff0, sizeof(buff0))));
    ASSERT_TRUE(mem.mapMemory(Mmio::RW(0x200, &reg, 1)));
    ASSERT_TRUE(mem.getEntry(0x100));
    ASSERT_TRUE(mem.getEntry(0x200));
    ASSERT_FALSE(mem.getEntry(0x300));
}

TEST(memory, remap_buffer)
{
    Memory mem;
    u8 buff0[] = { 1, 2, 3, 4, };
    u8 buff1[] = { 5, 6, 7, 8, };

    ASSERT_TRUE(mem.mapMemory(Mmio::RW(0x100, buff0, sizeof(buff0))));

    ASSERT_TRUE(mem.read8(0x100));
    ASSERT_EQ(mem.read8(0x100).value(), 1);

    auto buff = mem.getEntry(0x100);
    ASSERT_TRUE(buff);
    buff.value()->m_read_func = Mmio::readFunc(buff1);

    ASSERT_TRUE(mem.read8(0x100));
    ASSERT_EQ(mem.read8(0x100).value(), 5);
}