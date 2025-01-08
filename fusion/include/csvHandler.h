#pragma once

#include <fusion_export.h>

#include "dataRepresentation.h"
#include "experiment.h"
#include "fusion.h"

#include <vector>

namespace fusion
{

using namespace std;

struct FUSION_EXPORT CSVHandler
{
    static vector<Instr> loadInstructionsFromCSV(string fullPath);
    static void writeResultsToCSV(
        ExperimentResults const& results,
        string resultsPath
    );
};

} // namespace fusion
