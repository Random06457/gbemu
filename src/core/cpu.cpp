#include "cpu.hpp"
#include "io.hpp"
#include "attributes.hpp"
#include "opcode.hpp"
#include <cassert>
#include <iostream>
#include <cstdarg>

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
    log("step\n");

    static u16 last_pc = 0;

    m_logging_enable = regs().pc >= 0xE6;

    assert(regs().pc < 0x100);

    u8 op = fetch8();

    execute(op);

    assert(last_pc != regs().pc);

    last_pc = regs().pc;
}

void Cpu::log(const char* fmt, ...)
{
    std::va_list args;

    va_start(args, fmt);

    if (m_logging_enable)
        vprintf(fmt, args);

    va_end(args);
}


u8 Cpu::read8(u16 addr)
{
    tick(4);
    auto ret = mem()->read8(addr);
    log("read(0x%04X)=%02X\n", addr, ret.value_or(0));

    if (!ret)
        log("INVALID MEMORY : 0x%04X\n", addr);

    // TODO: handle error
    // assert(ret);
    return ret.value_or(0);
}

void Cpu::write8(u16 addr, u8 x)
{
    tick(4);
    auto ret = mem()->write8(addr, x);
    log("write8(0x%04X, 0x%02X)\n", addr, x);

    if (!ret)
        log("INVALID MEMORY : 0x%04X\n", addr);

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


u8 Cpu::readReg(VREG8 reg)
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

void Cpu::writeReg(VREG8 reg, u8 data)
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

#define MAKE_OP_LD(start, size, func, ...) \
    if (op >= start && op < start + size) \
    { \
        VREG8 dst = (VREG8)((op - start) / 8); \
        VREG8 src = (VREG8)((op - start) % 8); \
        func(dst, src, ##__VA_ARGS__); \
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
        H = !!(((a & 0xF) + b) & 0x10);

        writeReg(r, d);
    };

    auto op_dec = [this] (VREG8 r) ALWAYS_INLINE
    {
        u8 a = readReg(r);
        u8 b = 1;
        u8 d = a - b;

        Z = d == 0;
        N = 1;
        H = !!(((a & 0xF) - b) & 0x10);

        writeReg(r, d);
    };

    // LD
    if (op != OP_HALT)
        MAKE_OP_LD(OP_LD_B_B, 0x40, ld);

    MAKE_OP_ACCU(OP_ADD_A_B, 8, add, false);
    MAKE_OP_ACCU(OP_ADC_A_B, 8, add, true);
    MAKE_OP_ACCU(OP_SUB_B, 8, sub, false);
    MAKE_OP_ACCU(OP_SBC_A_B, 8, sub, true);
    MAKE_OP_ACCU(OP_AND_B, 8, op_and);
    MAKE_OP_ACCU(OP_XOR_B, 8, op_xor);
    MAKE_OP_ACCU(OP_OR_B, 8, op_or);
    MAKE_OP_ACCU(OP_CP_B, 8, op_cp);

    switch (op)
    {
        case OP_NOP:
            break;
        case OP_STOP_d8:
            UNIMPLEMENTED("STOP");
            break;
        case OP_HALT:
            UNIMPLEMENTED("HALT");
            break;

        case OP_JR_NZ_r8: JR_R8(NZ);
        case OP_JR_NC_r8: JR_R8(NC);
        case OP_JR_r8: JR_R8(true);
        case OP_JR_Z_r8: JR_R8(Z);
        case OP_JR_C_r8: JR_R8(C);

        case OP_CALL_a16: CALL_A16(true);
        case OP_CALL_NZ_a16: CALL_A16(NZ);
        case OP_CALL_NC_a16: CALL_A16(NC);
        case OP_CALL_Z_a16: CALL_A16(Z);
        case OP_CALL_C_a16: CALL_A16(C);

        case OP_PUSH_BC: PUSH(bc);
        case OP_PUSH_DE: PUSH(de);
        case OP_PUSH_HL: PUSH(hl);
        case OP_PUSH_AF: PUSH(af);

        case OP_POP_BC: POP(bc);
        case OP_POP_DE: POP(de);
        case OP_POP_HL: POP(hl);
        case OP_POP_AF: POP(af);

        case OP_LD_BC_d16: regs().bc = fetch16(); break;
        case OP_LD_DE_d16: regs().de = fetch16(); break;
        case OP_LD_HL_d16: regs().hl = fetch16(); break;
        case OP_LD_SP_d16: regs().sp = fetch16(); break;

        case OP_LD_MEM_BC_A: LD(BC8, A);
        case OP_LD_MEM_DE_A: LD(DE8, A);
        case OP_LD_MEM_HLI_A: LD(HLI, A);
        case OP_LD_MEM_HLD_A: LD(HLD, A);

        case OP_LD_B_d8: LD(B, D8);
        case OP_LD_D_d8: LD(D, D8);
        case OP_LD_H_d8: LD(H, D8);
        case OP_LD_MEM_HL_d8: LD(HL8, D8);

        case OP_LD_A_MEM_BC: LD(A, BC8);
        case OP_LD_A_MEM_DE: LD(A, DE8);
        case OP_LD_A_MEM_HLI: LD(A, HLI);
        case OP_LD_A_MEM_HLD: LD(A, HLD);

        case OP_LD_C_d8: LD(C, D8);
        case OP_LD_E_d8: LD(E, D8);
        case OP_LD_L_d8: LD(L, D8);
        case OP_LD_A_d8: LD(A, D8);

        case OP_INC_B: INC(B);
        case OP_INC_D: INC(D);
        case OP_INC_H: INC(H);
        case OP_INC_MEM_HL: INC(HL8);
        case OP_INC_C: INC(C);
        case OP_INC_E: INC(E);
        case OP_INC_L: INC(L);
        case OP_INC_A: INC(A);

        case OP_DEC_B: DEC(B);
        case OP_DEC_D: DEC(D);
        case OP_DEC_H: DEC(H);
        case OP_DEC_MEM_HL: DEC(HL8);
        case OP_DEC_C: DEC(C);
        case OP_DEC_E: DEC(E);
        case OP_DEC_L: DEC(L);
        case OP_DEC_A: DEC(A);

        case OP_INC_BC: regs().bc++; break;
        case OP_INC_DE: regs().de++; break;
        case OP_INC_HL: regs().hl++; break;
        case OP_INC_SP: regs().sp++; break;
        case OP_DEC_BC: regs().bc--; break;
        case OP_DEC_DE: regs().de--; break;
        case OP_DEC_HL: regs().hl--; break;
        case OP_DEC_SP: regs().sp--; break;

        case OP_RET_Z: RET(Z);
        case OP_RET_C: RET(C);
        case OP_RET: RET(true);

        case OP_CP_d8: op_cp(VREG8_A, VREG8_D8); break;

        case OP_LD_MEM_a16_A: LD(A16, A);
        case OP_LD_A_MEM_a16: LD(A, A16);

        case OP_LD_MEM_a16_SP: write16(fetch16(), regs().sp); break;

        case OP_RLCA:
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
        case OP_RLA:
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
        case OP_RRCA:
        {
            bool new_c = regs().a & 1;

            regs().a >>= 1;
            regs().a |= new_c << 7;

            Z = 0;
            N = 0;
            H = 0;
            C = new_c;
            break;
        }
        case OP_RRA:
        {
            bool new_c = regs().a & 1;

            regs().a >>= 1;
            regs().a |= C << 7;

            Z = 0;
            N = 0;
            H = 0;
            C = new_c;
            break;
        }


        case OP_LDH_MEM_a8_A: LD(HA8, A);
        case OP_LDH_A_MEM_a8: LD(A, HA8);

        case OP_LD_MEM_C_A: LD(HC, A);
        case OP_LD_A_MEM_C: LD(A, HC);

        case OP_PREFIX:
            executeCB(fetch8());
            break;

        case OP_DI: // DI
            interrupt_master_enable = false;
            break;
        case OP_EI: // EI
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
