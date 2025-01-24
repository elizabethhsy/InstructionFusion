#pragma once

#include "fusion.h"
#include "instructions.h"

#include <utility>

namespace fusion
{

using namespace std;

// the functions are example fusion rules, which take a sequence of (previous)
// instructions in the block as input, along with the current instruction, and
// outputs an enum value stating whether the current instruction is fusable
// with the rest of the block.

// Note that this library relies on the assumption that every prefix of a
// sequence of fusable instructions is also fusable. This means that when a
// function receives the block and the current instruction, all the instructions
// in the block are assumed to be fusable with each other.

const FusionRule FUSION_CORE_EXPORT arithmeticOnly(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction) {
        return (arithmeticInstructions.contains(instruction.instr)) ?
            FusableResult::FUSABLE : FusableResult::NOT_FUSABLE;
    }
);

const FusionRule FUSION_CORE_EXPORT arithmeticEndMemory(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction) {
        if (arithmeticInstructions.contains(instruction.instr)) {
            return FusableResult::FUSABLE;
        }
        if (memoryInstructions.contains(instruction.instr)) {
            return FusableResult::END_OF_FUSABLE;
        }
        return FusableResult::NOT_FUSABLE;
    }
);

const FusionRule FUSION_CORE_EXPORT arithmeticEndBranch(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction) {
        if (arithmeticInstructions.contains(instruction.instr)) {
            return FusableResult::FUSABLE;
        }
        if (branchInstructions.contains(instruction.instr)) {
            return FusableResult::END_OF_FUSABLE;
        }
        return FusableResult::NOT_FUSABLE;
    }
);

// first check if the maximum length of a fusable block is exceeded, then call
// another fusion rule
FusionRule FUSION_CORE_EXPORT maxLength(int length) {
    return FusionRule(
        [length](
            vector<shared_ptr<Instr>> const& block,
            Instr const& instruction
        ) -> FusableResult
        {
            return (block.size() < length) ? FusableResult::FUSABLE :
                FusableResult::NOT_FUSABLE;
        });
};

FusionRule FUSION_CORE_EXPORT independent(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
    {
        unordered_set<Operand> operands;
        for (auto const& instrPtr : block) {
            for (auto const& op : instrPtr->operands) {
                if (operands.contains(op)) return FusableResult::NOT_FUSABLE;
                operands.insert(op);
            }
        }
        return FusableResult::FUSABLE;
    }
);

FusionRule FUSION_CORE_EXPORT sameCount(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
    {
        if (block.size() > 0) {
            auto const& lastInstr = block.back();
            if (lastInstr->count != instruction.count) {
                return FusableResult::NOT_FUSABLE;
            }
        }
        return FusableResult::FUSABLE;
    }
);

}
