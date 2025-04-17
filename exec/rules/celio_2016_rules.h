#pragma once

#include <fileHandler.h>
#include <exampleRules.h>
#include <experiment.h>
#include <fusion.h>
#include <instructions.h>
#include <macros.h>

#include <string>
#include <unordered_set>
#include <vector>

namespace celio_2016_rules
{

using namespace fusion;
using namespace std;

// slli rd, rs1, {1,2,3}
// add  rd, rd, rs2 --> cincoffset
const FusionRule loadEffectiveAddress(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction) {
        // match slli instruction
        if (instruction.instr == "slli") {
            auto const& ops = instruction.operands;
            unordered_set<string> possibleValues = { "1", "2", "3" };
            if (ops.size() == 3 && possibleValues.contains(ops[2].op)) {
                return FusableResult::START_OF_FUSABLE;
            }
        }

        // match c.slli instruction
        if (instruction.instr == "c.slli") {
            auto const& ops = instruction.operands;
            unordered_set<string> possibleValues = { "1", "2", "3" };
            if (ops.size() == 2 && possibleValues.contains(ops[1].op)) {
                return FusableResult::START_OF_FUSABLE;
            }
        }

        // match add instruction
        if (block.size() == 1 && instruction.instr == "add") {
            auto const& slli = block.back();
            auto const& ops = instruction.operands;
            if (ops.size() == 3 && ops[0] == ops[1] &&
                ops[0] == slli->operands[0])
            {
                return FusableResult::END_OF_FUSABLE;
            }
        }

        // match c.add instruction
        if (block.size() == 1 && instruction.instr == "c.add") {
            auto const& slli = block.back();
            auto const& ops = instruction.operands;
            if (ops.size() == 2 && ops[0] == slli->operands[0])
            {
                return FusableResult::END_OF_FUSABLE;
            }
        }

        // match addi instruction
        if (block.size() == 1 && instruction.instr == "addi") {
            auto const& slli = block.back();
            auto const& ops = instruction.operands;
            if (ops.size() == 3 && ops[0] == ops[1] &&
                ops[0] == slli->operands[0])
            {
                return FusableResult::END_OF_FUSABLE;
            }
        }

        // match cincoffset instruction
        if (block.size() == 1 && instruction.instr == "cincoffset") {
            auto const& slli = block.back();
            auto const& ops = instruction.operands;
            if (ops.size() == 3 && ops[0] == ops[1] &&
                ops[2] == slli->operands[0])
            {
                return FusableResult::END_OF_FUSABLE;
            }
        }
        
        return FusableResult::NOT_FUSABLE;
    }
);

// add  rd, rs1, rs2    --> cincoffset  ca1 cs0 a1
// ld   rd, 0(rd)       --> cld         a2 0(ca1)
const FusionRule indexedLoad(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction) {
        // match cincoffset instruction
        if (instruction.instr == "cincoffset" &&
            instruction.operands.size() == 3)
        {
            return FusableResult::START_OF_FUSABLE;
        }

        auto zero_offset = [&](Operand op1, Operand op2) -> bool {
            return op1.op == fmt::format("0({})", op2.op) ||
                op1.op == fmt::format("0(c{})", op2.op); // capability register
        };
        
        // match cld instruction
        if (block.size() == 1 && loadInstructions.contains(instruction.instr)) {
            auto const& cincoffset = block.back();
            auto const& ops = instruction.operands;
            if (ops.size() == 2 && zero_offset(ops[1], ops[0])) {
                if (zero_offset(ops[1], cincoffset->operands[0]))
                {
                    return FusableResult::END_OF_FUSABLE;
                }
            }
        }

        return FusableResult::NOT_FUSABLE;
    }
);

