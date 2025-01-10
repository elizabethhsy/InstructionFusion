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
// TODO: check with Jon whether this assumption is valid

const FusionRuleStruct FUSION_EXPORT arithmeticOnly(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction) {
        return (arithmeticInstructions.contains(instruction.instr)) ?
            FusableResult::FUSABLE : FusableResult::NOT_FUSABLE;
    }
);

const FusionRuleStruct FUSION_EXPORT arithmeticEndMemory(
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

const FusionRuleStruct FUSION_EXPORT arithmeticEndBranch(
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
FusionRuleStruct FUSION_EXPORT maxLength(int length) {
    return FusionRuleStruct(
        [length](
            vector<shared_ptr<Instr>> const& block,
            Instr const& instruction
        ) -> FusableResult
        {
            return (block.size() < length) ? FusableResult::FUSABLE :
                FusableResult::NOT_FUSABLE;
        });
};

FusionRuleStruct FUSION_EXPORT independentInstructions(
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

// FusableResult FUSION_EXPORT maxLength(
//     FusionRule rule,
//     vector<shared_ptr<Instr>> const& block,
//     Instr const& instruction,
//     uint64_t length
// )
// {
//     LOG_DEBUG(fmt::format("block size: {}", block.size()));
//     if (block.size() >= length) return FusableResult::NOT_FUSABLE;
//     return rule(block, instruction);
// }

// template<typename CurriedRule, typename... Args>
// FusionRule FUSION_EXPORT curriedRule(
//     CurriedRule rule1,
//     FusionRule rule2,
//     Args... args
// )
// {
//     return 
//         [rule1, rule2, ...args = std::forward<Args>(args)]
//         (vector<shared_ptr<Instr>> const& block, Instr const& instruction)
//             -> FusableResult
//         {
//             return rule1(rule2, block, instruction, args...);
//         };
// }

// TODO: figure out curried functions properly
// template <typename CurriedRule>
// FusableResult FUSION_EXPORT independentInstructions(
//     CurriedRule rule,
//     vector<shared_ptr<Instr>> const& block,
//     Instr const& instruction
// )
// {
//     unordered_set<Operand> operands;
//     for (auto const& instrPtr : block) {
//         for (auto const& op : instrPtr->operands) {
//             if (operands.contains(op)) return FusableResult::NOT_FUSABLE;
//             operands.insert(op);
//         }
//     }
//     return rule(block, instruction);
// }

}
