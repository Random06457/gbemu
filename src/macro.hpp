#pragma once

#define ERROR_IF(cond, error) do { if (cond) return tl::make_unexpected(error); } while (0)
#define PROPAGATE_ERROR(...) ({auto err = __VA_ARGS__; if (err) return tl::make_unexpected(err.error()); err.value();})

#define UNREACHABLE(msg) assert(0 && msg)
#define UNIMPLEMENTED(msg) assert(0 && msg)

#define GETTER(type, name) type name() { return m_##name; }
#define GETTER_CONST(type, name) type name() const { return m_##name; }

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))