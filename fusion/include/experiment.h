#pragma once

#include <fusion_export.h>

#include "dataRepresentation.h"
#include "fusion.h"

#include <string>
#include <vector>

namespace fusion
{

using namespace std;

// store the results of running a single experiment, with multiple fusion
// configs. A separate run is done for each fusion config, and the results
// are appended to the vectors.
struct FUSION_EXPORT ExperimentResults
{
    vector<FusionResults> results;
    vector<FusionResults> aggregateResults;
};

// struct FUSION_EXPORT ExperimentRunResults
// {
//     vector<FusionResults> results;
//     vector<FusionResults> aggregateResults;
// };

struct FUSION_EXPORT ExperimentRun
{
    string title;
    unordered_set<FusionRulePtr> rules;
    string description = "";
};

struct FUSION_EXPORT Experiment
{
public:
    // constructor - takes in list of file names and parses the corresponding
    // csv files
    Experiment(vector<string> filepaths, vector<ExperimentRun> const& runs);

    float avgCriticalSectionSize();
    uint64_t totalInstructionNum();
    ExperimentResults run();
    // writes results to CSV
    void save(ExperimentResults const& results, string resultsPath);
private:
    vector<unique_ptr<File>> files;
    vector<ExperimentRun> const& runs;
};

} // namespace fusion
