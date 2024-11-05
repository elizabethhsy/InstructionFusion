#include "dataRepresentation.h"
#include "macros.h"

#include <cassert>
#include <fmt/core.h>

namespace fusion
{

void File::calculateStats()
{
    // assume that the instructions are already populated
    assertm(!instructions.empty(), "the instructions from file");
}

Experiment::Experiment(vector<string> filenames)
{
    // all the filenames should correspond to valid csvs in some path
}

}