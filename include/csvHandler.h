#pragma once

#include "dataRepresentation.h"

#include <vector>

namespace fusion
{

using namespace std;

struct CSVHandler
{
    static vector<Instr> loadInstructionsFromCSV(string fullPath);
};

} // namespace fusion
