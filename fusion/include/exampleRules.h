#include "fusion.h"
#include "instructions.h"

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
// TODO: check with Jon whether this assumption is valid

FusableResult FUSION_EXPORT arithmeticOnly(
    vector<shared_ptr<Instr>> const& block, Instr const& instruction
)
{
    return (arithmeticInstructions.contains(instruction.instr)) ?
        FusableResult::FUSABLE : FusableResult::NOT_FUSABLE;
}

FusableResult FUSION_EXPORT arithmeticEndMemory(
    vector<shared_ptr<Instr>> const& block, Instr const& instruction
)
{
    if (arithmeticInstructions.contains(instruction.instr)) {
        return FusableResult::FUSABLE;
    }
    if (memoryInstructions.contains(instruction.instr)) {
        return FusableResult::END_OF_FUSABLE;
    }
    return FusableResult::NOT_FUSABLE;
}

FusableResult FUSION_EXPORT arithmeticEndBranch(
    vector<shared_ptr<Instr>> const& block, Instr const& instruction
)
{
    if (arithmeticInstructions.contains(instruction.instr)) {
        return FusableResult::FUSABLE;
    }
    if (branchInstructions.contains(instruction.instr)) {
        return FusableResult::END_OF_FUSABLE;
    }
    return FusableResult::NOT_FUSABLE;
}

// first check if the maximum length of a fusable block is exceeded, then call
// another fusion rule
FusableResult FUSION_EXPORT maxLength(
    uint64_t length,
    FusionRulePtr rule,
    vector<shared_ptr<Instr>> const& block,
    Instr const& instruction
)
{
    if (block.size() >= length) return FusableResult::NOT_FUSABLE;
    return (*rule)(block, instruction);
}

}
