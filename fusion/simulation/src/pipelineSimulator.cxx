#include "pipelineSimulator.h"

namespace fusion
{

string BaselineResult::toString() const
{
    return fmt::format(
        "Baseline result: count_without_stalls={}, stalls={}, total_count={}",
        cyclesWithoutStalls,
        stalls,
        totalCycles
    );
}

string PipelineResult::toString() const
{
    return fmt::format(
        "Pipeline result: baseline=[{}], count_without_stalls={}, stalls={}, "
        "total_count={}, cycle_percentage={}",
        baseline->toString(),
        cyclesWithoutStalls,
        stalls,
        totalCycles,
        cyclePercentage()
    );
}

void PipelineResult::addBaseline(shared_ptr<BaselineResult> baseline)
{
    this->baseline = baseline;
}

double PipelineResult::cyclePercentage() const
{
    assert(baseline);
    return 100-(100*(totalCycles)/double(baseline->totalCycles));
}

} // fusion
