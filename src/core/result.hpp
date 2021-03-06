#pragma once

#include "tl/expected.hpp"

namespace gbemu::core
{
enum Error
{
    // Gameboy
    Error_InvalidRom,
    Errro_InvalidBootromSize,

    // Memory
    MemoryError_WriteToReadOnlyAddress,
    MemoryError_ReadToWriteOnlyAddress,
    MemoryError_BufferOOBRead,
    MemoryError_BufferOOBWrite,
    MemoryError_MapReservedRegion,
    MemoryError_ReadUnmappedMemory,
    MemoryError_WriteUnmappedMemory,
    MemoryError_UnmapUnmappedAddress,
    MemoryError_CannotFindMapped,
    MemoryError_RemapWithDifferentSize,
    MemoryError_RemapWithDifferentAddr,
};

template<typename T>
using Result = tl::expected<T, Error>;

}
