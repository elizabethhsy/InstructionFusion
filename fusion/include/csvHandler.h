#pragma once

#include <fusion_export.h>

#include "dataRepresentation.h"
#include "fusion.h"

#include <vector>

namespace fusion
{

using namespace std;

struct FUSION_EXPORT CSVHandler
{
    static vector<Instr> loadInstructionsFromCSV(string fullPath);
    static void writeResultsToCSV(
        vector<FusionResults> const& results,
        vector<FusionResults> const& aggregate_results,
        string resultsPath
    );
};

} // namespace fusion
