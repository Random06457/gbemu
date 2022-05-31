#include <span>
#include "common/arg_parser.hpp"
#include "common/fs.hpp"
#include "common/logging.hpp"
#include "types.hpp"
#include "core/cart.hpp"
#include "core/gameboy.hpp"

void printCart(gbemu::core::Cart& cart)
{
    auto hdr = cart.header();
    fmt::print("Title: {}\n", hdr->title);
    fmt::print("SGB Flag: 0x{:X}\n", hdr->sgb_flag);
    fmt::print("Cartridge Type: 0x{:X} ({})\n", hdr->cart_type, hdr->cartType());
    fmt::print("ROM Size: {}\n",hdr->romSize());
    fmt::print("RAM Size: {}\n", hdr->ramSize());
}

s32 gui_main(gbemu::core::Gameboy& gb);

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
            LOG_ERROR("Error while opening bootrom : {}\n", rom.error());
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
                LOG_ERROR("Error while opening file : {}\n", rom.error());
                return 1;
            }

            printCart(*cart);
        }

        gb.setCartridge(std::move(cart));
    }
    else
    {
        LOG_ERROR("No input rom\n");
        return 1;
    }

    gui_main(gb);

    return 0;
}
