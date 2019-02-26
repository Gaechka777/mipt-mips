/*
 * riscv_instr.cpp - instruction parser for risc_v
 * Copyright 2018 MIPT-MIPS
 */

#include "riscv_decoder.h"
#include "riscv_instr.h"

#include <func_sim/alu.h>
#include <func_sim/operation.h>

#include <sstream>
#include <vector>

template<typename I> void do_nothing(I* /* instr */) { }
template<typename I> auto execute_lui = do_nothing<I>;
template<typename I> auto execute_auipc = do_nothing<I>;
template<typename I> auto execute_jal = do_nothing<I>;
template<typename I> auto execute_jalr = do_nothing<I>;
template<typename I> auto execute_beq = do_nothing<I>;
template<typename I> auto execute_bne = do_nothing<I>;
template<typename I> auto execute_blt = do_nothing<I>;
template<typename I> auto execute_bge = do_nothing<I>;
template<typename I> auto execute_bltu = do_nothing<I>;
template<typename I> auto execute_bgeu = do_nothing<I>;
template<typename I> auto execute_lb = do_nothing<I>;
template<typename I> auto execute_lh = do_nothing<I>;
template<typename I> auto execute_lw = do_nothing<I>;
template<typename I> auto execute_lbu = do_nothing<I>;
template<typename I> auto execute_lhu = do_nothing<I>;
template<typename I> auto execute_sb = do_nothing<I>;
template<typename I> auto execute_sh = do_nothing<I>;
template<typename I> auto execute_sw = do_nothing<I>;
template<typename I> auto execute_addi = do_nothing<I>;
template<typename I> auto execute_slti = do_nothing<I>;
template<typename I> auto execute_sltiu = do_nothing<I>;
template<typename I> auto execute_xori = do_nothing<I>;
template<typename I> auto execute_ori = do_nothing<I>;
template<typename I> auto execute_andi = do_nothing<I>;
template<typename I> auto execute_slli = do_nothing<I>;
template<typename I> auto execute_srli = do_nothing<I>;
template<typename I> auto execute_srai = do_nothing<I>;
template<typename I> auto execute_add = do_nothing<I>;
template<typename I> auto execute_sub = do_nothing<I>;
template<typename I> auto execute_sll = do_nothing<I>;
template<typename I> auto execute_slt = do_nothing<I>;
template<typename I> auto execute_sltu = do_nothing<I>;
template<typename I> auto execute_xor = do_nothing<I>;
template<typename I> auto execute_srl = do_nothing<I>;
template<typename I> auto execute_sra = do_nothing<I>;
template<typename I> auto execute_or = do_nothing<I>;
template<typename I> auto execute_and = do_nothing<I>;
template<typename I> auto execute_ecall = do_nothing<I>;
template<typename I> auto execute_ebreak = do_nothing<I>;
template<typename I> auto execute_uret = do_nothing<I>;
template<typename I> auto execute_sret = do_nothing<I>;
template<typename I> auto execute_mret = do_nothing<I>;
template<typename I> auto execute_wfi = do_nothing<I>;
template<typename I> auto execute_fence = do_nothing<I>;
template<typename I> auto execute_csrrw = do_nothing<I>;
template<typename I> auto execute_csrrs = do_nothing<I>;
template<typename I> auto execute_csrrc = do_nothing<I>;
template<typename I> auto execute_csrrwi = do_nothing<I>;
template<typename I> auto execute_csrrsi = do_nothing<I>;
template<typename I> auto execute_csrrci = do_nothing<I>;
template<typename I> auto execute_mul = do_nothing<I>;
template<typename I> auto execute_mulh = do_nothing<I>;
template<typename I> auto execute_mulhsu = do_nothing<I>;
template<typename I> auto execute_mulhu = do_nothing<I>;
template<typename I> auto execute_div = do_nothing<I>;
template<typename I> auto execute_divu = do_nothing<I>;
template<typename I> auto execute_rem = do_nothing<I>;
template<typename I> auto execute_remu = do_nothing<I>;

using Src1 = Reg;
using Src2 = Reg;
using Dst  = Reg;

struct RISCVAutogeneratedTableEntry
{
    std::string_view name;
    uint32 match;
    uint32 mask;
    RISCVAutogeneratedTableEntry() = delete;
    constexpr bool check_mask( uint32 bytes) const noexcept { return ( bytes & mask) == match; }
};

#define PASTER(x,y) x ## y
#define EVALUATOR(x,y) PASTER(x,y)

#define DECLARE_INSN(name, match, mask) \
static const RISCVAutogeneratedTableEntry instr_ ## name = { #name, match, mask };
#include <riscv.opcode.gen.h>
#undef DECLARE_INSN

