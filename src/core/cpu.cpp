#include "cpu.hpp"
#include "io.hpp"
#include "attributes.hpp"
#include <cassert>
#include <iostream>

namespace gbemu::core
{


void Cpu::reset()
{
    m_regs.af = 0;
    m_regs.bc = 0;
    m_regs.de = 0;
    m_regs.hl = 0;
    m_regs.pc = 0;
    m_regs.sp = 0xFFFF;
    m_clocks = 0;
}

void Cpu::step()
{
    printf("step\n");

    assert(regs().pc < 0x100);

    u8 op = fetch8();

    execute(op);
}


u8 Cpu::read8(u16 addr)
{
    tick(4);
    auto ret = mem()->read8(addr);
    printf("read(0x%04X)=%02X\n", addr, ret.value_or(0));

    if (!ret)
        printf("INVALID MEMORY : 0x%04X\n", addr);

    // TODO: handle error
    // assert(ret);
    return ret.value_or(0);
}

void Cpu::write8(u16 addr, u8 x)
{
    tick(4);
    auto ret = mem()->write8(addr, x);
    printf("write8(0x%04X, 0x%02X)\n", addr, x);

    if (!ret)
        printf("INVALID MEMORY : 0x%04X\n", addr);

    // TODO: handle error
    // assert(ret);
}

u16 Cpu::read16(u16 addr)
{
    u8 b0 = read8(addr + 0);
    u8 b1 = read8(addr + 1);
    return b1 << 8 | b0;
}

void Cpu::write16(u16 addr, u16 x)
{
    write8(addr + 0, (x >> 0) & 0xFF);
    write8(addr + 1, (x >> 8) & 0xFF);
}

u8 Cpu::fetch8()
{
    return read8(regs().pc++);
}

u16 Cpu::fetch16()
{
    u16 ret = read16(regs().pc);
    regs().pc += 2;
    return ret;
}

void Cpu::push16(u16 x)
{
    tick(4);
    regs().sp -= 2;
    write16(regs().sp, x);
}

u16 Cpu::pop16()
{
    tick(4);
    u16 ret = read16(regs().sp);
    regs().sp += 2;
    return ret;
}


constexpr u8 Cpu::readReg(VREG8 reg)
{
    switch (reg)
    {
        case VREG8_B: return regs().b;
        case VREG8_C: return regs().c;
        case VREG8_D: return regs().d;
        case VREG8_E: return regs().e;
        case VREG8_H: return regs().h;
        case VREG8_L: return regs().l;
        case VREG8_HL8: return read8(regs().hl);
        case VREG8_A: return regs().a;
        case VREG8_HLI: return read8(regs().hl++);
        case VREG8_HLD: return read8(regs().hl--);
        case VREG8_BC8: return read8(regs().bc);
        case VREG8_DE8: return read8(regs().de);
        case VREG8_HA8: return read8(IO_START + fetch8());
        case VREG8_HC: return read8(IO_START + regs().c);
        case VREG8_A16: return read8(fetch16());
        case VREG8_D8: return fetch8();
        default: UNREACHABLE("Invalid VREG8");
    }
}

constexpr void Cpu::writeReg(VREG8 reg, u8 data)
{
    switch (reg)
    {
        case VREG8_B: regs().b = data; break;
        case VREG8_C: regs().c = data; break;
        case VREG8_D: regs().d = data; break;
        case VREG8_E: regs().e = data; break;
        case VREG8_H: regs().h = data; break;
        case VREG8_L: regs().l = data; break;
        case VREG8_HL8: write8(regs().hl, data); break;
        case VREG8_A: regs().a = data; break;
        case VREG8_HLI: write8(regs().hl++, data); break;
        case VREG8_HLD: write8(regs().hl--, data); break;
        case VREG8_BC8: write8(regs().bc, data); break;
        case VREG8_DE8: write8(regs().de, data); break;
        case VREG8_HA8: write8(IO_START + fetch8(), data); break;
        case VREG8_HC: write8(IO_START + regs().c, data); break;
        case VREG8_A16: write8(fetch16(), data); break;
        default: UNREACHABLE("Invalid VREG8");
    }
}

#define LD(d, s) ld(VREG8_##d, VREG8_##s); break
#define INC(r) op_inc(VREG8_##r); break
#define DEC(r) op_dec(VREG8_##r); break
#define PUSH(x) push16(regs().x); break
#define POP(x) regs().x = pop16(); break
#define CALL_A16(cond) call(cond, fetch16()); break
#define RET(cond) ret(cond, pop16()); break
#define JR_R8(cond) jr(cond, fetch8()); break

#define Z regs().flags.z
#define NZ !regs().flags.z
#define C regs().flags.c
#define NC !regs().flags.c
#define N regs().flags.n
#define H regs().flags.h


#define MAKE_OP1(start, size, func, ...) \
    if (op >= start && op < start + size) \
    { \
        VREG8 r = (VREG8)((op - start) % 8); \
        func(r, ##__VA_ARGS__); \
        return; \
    }

#define MAKE_OP_ACCU(start, size, func, ...) \
    if (op >= start && op < start + size) \
    { \
        VREG8 src = (VREG8)((op - start) % 8); \
        func(VREG8_A, src, ##__VA_ARGS__); \
        return; \
    }

#define MAKE_OP_IDX(start, size, func, ...) \
    if (op >= start && op <= start + size - 1) \
    { \
        u8 idx = (op - start) / 8; \
        VREG8 r = (VREG8)((op - start) % 8); \
        func(r, idx, ##__VA_ARGS__); \
        return; \
    }

void Cpu::execute(u8 op)
{
    auto jr = [this] (bool cond, s8 n) ALWAYS_INLINE
    {
        if (cond)
        {
            tick(4);
            regs().pc += n;
        }
    };

    auto ret = [this] (bool cond, u16 x) ALWAYS_INLINE
    {
        if (cond)
            regs().pc = x;
    };

    auto call = [this] (bool cond, u16 addr) ALWAYS_INLINE
    {
        if (cond)
        {
            push16(regs().pc);
            regs().pc = addr;
        }
    };

    auto ld = [this] (VREG8 dst, VREG8 src) ALWAYS_INLINE
    {
        writeReg(dst, readReg(src));
    };

    auto add = [this] (VREG8 dst, VREG8 src, bool c) ALWAYS_INLINE
    {
        u8 a = readReg(dst);
        u8 b = readReg(src);
        if (c && regs().flags.c)
            b++;
        u8 d = a + b;

        Z = d == 0;
        N = 0;
        H = !!(((a & 0xF) + (b & 0xF)) & 0x10);
        C = d < a + b;

        writeReg(dst, d);
    };
    auto sub = [this] (VREG8 dst, VREG8 src, bool c) ALWAYS_INLINE
    {
        u8 a = readReg(dst);
        u8 b = readReg(src);
        if (c && regs().flags.c)
            b--;
        u8 d = a + b;

        Z = d == 0;
        N = 1;
        H = !!(((a & 0xF) - (b & 0xF)) & 0x10);
        C = d > a - b;

        writeReg(dst, d);
    };
    auto op_and = [this] (VREG8 dst, VREG8 src) ALWAYS_INLINE
    {
        u8 a = readReg(dst);
        u8 b = readReg(src);
        u8 d = a & b;

        Z = d == 0;
        N = 0;
        H = 1;
        C = 0;

        writeReg(dst, d);
    };
    auto op_xor = [this] (VREG8 dst, VREG8 src) ALWAYS_INLINE
    {
        u8 a = readReg(dst);
        u8 b = readReg(src);
        u8 d = a ^ b;

        Z = d == 0;
        N = 0;
        H = 0;
        C = 0;

        writeReg(dst, d);
    };
    auto op_or = [this] (VREG8 dst, VREG8 src) ALWAYS_INLINE
    {
        u8 a = readReg(dst);
        u8 b = readReg(src);
        u8 d = a | b;

        Z = d == 0;
        N = 0;
        H = 0;
        C = 0;

        writeReg(dst, d);
    };
    auto op_cp = [this] (VREG8 dst, VREG8 src) ALWAYS_INLINE
    {
        u8 a = readReg(dst);
        u8 b = readReg(src);
        u8 d = a - b;

        Z = d == 0;
        N = 1;
        H = !!(((a & 0xF) - (b & 0xF)) & 0x10);
        C = d > a - b;
    };

    auto op_inc = [this] (VREG8 r) ALWAYS_INLINE
    {
        u8 a = readReg(r);
        u8 b = 1;
        u8 d = a + b;

        Z = d == 0;
        N = 0;
        H = !!(((b & 0xF) + a) & 0x10);

        writeReg(r, d);
    };

    auto op_dec = [this] (VREG8 r) ALWAYS_INLINE
    {
        u8 a = readReg(r);
        u8 b = 1;
        u8 d = a - b;

        Z = d == 0;
        N = 1;
        H = !!(((b & 0xF) - a) & 0x10);

        writeReg(r, d);
    };

    // LD
    if (op != 0x76)
        MAKE_OP_ACCU(0x40, 0x40, ld);

    MAKE_OP_ACCU(0x80, 8, add, false);
    MAKE_OP_ACCU(0x88, 8, add, true);
    MAKE_OP_ACCU(0x90, 8, sub, false);
    MAKE_OP_ACCU(0x98, 8, sub, true);
    MAKE_OP_ACCU(0xA0, 8, op_and);
    MAKE_OP_ACCU(0xA8, 8, op_xor);
    MAKE_OP_ACCU(0xB0, 8, op_or);
    MAKE_OP_ACCU(0xB8, 8, op_cp);

    switch (op)
    {
        case 0x00: // NOP
            break;
        case 0x10: // STOP
            UNIMPLEMENTED("STOP");
            break;
        case 0x76: // HALT
            UNIMPLEMENTED("HALT");
            break;

        case 0x20: JR_R8(NZ);
        case 0x30: JR_R8(NC);
        case 0x18: JR_R8(true);
        case 0x28: JR_R8(Z);
        case 0x38: JR_R8(C);

        case 0xCD: CALL_A16(true);
        case 0xC4: CALL_A16(NZ);
        case 0xD4: CALL_A16(NC);
        case 0xCC: CALL_A16(Z);
        case 0xDC: CALL_A16(C);

        case 0xC5: PUSH(bc);
        case 0xD5: PUSH(de);
        case 0xE5: PUSH(hl);
        case 0xF5: PUSH(af);

        case 0xC1: POP(bc);
        case 0xD1: POP(de);
        case 0xE1: POP(hl);
        case 0xF1: POP(af);

        case 0x01: // LD BC, d16
            regs().bc = fetch16();
            break;
        case 0x11: // LD DE, d16
            regs().de = fetch16();
            break;
        case 0x21: // LD HL, d16
            regs().hl = fetch16();
            break;
        case 0x31: // LD SP, d16
            regs().sp = fetch16();
            break;

        case 0x02: LD(BC8, A);
        case 0x12: LD(DE8, A);
        case 0x22: LD(HLI, A);
        case 0x32: LD(HLD, A);

        case 0x06: LD(B, D8);
        case 0x16: LD(D, D8);
        case 0x26: LD(E, D8);
        case 0x36: LD(HL8, D8);

        case 0x0A: LD(A, BC8);
        case 0x1A: LD(A, D8);
        case 0x2A: LD(A, HLI);
        case 0x3A: LD(A, HLD);

        case 0x0E: LD(C, D8);
        case 0x1E: LD(E, D8);
        case 0x2E: LD(L, D8);
        case 0x3E: LD(A, D8);

        case 0x04: INC(B);
        case 0x14: INC(D);
        case 0x24: INC(H);
        case 0x34: INC(HL8);
        case 0x0C: INC(C);
        case 0x1C: INC(E);
        case 0x2C: INC(L);
        case 0x3C: INC(A);

        case 0x05: DEC(B);
        case 0x15: DEC(D);
        case 0x25: DEC(H);
        case 0x35: DEC(HL8);
        case 0x0D: DEC(C);
        case 0x1D: DEC(E);
        case 0x2D: DEC(L);
        case 0x3D: DEC(A);

        case 0x03: regs().bc++; break;
        case 0x13: regs().de++; break;
        case 0x23: regs().hl++; break;
        case 0x33: regs().sp++; break;
        case 0x0B: regs().bc--; break;
        case 0x1B: regs().de--; break;
        case 0x2B: regs().hl--; break;
        case 0x3B: regs().sp--; break;

        case 0xC8: RET(Z);
        case 0xD8: RET(C);
        case 0xC9: RET(true);

        case 0xFE: op_cp(VREG8_A, VREG8_D8); break;

        case 0xEA: LD(A16, A);
        case 0xFA: LD(A, A16);

        case 0x07: // RLCA
        {
            bool new_c = regs().a >> 7;
            regs().a <<= 1;
            regs().a |= new_c;

            Z = 0;
            N = 0;
            H = 0;
            C = new_c;
            break;
        }
        case 0x17: // RLA
        {
            bool new_c = regs().a >> 7;
            regs().a <<= 1;
            regs().a |= C;

            Z = 0;
            N = 0;
            H = 0;
            C = new_c;
            break;
        }


        case 0xE0: LD(HA8, A);
        case 0xF0: LD(A, HA8);

        case 0xE2: LD(HC, A);
        case 0xF2: LD(A, HC);

        case 0xCB:
            executeCB(fetch8());
            break;

        case 0xF3: // DI
            interrupt_master_enable = false;
            break;
        case 0xFB: // EI
            interrupt_master_enable = true;
            break;

        default:
            fflush(stdout);
            UNIMPLEMENTED("Unimplemented opcode");
    }
}

void Cpu::executeCB(u8 op)
{
    auto op_bit = [this] (VREG8 r, size_t idx) ALWAYS_INLINE
    {
        u8 b = readReg(r);

        Z = (b & (1 << idx)) == 0;
        N = 0;
        H = 1;
    };

    auto op_res = [this] (VREG8 r, size_t idx) ALWAYS_INLINE
    {
        u8 b = readReg(r);
        b &= ~(1 << idx);
        writeReg(r, b);
    };

    auto op_set = [this] (VREG8 r, size_t idx) ALWAYS_INLINE
    {
        u8 b = readReg(r);
        b |= 1 << idx;
        writeReg(r, b);
    };

    auto op_sr = [this]<typename T> (VREG8 r, bool c, bool rotate) ALWAYS_INLINE
    {
        T b = readReg(r);

        bool new_c = b & 1;

        b >>= 1;
        if (rotate)
            b |= (c ? new_c : C) << 7;

        Z = b == 0;
        N = 0;
        H = 0;
        C = new_c;

        writeReg(r, b);
    };

    auto op_sl = [this]<typename T> (VREG8 r, bool c, bool rotate) ALWAYS_INLINE
    {
        T b = readReg(r);

        bool new_c = b >> 7;

        b <<= 1;
        if (rotate)
            b |= c ? new_c : C;

        Z = b == 0;
        N = 0;
        H = 0;
        C = new_c;

        writeReg(r, b);
    };

    auto op_swap = [this] (VREG8 r) ALWAYS_INLINE
    {
        u8 b = readReg(r);
        b = ((b & 0xF) << 4) | ((b >> 4) & 0xF);
        writeReg(r, b);
    };

    MAKE_OP1(0x00, 8, op_sl.operator()<u8>, true, true); // rlc
    MAKE_OP1(0x08, 8, op_sr.operator()<u8>, true, true); // rrc
    MAKE_OP1(0x10, 8, op_sl.operator()<u8>, false, true); // rl
    MAKE_OP1(0x18, 8, op_sr.operator()<u8>, false, true); // rr
    MAKE_OP1(0x20, 8, op_sl.operator()<u8>, false, false); // sla
    MAKE_OP1(0x28, 8, op_sr.operator()<u8>, false, false); // sra
    MAKE_OP1(0x30, 8, op_swap); // swap
    MAKE_OP1(0x38, 8, op_sl.operator()<s8>, false, false); // srl

    MAKE_OP_IDX(0x40, 0x40, op_bit);
    MAKE_OP_IDX(0x80, 0x40, op_res);
    MAKE_OP_IDX(0xC0, 0x40, op_set);

    UNIMPLEMENTED("Unimplemented opcode");
}

}
