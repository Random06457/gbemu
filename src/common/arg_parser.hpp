#pragma once

#include <algorithm>
#include <assert.h>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "types.hpp"

class ArgParser
{
public:
    enum ArgType
    {
        ArgType_None,       // arg
        ArgType_U32,        // <arg=[\d+]>
        ArgType_Bool,       // <arg=true|false>
        ArgType_String,     // <arg="value">
        ArgType_StringNext, // <arg "value">

        ArgType_Count,
    };

    struct ArgValue
    {
        std::string value;
        union
        {
            u32 value_u32;
            bool value_bool;
        };

        static ArgValue fromU32(u32 x)
        {
            return ArgValue{ std::to_string(x), { x } };
        }
        static ArgValue fromBool(bool x)
        {
            return ArgValue{ x ? "true" : "false", { x } };
        }
        static ArgValue fromString(const std::string& x)
        {
            return ArgValue{ x, { 0 } };
        }
    };

    struct ArgDesc
    {
        std::string name;
        std::string description;
        ArgType type;
        std::optional<ArgValue> default_value;
    };

    struct ParsedArg
    {
        ArgDesc* description;
        std::optional<ArgValue> value;
    };

public:
    void registerArg(const ArgDesc& arg) { m_descriptions[arg.name] = arg; }

    bool hasArg(const std::string& name);
    std::optional<ParsedArg> getArg(const std::string& name);

    bool parse(s32 argc, const char** argv);
    bool parse(s32 argc, char** argv)
    {
        return parse(argc, const_cast<const char**>(argv));
    }

    void showUsage();

    static const char* argType(ArgType type);

private:
    std::optional<ArgValue> parseArgValue(ArgType type,
                                          const std::string& value);

    bool parseArg(s32 argc, const char** argv, s32& i);

public:
    auto& parsedArgs() const { return m_parsed_args; }

private:
    std::vector<ParsedArg> m_parsed_args;
    std::map<std::string, ArgDesc> m_descriptions;
};