// combine loadEffectiveAddress and fusedIndexedLoad
const FusionRule fusedIndexedLoad(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
        -> FusableResult
    {
        auto leaResult = loadEffectiveAddress.apply(block, instruction);
        if (leaResult == FusableResult::START_OF_FUSABLE) return leaResult;

        if (block.size() >= 1) {
            auto leaResult = loadEffectiveAddress.apply(block, instruction);
            vector<shared_ptr<Instr>> subBlock(block.begin() + 1, block.end());
            auto idxLdResult = indexedLoad.apply(subBlock, instruction);
            return max(leaResult, idxLdResult);
        }

        return FusableResult::NOT_FUSABLE;
    }
);

// slli rd, rs1, 32
// srli rd, rd, {29, 30, 31, 32}
const FusionRule clearUpperWord(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction) {
        // match slli instruction
        if (instruction.instr == "slli") {
            auto const& ops = instruction.operands;
            if (ops.size() == 3 && ops[2].op == "32") {
                return FusableResult::START_OF_FUSABLE;
            }
        }

        // match slli instruction
        if (instruction.instr == "c.slli" && instruction.operands.size() == 2) {
            return FusableResult::START_OF_FUSABLE;
        }

        unordered_set<string> possibleValues = { "29", "30", "31", "32" };
        // match srli instruction
        if (block.size() == 1 && instruction.instr == "srli") {
            auto const& slli = block.back();
            auto const& ops = instruction.operands;
            if (ops.size() == 3 && //ops[0] == ops[1] &&
                possibleValues.contains(ops[2].op) &&
                ops[1] == slli->operands[0])
            {
                return FusableResult::END_OF_FUSABLE;
            }
        }

        // match c.srli instruction
        if (block.size() == 1 && instruction.instr == "c.srli") {
            auto const& slli = block.back();
            auto const& ops = instruction.operands;
            if (ops.size() == 2 && possibleValues.contains(ops[1].op) &&
                ops[0] == slli->operands[0])
            {
                return FusableResult::END_OF_FUSABLE;
            }
        }

        return FusableResult::NOT_FUSABLE;
    }
);

// auipcc   ca4, 321
// clc      ca4, 882(ca4)
const FusionRule loadGlobal(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction) {
        // match auipcc instruction
        if (instruction.instr == "auipcc" &&
            instruction.operands.size() == 2)
        {
            return FusableResult::START_OF_FUSABLE;
        }

        // match clc instruction
        if (block.size() == 1 && instruction.instr == "clc") {
            auto const& auipcc = block.back();
            auto const& ops = instruction.operands;
            if (ops.size() == 2 && ops[1].op.contains(ops[0].op) &&
                ops[0] == auipcc->operands[0])
            {
                return FusableResult::END_OF_FUSABLE;
            }
        }

        return FusableResult::NOT_FUSABLE;
    }
);

std::vector<fusion::ExperimentRun> baseRuns = {
    fusion::ExperimentRun{
        .title = "load effective address",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::sameCount.chain(loadEffectiveAddress)
            )
        }
    },
    fusion::ExperimentRun{
        .title = "indexed load",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::sameCount.chain(indexedLoad)
            )
        }
    },
    fusion::ExperimentRun{
        .title = "fused indexed load",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::sameCount.chain(fusedIndexedLoad)
            )
        }
    },
    fusion::ExperimentRun{
        .title = "clear upper word",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::sameCount.chain(clearUpperWord)
            )
        }
    },
    fusion::ExperimentRun{
        .title = "load global",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::sameCount.chain(loadGlobal)
            )
        }
    },
    fusion::ExperimentRun{
        .title = "overall results",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::sameCount.chain(loadEffectiveAddress)
            ),
            std::make_shared<fusion::FusionRule>(
                fusion::sameCount.chain(indexedLoad)
            ),
            // std::make_shared<fusion::FusionRule>(
            //     fusion::sameCount.chain(fusedIndexedLoad)
            // ),
            std::make_shared<fusion::FusionRule>(
                fusion::sameCount.chain(clearUpperWord)
            )
            // std::make_shared<fusion::FusionRule>(
            //     fusion::sameCount.chain(loadGlobal)
            // )
        }
    }
};

} // celio_2016
