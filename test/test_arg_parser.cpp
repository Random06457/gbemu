#include <gtest/gtest.h>

#include "macro.hpp"
#include "common/arg_parser.hpp"

#define PARSE(...) \
    const char* args[] = \
    { \
        "dummy", \
        ##__VA_ARGS__ \
    }; \
    ArgParser parser{ARRAY_COUNT(args), args};

TEST(arg_parser, arg_int)
{
    const char* args0[] =
    {
        "dummy",
        "--test=12",
    };
    const char* args1[] =
    {
        "dummy",
        "--test=0x1F",
    };

    const char* args2[] =
    {
        "dummy",
        "--test=1F",
    };

    ArgParser parser;
    parser.registerArg({ "--test", "", ArgParser::ArgType_U32, std::nullopt });

    ASSERT_TRUE(parser.parse(ARRAY_COUNT(args0), args0));
    ASSERT_TRUE(parser.hasArg("--test"));
    ASSERT_EQ(parser.getArg("--test").value().value.value().value_u32, 12);

    ASSERT_TRUE(parser.parse(ARRAY_COUNT(args1), args1));
    ASSERT_TRUE(parser.hasArg("--test"));
    ASSERT_EQ(parser.getArg("--test").value().value.value().value_u32, 0x1F);

    ASSERT_FALSE(parser.parse(ARRAY_COUNT(args2), args2));
}


TEST(arg_parser, arg_bool)
{
    const char* args0[] =
    {
        "dummy",
        "--test=false",
    };
    const char* args1[] =
    {
        "dummy",
        "--test=true",
    };

    const char* args2[] =
    {
        "dummy",
        "--test=1",
    };

    ArgParser parser;
    parser.registerArg({ "--test", "", ArgParser::ArgType_Bool, std::nullopt });

    ASSERT_TRUE(parser.parse(ARRAY_COUNT(args0), args0));
    ASSERT_TRUE(parser.hasArg("--test"));
    ASSERT_EQ(parser.getArg("--test").value().value.value().value_bool, false);

    ASSERT_TRUE(parser.parse(ARRAY_COUNT(args1), args1));
    ASSERT_TRUE(parser.hasArg("--test"));
    ASSERT_EQ(parser.getArg("--test").value().value.value().value_bool, true);

    ASSERT_FALSE(parser.parse(ARRAY_COUNT(args2), args2));
}


TEST(arg_parser, arg_string)
{
    const char* args0[] =
    {
        "dummy",
        "--test=abc",
    };
    const char* args1[] =
    {
        "dummy",
        "--test=",
    };

    const char* args2[] =
    {
        "dummy",
        "--test",
    };

    ArgParser parser;
    parser.registerArg({ "--test", "", ArgParser::ArgType_String, std::nullopt });

    ASSERT_TRUE(parser.parse(ARRAY_COUNT(args0), args0));
    ASSERT_TRUE(parser.hasArg("--test"));
    ASSERT_EQ(parser.getArg("--test").value().value.value().value, "abc");

    ASSERT_TRUE(parser.parse(ARRAY_COUNT(args1), args1));
    ASSERT_TRUE(parser.hasArg("--test"));
    ASSERT_EQ(parser.getArg("--test").value().value.value().value, "");

    ASSERT_FALSE(parser.parse(ARRAY_COUNT(args2), args2));
}

TEST(arg_parser, arg_string_next)
{
    const char* args0[] =
    {
        "dummy",
        "--test", "abc",
    };
    const char* args1[] =
    {
        "dummy",
        "--test=abc",
    };

    const char* args2[] =
    {
        "dummy",
        "--test",
    };

    ArgParser parser;
    parser.registerArg({ "--test", "", ArgParser::ArgType_StringNext, std::nullopt });

    ASSERT_TRUE(parser.parse(ARRAY_COUNT(args0), args0));
    ASSERT_TRUE(parser.hasArg("--test"));
    ASSERT_EQ(parser.getArg("--test").value().value.value().value, "abc");

    ASSERT_FALSE(parser.parse(ARRAY_COUNT(args1), args1));

    ASSERT_FALSE(parser.parse(ARRAY_COUNT(args2), args2));
}

TEST(arg_parser, default_arg)
{
    const char* args[] =
    {
        "dummy",
    };

    ArgParser parser;
    parser.registerArg({ "--test-int", "", ArgParser::ArgType_U32, ArgParser::ArgValue::fromU32(4)});
    parser.registerArg({ "--test-hex-int", "", ArgParser::ArgType_U32, ArgParser::ArgValue::fromU32(5)});
    parser.registerArg({ "--test-string", "", ArgParser::ArgType_String, ArgParser::ArgValue::fromString("def")});
    parser.registerArg({ "--test-bool", "", ArgParser::ArgType_Bool, ArgParser::ArgValue::fromBool(true)});
    parser.registerArg({ "--test-default", "", ArgParser::ArgType_String, ArgParser::ArgValue::fromString("default_val")});
    parser.registerArg({ "--test-no-default", "", ArgParser::ArgType_Bool, std::nullopt});

    ASSERT_TRUE(parser.parse(ARRAY_COUNT(args), args));
    ASSERT_TRUE(parser.hasArg("--test-int"));
    ASSERT_EQ(parser.getArg("--test-int").value().value.value().value_u32, 4);
    ASSERT_TRUE(parser.hasArg("--test-hex-int"));
    ASSERT_EQ(parser.getArg("--test-hex-int").value().value.value().value_u32, 5);
    ASSERT_TRUE(parser.hasArg("--test-string"));
    ASSERT_EQ(parser.getArg("--test-string").value().value.value().value, "def");
    ASSERT_TRUE(parser.hasArg("--test-bool"));
    ASSERT_EQ(parser.getArg("--test-bool").value().value.value().value_bool, true);
    ASSERT_TRUE(parser.hasArg("--test-default"));
    ASSERT_EQ(parser.getArg("--test-default").value().value.value().value, "default_val");
    ASSERT_FALSE(parser.hasArg("--test-no-default"));
}