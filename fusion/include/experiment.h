#pragma once

#include <fusion_export.h>

#include "dataRepresentation.h"
#include "fusion.h"

#include <string>
#include <vector>

namespace fusion
{

using namespace std;

struct FUSION_EXPORT Experiment
{
public:
    // constructor - takes in list of file names and parses the corresponding
    // csv files
    Experiment(vector<string> filenames, vector<FusionConfig> const& configs);

    float avgCriticalSectionSize();
    uint64_t totalInstructionNum();
    void run(string resultsPath); // writes results to CSV
private:
    vector<FusionConfig> const& configs;
    vector<unique_ptr<File>> files;
};

} // namespace fusion
