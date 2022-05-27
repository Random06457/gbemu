#pragma once

#define ERROR_IF(cond, error) do { if (cond) return tl::make_unexpected(error); } while (0)
#define PROPAGATE_ERROR(...) ({auto ret = __VA_ARGS__; if (!ret) return tl::make_unexpected(ret.error()); ret.value();})

#define GETTER(type, name) type name() { return m_##name; }
#define GETTER_CONST(type, name) type name() const { return m_##name; }

#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))