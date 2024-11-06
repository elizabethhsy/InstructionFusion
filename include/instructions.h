#pragma once

#include <string>
#include <vector>

namespace fusion
{

using namespace std;

const vector<string> branchInstructions = {
    "beq",
    "bne",
    "blt",
    "bge",
    "bltu",
    "bgeu",
    "cjal",
    "cjalr",
    "j",
    "jalr",
    "beqz",
    "bnez",
    "blez",
    "bgez",
    "bltz",
    "bgtz",
    "c.bnez",
    "c.beqz",
    "jr",
    "ret"
};

const vector<string> memoryInstructions = {
    "cld",
    "csw",
    "clh",
    "csc",
    "csd",
    "csh",
    "clhu",
    "csb",
    "cfsd",
    "clc",
    "clw",
    "cflw",
    "clbu",
    "fcvt.d.l"
};

const vector<string> arithmeticInstructions = {
    "add", "addi", "addw", "addiw",  // Addition instructions
    "sub", "subw",  // Subtraction instructions
    "mul", "mulw", "mulh", "mulhu",  // Multiplication instructions
    "div", "divu", "divw", "divuw",  // Division instructions
    "rem", "remu", "remw", "remuw",  // Remainder instructions
    "sll", "slli", "slliw", "sllw",  // Shift left logical
    "srl", "srli", "srliw", "srlw",  // Shift right logical
    "sra", "srai", "sraiw", "sraw",  // Shift right arithmetic
    "xor", "xori",  // Bitwise XOR
    "and", "andi",  // Bitwise AND
    "or", "ori",  // Bitwise OR
    "neg", "negw",  // Negation
    "slt", "slti", "sltu", "sltiu"  // Set less than (with unsigned variants)
};

}
