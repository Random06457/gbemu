#include "arg_parser.hpp"
#include "macro.hpp"
#include "common/logging.hpp"

std::optional<ArgParser::ParsedArg> ArgParser::getArg(const std::string& name)
{
    if (!m_descriptions.contains(name))
        UNREACHABLE("Invalid argument name");

    for (auto& arg : m_parsed_args)
    {
        if (arg.description->name == name)
            return arg;
    }

    // check default args
    if (m_descriptions[name].default_value.has_value())
        return ParsedArg{ &m_descriptions[name],
                          m_descriptions[name].default_value };

    return std::nullopt;
}

bool ArgParser::hasArg(const std::string& name)
{
    return getArg(name).has_value();
}

bool ArgParser::parse(s32 argc, const char** argv)
{
    m_parsed_args.clear();
    for (s32 i = 1, incr = 0; i < argc; i += incr)
    {
        if (!parseArg(argc, argv, i))
            return false;
    }
    return true;
}

void ArgParser::showUsage()
{

    fmt::print("Usage: gbemu <arguments...>\n\n");
    fmt::print("Arguments:\n");
    for (auto& desc : m_descriptions)
    {
        fmt::print("{}", desc.second.name);
        switch (desc.second.type)
        {
            case ArgType_Bool: fmt::print("=<true|false>"); break;
            case ArgType_U32: fmt::print("=<int>"); break;
            case ArgType_String: fmt::print("=<string>"); break;
            case ArgType_StringNext: fmt::print(" <string>"); break;
            default: UNREACHABLE("Invalid ArgType");
        }
        fmt::print(" : {}\n", desc.second.description);
    }
}

const char* ArgParser::argType(ArgType type)
{
    switch (type)
    {
        case ArgType_None: return "none";
        case ArgType_U32: return "int";
        case ArgType_Bool: return "bool";
        case ArgType_String: return "string";
        case ArgType_StringNext: return "string";
        default: UNREACHABLE("Invalid ArgType");
    }
}

std::optional<ArgParser::ArgValue>
ArgParser::parseArgValue(ArgType type, const std::string& value)
{
    ArgValue ret;
    ret.value = value;
    switch (type)
    {
        case ArgType_Bool:
            if (value == "true")
                ret.value_bool = true;
            else if (value == "false")
                ret.value_bool = false;
            else
                return std::nullopt;
            return ret;
        case ArgType_String:
        case ArgType_StringNext: return ret;

        case ArgType_U32:
        {
            bool hex = value.starts_with("0x");
            int (*checkc)(int) = std::isdigit;
            int base = 10;
            std::string digits = value;
            if (hex)
            {
                checkc = std::isxdigit;
                base = 16;
                digits = value.substr(2);
            }
            // check if valid
            if (!std::ranges::all_of(digits, checkc))
                return std::nullopt;
            ret.value_u32 = std::stoi(digits, nullptr, base);
        }
            return ret;

        default: UNREACHABLE("Invalid ArgType");
    }
}

static bool unexpectedValue(ArgParser::ArgDesc* desc)
{
    LOG_ERROR("Expected value for argument \"{}\" of type {}\n", desc->name,
              ArgParser::argType(desc->type));
    return false;
}

bool ArgParser::parseArg(s32 argc, const char** argv, s32& i)
{
    std::string arg = argv[i++];

    size_t pos = arg.find('=');
    std::string name = arg.substr(0, pos);

    // check if arg was already parsed
    if (std::ranges::any_of(m_parsed_args, [&name](auto arg)
                            { return arg.description->name == name; }))
    {
        LOG_ERROR("Argument \"{}\" was passed twice.\n", name);
        return false;
    }

    if (!m_descriptions.contains(name))
    {
        LOG_ERROR("Invalid argument \"{}\"\n", name);
        return false;
    }

    ArgDesc* desc = &m_descriptions[name];

    // handle case where there's no '='
    if (pos == std::string::npos)
    {
        // handle case <arg "value">
        if (desc->type == ArgType_StringNext)
        {
            // if there are no more arguments
            if (i >= argc)
                return unexpectedValue(desc);

            m_parsed_args.push_back(
                { desc, parseArgValue(desc->type, argv[i++]) });
            return true;
        }

        if (desc->type != ArgType_None)
            return unexpectedValue(desc);

        m_parsed_args.push_back({ desc, std::nullopt });
        return true;
    }

    if (desc->type != ArgType_Bool && desc->type != ArgType_U32 &&
        desc->type != ArgType_String)
        return unexpectedValue(desc);

    std::string value = arg.substr(pos + 1);
    auto parsed_value = parseArgValue(desc->type, value);
    if (!parsed_value.has_value())
        return unexpectedValue(desc);

    m_parsed_args.push_back({ desc, parsed_value.value() });
    return true;
}
