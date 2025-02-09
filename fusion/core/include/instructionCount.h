#pragma once

#include <fusion_core_export.h>

#include "fusionCalculator.h"

#include <dataRepresentation.h>
#include <experiment.h>

#include <string>
#include <vector>

namespace fusion
{

using namespace std;

struct FUSION_CORE_EXPORT InstructionCountRunResults
{
    ExperimentRun const run;
    vector<FusionResults> results;
    FusionResults aggregateResults;
};

// store the results of running a single experiment, with multiple fusion
// configs. A separate run is done for each fusion config, and the results
// are appended to the vectors.
struct FUSION_CORE_EXPORT InstructionCountResults
{
    vector<InstructionCountRunResults> runResults;
};

struct FUSION_CORE_EXPORT InstructionCountExperiment
{
    InstructionCountExperiment(shared_ptr<ExperimentManager> manager)
        : manager(manager) {};

    ExperimentResults<FusionResults> run();
    // writes results to CSV
    static void save(
        ExperimentResults<FusionResults> const& results,
        string resultsPath
    );
private:
    shared_ptr<ExperimentManager> manager;
};

} // namespace fusion
