#include <sstream>
#include <fstream>
#include <iterator>
#include "fs.hpp"
#include "macro.hpp"

bool File::exists(const fs::path& path)
{
    return fs::exists(path) && !fs::is_directory(path);
}

File::Result<std::string> File::readAllText(const fs::path& path)
{
    std::ifstream fs;
    fs.open(path);

    ERROR_IF(!fs.is_open(), FileSystemError_FileOpenFailed);

    std::ostringstream ss;
    ss << fs.rdbuf();
    return ss.str();
}

File::Result<std::vector<u8>> File::readAllBytes(const fs::path& path)
{
    std::ifstream fs;
    fs.open(path, std::ios::binary);

    ERROR_IF(!fs.is_open(), FileSystemError_FileOpenFailed);

    std::vector<u8> ret((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));

    return ret;
}

File::Result<void> File::writeAllBytes(const fs::path& path, const void* data, size_t size)
{
    std::ofstream fs;
    fs.open(path, std::ios::binary);

    ERROR_IF(!fs.is_open(), FileSystemError_FileOpenFailed);

    fs.write(reinterpret_cast<const char*>(data), size);

    return {};
}
