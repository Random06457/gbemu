#! /usr/bin/env python3
import argparse
import json

import argparse


def gen_disas_code(table):
    print("switch(op)")
    print("{")
    print()

    for op in table["unprefixed"]:
        if (table["unprefixed"][op]["mnemonic"].startswith("ILLEGAL")):
            continue
        gen_disas_code_instr(table["unprefixed"][op])

    print()
    print("    default:")
    print("        UNIMPLEMENTED(\"Unimplemented or invalid opcode (0x{:02X})\", op);")
    print("}")

def gen_disas_code_instr(instr):
    disas : str = disas_instr(instr)
    suffix = None
    if "a16" in disas or "d16" in disas:
        disas = disas.replace("d16", "a16")
        disas = disas.replace("a16", "${:04X}")
        suffix = ", read16()"
    if "a8" in disas or "d8" in disas or "r8" in disas or "s8" in disas:
        disas = disas.replace("d8", "a8")
        disas = disas.replace("r8", "a8")
        disas = disas.replace("s8", "a8")
        disas = disas.replace("a8", "${:02X}")
        suffix = ", read8()"

    if suffix:
        disas = "fmt::format(\"" + disas + "\"" + suffix + ")"
    else:
        disas = "\"" + disas + "\""

    print("    case OP_" + op_name(instr) + ": return " + disas + ";")


def disas_instr(instr):
    mnemonic : str = instr["mnemonic"]
    operands = instr["operands"]

    line = mnemonic + " "

    for i in range(len(operands)):
        operand : dict = operands[i]

        name : str = operand["name"]
        imm : str = operand["immediate"]
        inc : str = operand["increment"] if "increment" in operand else None
        dec : str = operand["decrement"] if "decrement" in operand else None

        if not imm:
            line += "("

        line += name

        if inc:
            line += "+"
        if dec:
            line += "-"

        if not imm:
            line += ")"

        # exclude case SP+r8
        if i + 1 < len(operands) and not (len(operands) == 3 and i == 1):
            line += ", "

    return line

def op_name(instr):
    mnemonic : str = instr["mnemonic"]
    operands = instr["operands"]
    line = mnemonic

    for i in range(len(operands)):
        operand : dict = operands[i]

        name : str = operand["name"]
        imm : str = operand["immediate"]
        inc : str = operand["increment"] if "increment" in operand else None
        dec : str = operand["decrement"] if "decrement" in operand else None


        if not imm:
            line += "_MEM"

        line += "_" + name

        if inc:
            line += "I"
        if dec:
            line += "D"
    return line


def gen_opcode_define(op, instr):
    mnemonic : str = instr["mnemonic"]

    if (mnemonic.startswith("ILLEGAL")):
        return

    line : str = ""
    line += "MAKE_OP(" + op + ", " + op_name(instr) + ")"

    line = line.ljust(40)
    line += "// " + disas_instr(instr)
    return line

def gen_opcode_header(table):
    print("#pragma once")
    print()
    print("#include \"types.hpp\"")
    print()
    print("#define MAKE_OP(i, name) OP_##name = i,")
    print("enum")
    print("{")
    print()

    for op in table["unprefixed"]:
        if (table["unprefixed"][op]["mnemonic"].startswith("ILLEGAL")):
            continue
        print(gen_opcode_define(op, table["unprefixed"][op]))

    print()
    print("};")
    print()
    print("#undef MAKE_OP")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("json")
    args = parser.parse_args()

    with open(args.json) as f:
        text = f.read()

    table = json.loads(text)
    # gen_opcode_header(table)
    gen_disas_code(table)

if __name__ == "__main__":
   main()