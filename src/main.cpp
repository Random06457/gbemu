#include <iostream>
#include <cstdio>
#include <span>
#include "types.hpp"
#include "core/gameboy.hpp"
#include "core/cart.hpp"
#include "common/arg_parser.hpp"
#include "common/fs.hpp"

void printCart(gbemu::core::Cart& cart)
{

    auto hdr = cart.header();
    std::cout << "Title: " << hdr->title << "\n";
    std::cout << "SGB Flag: 0x" << std::hex  << (u32)hdr->sgb_flag << "\n";
    std::cout << "Cartridge Type: 0x" << std::hex << (u32)hdr->cart_type << "(" << hdr->cartType() << ")\n";
    std::cout << "ROM Size: " << hdr->romSize() << "\n";
    std::cout << "RAM Size: " << hdr->ramSize() << "\n";
}

s32 main(s32 argc, char** argv)
{
    using namespace gbemu::core;


    ArgParser args;

    args.registerArg({"--print-header", "Displays information about the selected ROM", ArgParser::ArgType_None, std::nullopt});
    args.registerArg({"--input", "The Input ROM", ArgParser::ArgType_StringNext, std::nullopt});
    args.registerArg({"--bootrom", "The Bootrom", ArgParser::ArgType_StringNext, std::nullopt});

    if (!args.parse(argc, argv))
    {
        args.showUsage();
        return 1;
    }

    auto input = args.getArg("--input");
    auto bootrom = args.getArg("--bootrom");

    Gameboy gb;

    if (bootrom.has_value())
    {
        auto rom = File::readAllBytes(bootrom.value().value.value().value);
        if (!rom)
        {
            std::cerr << "Error while opening bootrom " << rom.error() << "\n";
            return 1;
        }
        gb.setBootrom(rom.value());
    }

    if (input.has_value())
    {
        auto rom = File::readAllBytes(input.value().value.value().value);
        auto cart = std::make_unique<Cart>(rom.value());

        if (args.hasArg("--print-header"))
        {
            if (!rom)
            {
                std::cerr << "Error while opening file " << rom.error() << "\n";
                return 1;
            }

            printCart(*cart);
        }

        gb.setCartridge(std::move(cart));
        gb.powerOn();
    }
    else
    {
        std::cerr << "No input rom\n";
        return 1;
    }

    return 0;
}
