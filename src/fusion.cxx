#include "fusion.h"

namespace fusion
{

using namespace std;

uint64_t FusionCalculator::calculateFusion(
    File const& file,
    vector<string> const& fusable
)
{
    uint64_t num = 0;

    for (auto const& criticalSection : file.stats->criticalSections) {
        const auto* startInstr = criticalSection->start;
        auto startIdx = distance(&file.instructions[0], startInstr);

        bool startOfFusable = true;

        for (int i = 0; i < criticalSection->length; i++) {
            auto const& instruction = file.instructions[startIdx+i];
            if (count(fusable.begin(), fusable.end(), instruction.instr) == 0) {
                // weighed by dynamic instruction count
                num += criticalSection->count;
                startOfFusable = true;
            } else if (startOfFusable) {
                num += criticalSection->count;
                startOfFusable = false;
            }
        }
    }

    return num;
}

}
