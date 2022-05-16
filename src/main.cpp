#include <iostream>
#include <span>
#include "types.hpp"
#include "core/cart.hpp"
#include "common/arg_parser.hpp"

s32 main(s32 argc, char** argv)
{
    using namespace gbemu::core;


    ArgParser args;

    args.registerArg({ "--test-int", "Does something with an integer", ArgParser::ArgType_U32, ArgParser::ArgValue::fromU32(0)});

    if (args.parse(argc, argv))
    {
        for (auto& arg : args.parsedArgs())
        {
            std::cout << arg.description->name;
            if (arg.value.has_value())
            {
                std::cout << " : " << arg.value.value().value << "\n";
            }
        }

    }
    else
    {
        args.showUsage();
    }

    return 0;
}
