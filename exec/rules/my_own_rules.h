#pragma once

#include <exampleRules.h>
#include <experiment.h>
#include <instructions.h>
#include <fusion.h>

#include <cstdlib>
#include <fmt/core.h>
#include <string>
#include <unordered_set>
#include <vector>

namespace my_rules
{

using namespace fusion;
using namespace std;

const FusionRule alwaysFusable(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction) {
        return FusableResult::FUSABLE;
    }
);

// enables fusion across branches etc.
const FusionRule similarCount(float proportion)
{
    return FusionRule(
        [proportion](vector<shared_ptr<Instr>> const& block, Instr const& instruction) {
            if (block.size() > 0) {
                auto const& lastInstr = block.back();
                auto blockCount = lastInstr->count;
                auto instrCount = instruction.count;
                if (abs(float(blockCount)-instrCount)/max(blockCount, instrCount)
                    > proportion)
                {
                    return FusableResult::START_OF_FUSABLE;
                }
            }
            return FusableResult::FUSABLE;
        }
    );
}

// allow fusion across branches as well. If the branch instruction doesn't
// affect the count, then ignore it and return fusable. If it does, then
// fuse as normal.
const FusionRule acrossBranches(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
        -> FusableResult
    {
        if (branchInstructions.contains(instruction.instr)) {
            return FusableResult::FUSABLE;
        }
        return sameCount.chain(arithmeticEndMemory).apply(block, instruction);
    }
);

const FusionRule contiguousMemory(
    [](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
        -> FusableResult
    {
        auto is_contiguous = [&](Instr const& prev, Instr const& next)
        {
            if (!loadInstructions.contains(prev.instr) ||
                !loadInstructions.contains(next.instr))
            {
                return false;
            }
            // TODO: implement logic
            return true;
        };

        if (is_contiguous(*block.back(), instruction)) {
            return FusableResult::FUSABLE;
        }
        return FusableResult::START_OF_FUSABLE;
    }
);

vector<ExperimentRun> baseRuns = {
    // ExperimentRun{
    //     .title = "arithmetic only",
    //     .userDefinedKey = "0",
    //     .rules = unordered_set<FusionRulePtr>{
    //         make_shared<FusionRule>(
    //             sameCount.chain(arithmeticOnly)
    //         )
    //     }
    // },
    // ExperimentRun{
    //     .title = "arithmetic end memory",
    //     .userDefinedKey = "0",
    //     .rules = unordered_set<FusionRulePtr>{
    //         make_shared<FusionRule>(
    //             sameCount.chain(arithmeticEndMemory)
    //         )
    //     }
    // },
    // ExperimentRun{
    //     .title = "arithmetic end branch",
    //     .userDefinedKey = "0",
    //     .rules = unordered_set<FusionRulePtr>{
    //         make_shared<FusionRule>(
    //             sameCount.chain(arithmeticEndBranch)
    //         )
    //     }
    // },
    ExperimentRun{
        .title = "arithmetic end memory/branch",
        .userDefinedKey = "0",
        .rules = unordered_set<FusionRulePtr>{
            make_shared<FusionRule>(
                sameCount.chain(arithmeticEndMemory)
            ),
            make_shared<FusionRule>(
                sameCount.chain(arithmeticEndBranch)
            )
        }
    }
    // ExperimentRun{
    //     .title = "arithmetic only (I)",
    //     .userDefinedKey = "0",
    //     .rules = unordered_set<FusionRulePtr>{
    //         make_shared<FusionRule>(
    //             independent
    //                 .chain(sameCount)
    //                 .chain(arithmeticOnly)
    //         )
    //     }
    // },
    // ExperimentRun{
    //     .title = "arithmetic end memory (I)",
    //     .userDefinedKey = "0",
    //     .rules = unordered_set<FusionRulePtr>{
    //         make_shared<FusionRule>(
    //             independent
    //                 .chain(sameCount)
    //                 .chain(arithmeticEndMemory)
    //         )
    //     }
    // },
    // ExperimentRun{
    //     .title = "arithmetic end branch (I)",
    //     .userDefinedKey = "0",
    //     .rules = unordered_set<FusionRulePtr>{
    //         make_shared<FusionRule>(
    //             independent
    //                 .chain(sameCount)
    //                 .chain(arithmeticEndBranch)
    //         )
    //     }
    // },
    // ExperimentRun{
    //     .title = "arithmetic end memory/branch (I)",
    //     .userDefinedKey = "0",
    //     .rules = unordered_set<FusionRulePtr>{
    //         make_shared<FusionRule>(
    //             independent
    //                 .chain(sameCount)
    //                 .chain(arithmeticEndMemory)
    //         ),
    //         make_shared<FusionRule>(
    //             independent
    //                 .chain(sameCount)
    //                 .chain(arithmeticEndBranch)
    //         )
    //     }
    // }
};

} // my_rules
