#include "fusion.h"

#include <utility>

namespace fusion
{

using namespace std;

FusionStats FusionCalculator::calculateFusion(
    File const& file,
    FusionConfig const& config
)
{
    uint64_t instructionsAfterFuse = 0;
    float numBlocks = 0;
    vector<pair<uint, uint>> fusionLengths;

    auto const& fusable = config.fusable;
    auto const& end = config.end;

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
            if (count(end.begin(), end.end(), instruction.instr) != 0)
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
                || blockLength >= config.maxFusableLength
            ) {
                record_end_of_block();
                blockLength++;
            }
            else {
                blockLength++;
            }
        }
        record_end_of_block();
    }

    FusionStats stats{
        .totalInstructions = file.stats->totalInstructionNum,
        .instructionsAfterFuse = instructionsAfterFuse,
        .fusionLengths = fusionLengths
    };

    return stats;
}

float FusionStats::getAvgFusionLength()
{
    // iterate through the vector of fusion lengths + count
    // and calculate the average, weighted by the number of times
    // each fusable block has been executed
    float avgLength = 0;
    uint totalCount = 0;

    for (auto block : fusionLengths) {
        auto const& count = block.first;
        auto const& length = block.second;

        avgLength += count*length;
        totalCount += count;
    }

    avgLength /= totalCount;
    return avgLength;
}

}
