#pragma once

#include <vector>
#include <filesystem>
#include "types.hpp"
#include "tl/expected.hpp"

namespace fs = std::filesystem;

enum FileSystemError
{
    FileSystemError_FileOpenFailed,
};

class File
{
    template<typename T>
    using Result = tl::expected<T, FileSystemError>;

    static Result<std::vector<u8>> readAllBytes(const fs::path& path);

    static Result<std::string> readAllText(const fs::path& path);

    static bool exists(const fs::path& path);
};
