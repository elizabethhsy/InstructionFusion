#pragma once

#include "dataRepresentation.h"
#include "fusion.h"

#include <vector>

namespace fusion
{

using namespace std;

struct CSVHandler
{
    static vector<Instr> loadInstructionsFromCSV(string fullPath);
    static void writeResultsToCSV(
        vector<FusionResults> const& results,
        vector<FusionResults> const& aggregate_results,
        string aggregateCSV,
        string overviewCSV,
        string fusionLengthsCSV
    );
};

} // namespace fusion
