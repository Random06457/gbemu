#pragma once

#include "tl/expected.hpp"

namespace gbemu::core
{
enum Error
{
    Error_InvalidRom,

    // Memory
    Error_MemoryWriteToReadOnlyAddress,
    Error_MemoryReadToWriteOnlyAddress,
    Error_MemoryBufferOOBRead,
    Error_MemoryBufferOOBWrite,
    Error_MemoryMapReservedRegion,
    Error_MemoryReadUnmappedMemory,
    Error_MemoryWriteUnmappedMemory,
};

template<typename T>
using Result = tl::expected<T, Error>;

}
