#include "inOrderPipeline.h"

#include <instructionCount.h>
#include <instructions.h>

#include <unordered_set>

namespace fusion
{

PipelineRunResult InOrderPipeline::run(
    InstructionCountRunResults const& instrCountRunResult
)
{
    PipelineRunResult results;
    for (auto const& instrCountResult : instrCountRunResult.results) {
        auto result = computeCycleCount(instrCountResult);
        results.pipelineResults.push_back(result);
    }
    return results;
}

// take a basic block at a time, and see if there are any stalls. In a simple
// in-order pipeline (with good forwarding), a stall only occurs if there is a
// load-after-use.
// we assume each fused block takes a single cycle, other than blocks which
// contain a load instruction followed by blocks which contain an arithmetic
// instruction that uses the result of that load.
PipelineResult InOrderPipeline::computeCycleCount(
    FusionResults const& instructionCounts
)
{
    auto const& blocks = instructionCounts.fusedBlocks;
    PipelineResult results;
    results.totalInstructions = instructionCounts.totalInstructions;

    unordered_set<Operand> loadOperands;

    auto find_ld_operands = [&loadOperands](FusedBlock const& block) {
        for (auto const& instruction : block.instructions) {
            // check if it's a load instruction
            if (loadInstructions.contains(instruction->instr)) {
                // add all operands to the unordered set
                for (auto const& op : instruction->operands) {
                    loadOperands.insert(op);
                }
            }
        }
    };

    auto test_for_operands = [&loadOperands](FusedBlock const& block) -> bool {
        for (auto const& instruction : block.instructions) {
            if (!memoryInstructions.contains(instruction->instr)) {
                for (auto const& op : instruction->operands) {
                    if (loadOperands.contains(op)) return true;
                }
            }
        }
        return false;
    };

    for (auto const& block : blocks) {
        results.cyclesWithoutStalls += block.count;

        // test for shared operands in the block
        if (test_for_operands(block)) {
            results.stalls += block.count;
        }

        // add load operands for the next block to test from
        loadOperands.clear();
        find_ld_operands(block);
    }

    results.totalCycles = results.cyclesWithoutStalls + results.stalls;
    return results;
}

};
