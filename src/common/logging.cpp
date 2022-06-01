#include "logging.hpp"

namespace logging
{

void unimplemented(const std::string& file, size_t line, const std::string& msg)
{
    fflush(stdout);
    fflush(stderr);
    fmt::print(stderr, fmt::fg(fmt::color::red), "UNIMPLEMENTED: ");
    fmt::print(stderr, fmt::fg(fmt::color::white), "\"{}\" at {}:{}\n", msg,
               file, line);
    std::abort();
}

void unreachable(const std::string& file, size_t line, const std::string& msg)
{
    fflush(stdout);
    fflush(stderr);
    fmt::print(stderr, fmt::fg(fmt::color::red), "UNREACHABLE: ");
    fmt::print(stderr, fmt::fg(fmt::color::white), "\"{}\" at {}:{}\n", msg,
               file, line);
    std::abort();
}

void logError(const std::string& file, size_t line, const std::string& msg)
{
    fmt::print(stderr, fmt::fg(fmt::color::red), "ERROR: ");
    fmt::print(stderr, fmt::fg(fmt::color::white), "\"{}\" at {}:{}\n", msg,
               file, line);
}

void logInfo(const std::string& file, size_t line, const std::string& msg)
{
    fmt::print("{}", msg);
}

}
