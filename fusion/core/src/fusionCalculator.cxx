#include "fusionCalculator.h"
#include "instructionCount.h"

#include <utility>
#include <unordered_set>

#include <dataRepresentation.h>

#include <boost/describe/enum.hpp>
#include <boost/describe/enum_to_string.hpp>

namespace fusion
{

using namespace std;

NumPorts NumPorts::numPorts(vector<shared_ptr<Instr>> const& block)
{
    unordered_set<Operand> read_operands;
    unordered_set<Operand> write_operands;

    // add the read operands to the set
    for (auto instruction : block) {
        for (int i = 0; i < instruction->operands.size(); i++) {
            if (instruction->operands[i].isImmediate()) continue;
            if (i == 0) { // destination register, it's a write port
                write_operands.insert(instruction->operands[i]);
            } else {
                read_operands.insert(instruction->operands[i]);
            }
        }
    }

    return NumPorts{
        .read = read_operands.size(),
        .write = write_operands.size()
    };
}

string FusionResults::toString() const
{
    return fmt::format(
        "FusionResults: file={}, "
        "total_instructions={}, "
        "instructions_after_fuse={}, fused_instructions={}, avg_fusion_length="
        "{}, fused_percentage={}",
        file->fileName,
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
    uint64_t totalCycles = 0;

    for (auto block : fusionLengths) {
        auto const& count = block.first;
        auto const& length = block.second;

        avgLength += count*length;
        totalCycles += count;
    }
    avgLength = double(avgLength)/totalCycles;
    return avgLength;
}

FusionResults FusionCalculator::calculateFusion(
    shared_ptr<File> file,
    ExperimentRun const& run)
{
    LOG_DEBUG(
        fmt::format(
            "starting run: {}, {} fusion rule(s) found",
            run.title,
            run.rules.size()
        )
    );

    vector<FusedBlock> fusedBlocks;
    vector<shared_ptr<Instr>> currBlock;
    uint64_t dynamicCount = file->instructions[0].count;
    auto tempRules = run.rules;

    // calculate the number of read and write ports
    map<uint64_t, uint64_t> numReadPorts;
    map<uint64_t, uint64_t> numWritePorts;

    // keep track of results
    uint64_t instructionsAfterFuse = 0;
    vector<pair<uint64_t, uint64_t>> fusionLengths;
    bool startOfBlock = true;

    // record the results, and start a new round
    auto new_round = [&](uint64_t count) {
        tempRules = run.rules;
        startOfBlock = true;

        if (currBlock.size() == 0) return;

        // record the results
        auto const& first = currBlock[0];
        fusedBlocks.emplace_back(FusedBlock(
            first->label,
            first->addr,
            count,
            currBlock
        ));
        instructionsAfterFuse += count;
        fusionLengths.push_back(make_pair(count, currBlock.size()));

        for (auto instruction : currBlock) {
            auto numPorts = NumPorts::numPorts(currBlock);
            numReadPorts[numPorts.read] += count;
            numWritePorts[numPorts.write] += count;   
        }

        currBlock.clear();
    };

    // iterate through every instruction. For each instruction, we iterate over
    // all the functions that are still in contention.
    for (auto const& instruction : file->instructions) {
        bool endOfFusable = false;
        bool startOfFusable = false;

        if (startOfBlock) {
            dynamicCount = instruction.count;
            startOfBlock = false;
        }

        auto rules = tempRules;
        for (auto fun : rules) {
            auto result = fun->apply(currBlock, instruction);
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
                    tempRules.erase(fun);
                    break;
                }
                case FusableResult::START_OF_FUSABLE:
                {
                    startOfFusable = true;
                    tempRules.erase(fun);
                    break;
                }
                case FusableResult::NOT_FUSABLE:
                {
                    // remove the function from the set
                    tempRules.erase(fun);
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
        if (tempRules.empty()) {
            // there are no functions in the set, so we wrap up the round and start
            // a new one with a fresh copy of tempRules.
            if (endOfFusable) {
                currBlock.push_back(make_shared<Instr>(instruction));
                new_round(dynamicCount);
            } else if (startOfFusable) {
                new_round(dynamicCount);
                currBlock.push_back(make_shared<Instr>(instruction));
                dynamicCount = instruction.count;
                startOfBlock = false;
            } else { // NOT_FUSABLE
                new_round(dynamicCount);
                dynamicCount = instruction.count;
                currBlock.push_back(make_shared<Instr>(instruction));
                new_round(dynamicCount);
            }
        } else {
            dynamicCount = max(dynamicCount, instruction.count);
            currBlock.push_back(make_shared<Instr>(instruction));
        }
    }
    new_round(dynamicCount); // wrap up the final block

    // calculate stats
    auto totalInstructions = file->stats->totalInstructionNum;
    assert(totalInstructions >= instructionsAfterFuse);
    auto fusedInstructions = totalInstructions - instructionsAfterFuse;
    auto fusedPercentage = 100*(fusedInstructions)/double(totalInstructions);

    return FusionResults{
        .file = file,
        .run = run,
        .totalInstructions = totalInstructions,
        .instructionsAfterFuse = instructionsAfterFuse,
        .fusedInstructions = fusedInstructions,
        .fusedPercentage = fusedPercentage,
        .fusedBlocks = std::move(fusedBlocks),
        .fusionLengths = std::move(fusionLengths),
        .avgFusionLength = calcAvgFusionLength(fusionLengths),
        .numReadPorts = std::move(numReadPorts),
        .numWritePorts = std::move(numWritePorts)
    };
}

}
