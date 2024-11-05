#include "dataRepresentation.h"
#include "macros.h"

#include <cassert>
#include <fmt/core.h>

namespace fusion
{

File::File()
{
    // find file with that name in the directory
    // if filePath is not specified, assume we're looking in the same directory
    // otherwise filePath should refer to the absolute path which the file is in
    
    // if the file doesn't exist, then we throw an exception

    // read CSV and populate the instruction vector
    calculateStats();
}

void File::calculateStats()
{
    // assume that the instructions are already populated
    assertm(
        !instructions.empty(),
        fmt::format(
            "the instructions from file {} have not been loaded",
            fileName
        )
    );

    stats.avgCriticalSectionSize = stats.getAvgCriticalSectionSize();
    stats.totalInstructionNum = stats.getTotalInstructionNum();
}

Experiment::Experiment(vector<string> filenames)
{
    // all the filenames should correspond to valid csvs in some path
}

}