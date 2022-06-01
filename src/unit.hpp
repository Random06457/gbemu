#pragma once

inline constexpr unsigned long long operator"" _kb(unsigned long long x)
{
    return x * 1024;
}

inline constexpr unsigned long long operator"" _mb(unsigned long long x)
{
    return x * 1024 * 1024;
}