#pragma once

#define ERROR_IF(cond, error) do { if (cond) tl::make_unexpected(error); } while (0)
#define UNREACHABLE(msg) assert(0 && msg)

#define GETTER(type, name) type name() { return m_##name; }
#define GETTER_CONST(type, name) type name() const { return m_##name; }

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))