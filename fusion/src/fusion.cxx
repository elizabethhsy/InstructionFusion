#include "experiment.h"
#include "fusion.h"

#include <utility>
#include <unordered_set>

#include <boost/describe/enum.hpp>
#include <boost/describe/enum_to_string.hpp>

namespace fusion
{

using namespace std;

string FusionResults::toString() const
{
    return fmt::format(
        "FusionResults: file={}, "
        // "config=({}), "
        "total_instructions={}, "
        "instructions_after_fuse={}, fused_instructions={}, avg_fusion_length="
        "{}, fused_percentage={}",
        file.fileName,
        // config.toString(),
        totalInstructions,
        instructionsAfterFuse,
        fusedInstructions,
        avgFusionLength,
        fusedPercentage
    );
}

float FusionCalculator::calcAvgFusionLength(
    vector<pair<uint64_t, uint64_t>> const& fusionLengths
)
{
    double avgLength = 0;
    uint64_t totalCount = 0;

    for (auto block : fusionLengths) {
        auto const& count = block.first;
        auto const& length = block.second;

        avgLength += count*length;
        totalCount += count;
    }
    avgLength = double(avgLength)/totalCount;
    return avgLength;
}

FusionResults FusionCalculator::calculateFusion(
    File const& file,
    ExperimentRun const& run)
{
    LOG_DEBUG(
        fmt::format(
            "starting run: {}, {} fusion rule(s) found",
            run.title,
            run.rules.size()
        )
    );

    vector<shared_ptr<Instr>> currBlock;
    auto const& rulePtrs = run.rules;
    uint64_t dynamicCount = 0;
    auto functionPtrs = rulePtrs;

    // keep track of results
    uint64_t instructionsAfterFuse = 0;
    vector<pair<uint64_t, uint64_t>> fusionLengths;

    // record the results, and start a new round
    auto new_round = [&](uint64_t count) {
        if (currBlock.size() == 0) return;
        
        // record the results
        instructionsAfterFuse += count;
        fusionLengths.push_back(make_pair(count, currBlock.size()));

        // reset the tracking variables
        currBlock.clear();
        functionPtrs = rulePtrs;
    };

    // iterate through every instruction. For each instruction, we iterate over
    // all the functions that are still in contention.
    for (auto const& instruction : file.instructions) {
        bool endOfFusable = false;

        // if we're starting a new critical section, then reset the round
        if (instruction.count != dynamicCount) {
            new_round(dynamicCount);
            dynamicCount = instruction.count;
        }

        for (auto fun : functionPtrs) {
            auto result = (*fun)(currBlock, instruction);
            LOG_DEBUG(
                fmt::format(
                    "FusableResult type of {}",
                    boost::describe::enum_to_string(result, "unknown")
                )
            );
            switch (result) {
                case FusableResult::FUSABLE:
                {
                    // leave the function in the set
                    break;
                }
                case FusableResult::END_OF_FUSABLE:
                {
                    // remove the function from the set, but also set a flag
                    endOfFusable = true;
                    functionPtrs.erase(fun);
                    break;
                }
                case FusableResult::NOT_FUSABLE:
                {
                    // remove the function from the set
                    functionPtrs.erase(fun);
                    break;
                }
                default:
                {
                    LOG_ERROR(
                        fmt::format(
                            "Invalid FusableResult type of {}",
                            boost::describe::enum_to_string(result, "unknown")
                        )
                    );
                }
            }
        }

        // if there are no functions left in the set, then this is the
        // biggest block that can be fused and we wrap it up. Otherwise,
        // we continue with the next instruction.
        if (functionPtrs.empty()) {
            // there are no functions in the set, so we wrap up the round and start
            // a new one with a fresh copy of functionPtrs.
            if (endOfFusable) {
                currBlock.push_back(make_shared<Instr>(instruction));
                new_round(dynamicCount);
            } else {
                new_round(dynamicCount);
                dynamicCount = instruction.count;
                currBlock.push_back(make_shared<Instr>(instruction));
                new_round(dynamicCount);
            }
        } else {
            currBlock.push_back(make_shared<Instr>(instruction));
        }
    }
    new_round(dynamicCount); // wrap up the final block

    // calculate stats
    auto totalInstructions = file.stats->totalInstructionNum;
    auto fusedInstructions = totalInstructions - instructionsAfterFuse;
    auto fusedPercentage = 100*(fusedInstructions)/double(totalInstructions);

    return FusionResults{
        .file = file,
        .run = run,
        .totalInstructions = totalInstructions,
        .instructionsAfterFuse = instructionsAfterFuse,
        .fusedInstructions = fusedInstructions,
        .fusedPercentage = fusedPercentage,
        .fusionLengths = fusionLengths,
        .avgFusionLength = calcAvgFusionLength(fusionLengths)
    };
}

}
