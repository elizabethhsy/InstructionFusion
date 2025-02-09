#include "histogramParser.h"
#include "pipelineSimulator.h"

#include <dataRepresentation.h>
#include <experiment.h>
#include <instructionCount.h>

#include <stdint.h>

namespace fusion
{

using namespace std;

struct InOrderPipeline
{
    PipelineRunResult run(InstructionCountRunResults const& instrCountResult);
    PipelineResult computeCycleCount(FusionResults const& instructionCounts);
};

} // fusion