template<typename I>
struct RISCVTableEntry
{
    char subset;
    RISCVAutogeneratedTableEntry entry;
    ALU::Execute<I> function;
    OperationType type;
    char immediate_type;
    char immediate_print_type;
    Src1 src1;
    Src2 src2;
    Dst  dst;
    RISCVTableEntry() = delete;
};

template<typename I>
static const std::vector<RISCVTableEntry<I>> cmd_desc =
{
    /*-------------- I --------------*/
    {'I', instr_lui,   execute_lui<I>,   OUT_ARITHM, 'U', 'x', Src1::ZERO, Src2::ZERO, Dst::RD},
    {'I', instr_auipc, execute_auipc<I>, OUT_ARITHM, 'U', 'x', Src1::ZERO, Src2::ZERO, Dst::RD},
    // Jumps and branches
    {'I', instr_jal,   execute_jal<I>,   OUT_J_JUMP, 'J', 'x', Src1::ZERO, Src2::ZERO, Dst::RD},
    {'I', instr_jalr,  execute_jalr<I>,  OUT_R_JUMP, 'I', 'x', Src1::RS1,  Src2::ZERO, Dst::RD},
    {'I', instr_beq,   execute_beq<I>,   OUT_BRANCH, 'B', 'd', Src1::RS1,  Src2::RS2,  Dst::ZERO},
    {'I', instr_bne,   execute_bne<I>,   OUT_BRANCH, 'B', 'd', Src1::RS1,  Src2::RS2,  Dst::ZERO},
    {'I', instr_blt,   execute_blt<I>,   OUT_BRANCH, 'B', 'd', Src1::RS1,  Src2::RS2,  Dst::ZERO},
    {'I', instr_bge,   execute_bge<I>,   OUT_BRANCH, 'B', 'd', Src1::RS1,  Src2::RS2,  Dst::ZERO},
    {'I', instr_bltu,  execute_bltu<I>,  OUT_BRANCH, 'B', 'd', Src1::RS1,  Src2::RS2,  Dst::ZERO},
    {'I', instr_bgeu,  execute_bgeu<I>,  OUT_BRANCH, 'B', 'd', Src1::RS1,  Src2::RS2,  Dst::ZERO},
    // Loads and stores
    {'I', instr_lb,    execute_lb<I>,    OUT_LOAD,   'I', 'x', Src1::RS1, Src2::ZERO,  Dst::RD},
    {'I', instr_lh,    execute_lh<I>,    OUT_LOAD,   'I', 'x', Src1::RS1, Src2::ZERO,  Dst::RD},
    {'I', instr_lw,    execute_lw<I>,    OUT_LOAD,   'I', 'x', Src1::RS1, Src2::ZERO,  Dst::RD},
    {'I', instr_lbu,   execute_lbu<I>,   OUT_LOADU,  'I', 'x', Src1::RS1, Src2::ZERO,  Dst::RD},
    {'I', instr_lhu,   execute_lhu<I>,   OUT_LOADU,  'I', 'x', Src1::RS1, Src2::ZERO,  Dst::RD},
    {'I', instr_sb,    execute_sb<I>,    OUT_STORE,  'S', 'x', Src1::RS1, Src2::RS2,   Dst::ZERO},
    {'I', instr_sh,    execute_sh<I>,    OUT_STORE,  'S', 'x', Src1::RS1, Src2::RS2,   Dst::ZERO},
    {'I', instr_sw,    execute_sw<I>,    OUT_STORE,  'S', 'x', Src1::RS1, Src2::RS2,   Dst::ZERO},
    // Immediate arithmetics
    {'I', instr_addi,  execute_addi<I>,  OUT_ARITHM, 'I', 'd', Src1::RS1, Src2::ZERO,  Dst::RD},
    {'I', instr_slti,  execute_slti<I>,  OUT_ARITHM, 'I', 'd', Src1::RS1, Src2::ZERO,  Dst::RD},
    {'I', instr_sltiu, execute_sltiu<I>, OUT_ARITHM, 'I', 'd', Src1::RS1, Src2::ZERO,  Dst::RD},
    {'I', instr_xori,  execute_xori<I>,  OUT_ARITHM, 'I', 'x', Src1::RS1, Src2::ZERO,  Dst::RD},
    {'I', instr_ori,   execute_ori<I>,   OUT_ARITHM, 'I', 'x', Src1::RS1, Src2::ZERO,  Dst::RD},
    {'I', instr_andi,  execute_andi<I>,  OUT_ARITHM, 'I', 'x', Src1::RS1, Src2::ZERO,  Dst::RD},
    {'I', instr_slli,  execute_slli<I>,  OUT_ARITHM, 'I', 'd', Src1::RS1, Src2::ZERO,  Dst::RD},
    {'I', instr_srli,  execute_srli<I>,  OUT_ARITHM, 'I', 'd', Src1::RS1, Src2::ZERO,  Dst::RD},
    {'I', instr_srai,  execute_srai<I>,  OUT_ARITHM, 'I', 'd', Src1::RS1, Src2::ZERO,  Dst::RD},
    // Register-register arithmetics
    {'I', instr_add,   execute_add<I>,   OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2,   Dst::RD},
    {'I', instr_sub,   execute_sub<I>,   OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2,   Dst::RD},
    {'I', instr_sll,   execute_sll<I>,   OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2,   Dst::RD},
    {'I', instr_slt,   execute_slt<I>,   OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2,   Dst::RD},
    {'I', instr_sltu,  execute_sltu<I>,  OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2,   Dst::RD},
    {'I', instr_xor,   execute_xor<I>,   OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2,   Dst::RD},
    {'I', instr_srl,   execute_srl<I>,   OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2,   Dst::RD},
    {'I', instr_sra,   execute_sra<I>,   OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2,   Dst::RD},
    {'I', instr_or,    execute_or<I>,    OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2,   Dst::RD},
    {'I', instr_and,   execute_and<I>,   OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2,   Dst::RD},
    /*-------------- M --------------*/
    {'M', instr_mul,    execute_mul<I>,    OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2, Dst::RD},
    {'M', instr_mulh,   execute_mulh<I>,   OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2, Dst::RD},
    {'M', instr_mulhsu, execute_mulhsu<I>, OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2, Dst::RD},
    {'M', instr_mulhu,  execute_mulhu<I>,  OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2, Dst::RD},
    {'M', instr_div,    execute_div<I>,    OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2, Dst::RD},
    {'M', instr_divu,   execute_divu<I>,   OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2, Dst::RD},
    {'M', instr_rem,    execute_rem<I>,    OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2, Dst::RD},
    {'M', instr_remu,   execute_remu<I>,   OUT_ARITHM, ' ', ' ', Src1::RS1, Src2::RS2, Dst::RD},
};

