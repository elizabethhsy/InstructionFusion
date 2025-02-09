#include "pipelineSimulator.h"

namespace fusion
{

string PipelineResult::toString() const
{
    return fmt::format(
        "Pipeline result: count_without_stalls={}, stalls={}, total_count={}, "
        "cycle_percentage={}",
        cyclesWithoutStalls,
        stalls,
        totalCycles,
        cyclePercentage()
    );
}

double PipelineResult::cyclePercentage() const
{
    return 100*(totalCycles)/double(totalInstructions);
}

} // fusion
