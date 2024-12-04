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

FusionResults FusionCalculator::calculateFusion(
    File const& file,
    FusionConfig const& config
)
{
    uint64_t instructionsAfterFuse = 0;
    float numBlocks = 0;
    vector<pair<uint, uint>> fusionLengths;

    auto const& fusable = config.fusable;
    auto const& end = config.end;

    auto maxFusableLength = config.maxFusableLength;
    if (!config.maxFusableLength) {
        maxFusableLength = UINT64_MAX;
    }

    // LOG_INFO(config.toString());

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
        };

        for (int i = 0; i < criticalSection->length; i++) {
            auto const& instruction = file.instructions[startIdx+i];

            // if the instruction is in the end block, record the end of the
            // fusable block to be after this instruction:
            // ------
            // fusable block
            // current instruction
            // ------
            if (count(end.begin(), end.end(), instruction.instr) != 0
                && blockLength < maxFusableLength)
            {
                blockLength++;
                record_end_of_block();
            }
            // if the instruction is not fusable, record the end of the fusable
            // block to be before this instruction:
            // ------
            // fusable block
            // ------
            // current instruction
            else if (
                count(fusable.begin(), fusable.end(), instruction.instr) == 0
                || blockLength >= maxFusableLength
            ) {
                record_end_of_block();
                blockLength++;
            }
            else {
                // if (blockLength)
                //     LOG_INFO(fmt::format("incrementing block_length from {}", blockLength));
                blockLength++;
            }
        }
        record_end_of_block();
    }

    // calculate average fusion length
    double avgLength = 0;
    uint64_t totalCount = 0;

    for (auto block : fusionLengths) {
        auto const& count = block.first;
        auto const& length = block.second;

        avgLength += count*length;
        totalCount += count;
    }
    avgLength = double(avgLength)/totalCount;

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
        .avgFusionLength = avgLength
    };

    return results;
}

}
