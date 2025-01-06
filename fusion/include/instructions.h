#pragma once

#include <fusion_export.h>

#include <string>
#include <vector>

namespace fusion
{

using namespace std;

const vector<string> arithmeticInstructions = {
    "add", "addi", "addiw", "addw", "and", "andi", "auipcc", "c.add", "c.addi", "c.addiw", "c.addw", 
    "c.and", "c.andi", "c.mv", "c.or", "c.slli", "c.srai", "c.srli", "c.sub", "c.subw", "c.xor",
    "camoadd.d", "camoadd.d.aqrl", "camoadd.w", "camoadd.w.aqrl", "camoadd.w.rl", "candperm",
    "cgetaddr", "cgetbase", "cgetlen", "cgetoffset", "cgetsealed", "cgettag", "cincoffset",
    "cmove", "cram", "crepresentablealignmentmask", "croundrepresentablelength",
    "csealentry", "csetaddr", "csetbounds", "csetboundsexact", "csetoffset",
    "cspecialr", "cspecialrw", "cspecialw", "csrrs", "csub", "divu", "divuw", "divw",
    "fabs.d", "fadd.d", "fadd.s", "fcvt.d.l", "fcvt.d.lu", "fcvt.d.s", 
    "fcvt.d.w", "fcvt.d.wu", "fcvt.l.d", "fcvt.l.s", "fcvt.lu.d", "fcvt.lu.s", "fcvt.s.d", "fcvt.s.l", 
    "fcvt.s.lu", "fcvt.s.w", "fcvt.s.wu", "fcvt.w.d", "fcvt.w.s", "fcvt.wu.d", "fdiv.d", "fdiv.s",
    "feq.d", "feq.s", "fle.d", "fle.s", "flt.d", "flt.s", "fmadd.d", "fmadd.s", "fmax.d", "fmax.s", 
    "fmin.d", "fmin.s", "fmsub.d", "fmul.d", "fmul.s", "fmv.d", "fmv.d.x", "fmv.s", "fmv.w.x", 
    "fmv.x.d", "fmv.x.w", "fneg.d", "fneg.s", "fnmsub.d", "frcsr", "fsgnj.d", "fsgnj.s", "fsgnjn.d", 
    "fsgnjx.d", "fsgnjx.s", "fsqrt.d", "fsub.d", "fsub.s", "mul", "mulh", "mulhu", "mulw", "mv", 
    "neg", "negw", "not", "or", "ori", "rdtime", "rem", "remu", "remuw", "remw",
    "seqz", "sext.w", "sgtz", "sll", "slli", "slliw", "sllw", "slt", "slti", "sltiu", "sltu",
    "snez", "srai", "sraiw", "sraw", "srl", "srli", "srliw", "srlw", "sub", 
    "subw", "xor", "xori"
};

const vector<string> branchInstructions = {
    "beq", "beqz", "bge", "bgeu", "bgez", "bgtz", "blez", "blt", "bltu", "bltz", "bne", "bnez",
    "c.beqz", "c.bnez", "cjal", "cjalr", "j", "jalr", "jr", "ret"
};

const vector<string> memoryInstructions = {
    "c.li", "c.lui", "cfld", "cflw", "cfsd", "cfsw", "clb", "clbu", "clc", "cld",
    "clh", "clhu", "clr.b.aq", "clr.w.aqrl", "clw", "clwu", "crrl", "csb", "csc",
    "csc.b", "csc.w.aqrl", "csd", "csh", "csw", "lui"
};

const vector<string> systemInstructions = {
    "<unknown>", "ecall", "fence"
};

}
