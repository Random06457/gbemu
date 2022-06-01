#include <string>
#include "fmt/color.h"

#define LOG(...) logging::logInfo(__FILE__, __LINE__, fmt::format(__VA_ARGS__))
#define LOG_ERROR(...)                                                         \
    logging::logError(__FILE__, __LINE__, fmt::format(__VA_ARGS__))
#define UNIMPLEMENTED(...)                                                     \
    logging::unimplemented(__FILE__, __LINE__, fmt::format(__VA_ARGS__))
#define UNREACHABLE(...)                                                       \
    logging::unreachable(__FILE__, __LINE__, fmt::format(__VA_ARGS__))

namespace logging
{

[[noreturn]] void unimplemented(const std::string& file, size_t line,
                                const std::string& msg);
[[noreturn]] void unreachable(const std::string& file, size_t line,
                              const std::string& msg);
void logError(const std::string& file, size_t line, const std::string& msg);
void logInfo(const std::string& file, size_t line, const std::string& msg);

}