template<typename I>
static const RISCVTableEntry<I> invalid_instr = {'I', {"unknown", 0, 0}, &do_nothing<I>, OUT_ARITHM, ' ', ' ', Src1::ZERO, Src2::ZERO, Dst::ZERO};

template<typename I>
const auto& find_entry( uint32 bytes)
{
    for (const auto& e : cmd_desc<I>)
        if ( e.entry.check_mask( bytes))
            return e;
        
    return invalid_instr<I>;
}

template<typename T>
RISCVInstr<T>::RISCVInstr( uint32 bytes, Addr PC)
    : instr( bytes), PC( PC), new_PC( PC + 4)
{
    const auto& entry = find_entry<RISCVInstr>( bytes);
    RISCVInstrDecoder decoder( bytes);

    operation = entry.type;
    imm_type = entry.immediate_type;
    imm_print_type = entry.immediate_print_type;
    v_imm = decoder.get_immediate( imm_type);
    src1  = decoder.get_register( entry.src1);
    src2  = decoder.get_register( entry.src2);
    dst   = decoder.get_register( entry.dst);
    name  = entry.entry.name;
    print_dst  = entry.dst == Dst::RD;
    print_src1 = entry.src1 == Src1::RS1;
    print_src2 = entry.src2 == Src1::RS1;    
}

static std::string print_immediate( char print_type, uint32 value)
{
    std::ostringstream oss;
    switch ( print_type)
    {
    case 'x': oss << ", 0x" << std::hex << value << std::dec; break;
    case 'd': oss << ", " << static_cast<int32>(value) << std::dec; break;
    default: break;
    }
    return oss.str();
}

template<typename T>
std::string RISCVInstr<T>::generate_disasm() const
{
    std::ostringstream oss;
    oss << name;

    if ( operation == OUT_LOAD || operation == OUT_LOADU || operation == OUT_STORE)
    {
        oss << " $" << (print_dst ? dst : src2)
            << print_immediate( imm_print_type, v_imm)
            << "($" << src1 << ")" << std::dec;
        return oss.str();
    }
    
    if ( print_dst)
        oss <<  " $" << dst;
    if ( print_src1)
        oss << ( print_dst ? ", $" : " $") << src1;
    if ( print_src2)
        oss << ", $" << src2;

    oss << print_immediate( imm_print_type, v_imm);
    return oss.str();
}

template<typename T>
std::string RISCVInstr<T>::get_disasm() const
{
    return generate_disasm();
}

template class RISCVInstr<uint32>;
template class RISCVInstr<uint64>;
template class RISCVInstr<uint128>;