#include "fusion.h"

#include <utility>

namespace fusion
{

using namespace std;

string FusionConfig::toString() const
{
    return fmt::format(
        "FusionConfig: fusable_name={}, end_name={}, max_fusable_length={}",
        fusableName,
        endName,
        maxFusableLength
    );
}

string FusionConfig::title() const
{
    return fmt::format(
        "{}{} (max {})",
        fusableName,
        endName.empty() ? "" : " + END " + endName,
        maxFusableLength
    );
}

string FusionResults::toString() const
{
    return fmt::format(
        "FusionResults: file={}, config=({}), total_instructions={}, "
        "instructions_after_fuse={}, fused_instructions={}, avg_fusion_length="
        "{}, fused_percentage={}",
        file.fileName,
        config.toString(),
        totalInstructions,
        instructionsAfterFuse,
        fusedInstructions,
        avgFusionLength,
        fusedPercentage
    );
}

FusionResults FusionCalculator::calculateFusion(
    File const& file,
    FusionConfig const& config
)
{
    uint64_t instructionsAfterFuse = 0;
    float numBlocks = 0;
    vector<pair<uint, uint>> fusionLengths;
    unordered_set<Operand> dependentOperands;

    auto const& fusable = config.fusable;
    auto const& end = config.end;

    auto maxFusableLength = config.maxFusableLength;
    if (!config.maxFusableLength) {
        maxFusableLength = UINT64_MAX;
    }

    for (auto const& criticalSection : file.stats->criticalSections) {
        const auto* startInstr = criticalSection->start;
        auto startIdx = distance(&file.instructions[0], startInstr);
        uint blockLength = 0;

        auto record_end_of_block = [&]()
        {
            if (blockLength == 0) return;

            instructionsAfterFuse += criticalSection->count;
            numBlocks += criticalSection->count;
            fusionLengths.push_back(
                make_pair(criticalSection->count, blockLength)
            );
            blockLength = 0;
            dependentOperands.clear();
        };

        LOG_DEBUG(
            fmt::format("critical_section_length={}", criticalSection->length)
        );

        for (int i = 0; i < criticalSection->length; i++) {
            auto const& instruction = file.instructions[startIdx+i];
            auto const& prevInstruction = file.instructions[startIdx + max(i-1, 0)];

            LOG_DEBUG(fmt::format("block_length={}", blockLength));

            // if the instruction is in the end block, record the end of the
            // fusable block to be after this instruction:
            // ------
            // fusable block
            // current instruction
            // ------
            if (count(end.begin(), end.end(), instruction.instr) != 0
                && blockLength < maxFusableLength
                && (!config.independentInstructionsOnly ||
                    !Instr::dependentOperands(instruction, dependentOperands)))
            {
                LOG_DEBUG(fmt::format("fusing {}", instruction.toString()));
                blockLength++;
                record_end_of_block();
            }
            // if the instruction is not fusable, record the end of the fusable
            // block to be before this instruction:
            // ------
            // fusable block
            // ------
            // current instruction
            // ------
            else if (
                count(fusable.begin(), fusable.end(), instruction.instr) == 0
                || blockLength >= maxFusableLength
                || (config.independentInstructionsOnly &&
                    Instr::dependentOperands(instruction, dependentOperands))
            ) {
                record_end_of_block();
                blockLength++;
                record_end_of_block();
            }
            else {
                LOG_DEBUG(fmt::format("fusing {}", instruction.toString()));
                dependentOperands.insert(
                    instruction.operands.begin(),
                    instruction.operands.end()
                );
                blockLength++;
            }
        }
        record_end_of_block();
    }

    auto totalInstructions = file.stats->totalInstructionNum;
    auto fusedInstructions = totalInstructions - instructionsAfterFuse;
    auto fusedPercentage = 100*(fusedInstructions)/double(totalInstructions);

    FusionResults results{
        .file = file,
        .config = config,
        .totalInstructions = totalInstructions,
        .instructionsAfterFuse = instructionsAfterFuse,
        .fusedInstructions = fusedInstructions,
        .fusedPercentage = fusedPercentage,
        .fusionLengths = fusionLengths,
        .avgFusionLength = calcAvgFusionLength(fusionLengths)
    };

    return results;
}

float FusionCalculator::calcAvgFusionLength(
    vector<pair<uint, uint>> const& fusionLengths
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

}
