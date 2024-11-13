# pragma once

#include "dataRepresentation.h"

#include <string>
#include <vector>

namespace fusion
{

struct Experiment
{
public:
    // constructor - takes in list of file names and parses the corresponding
    // csv files
    Experiment(vector<string> filenames);

    float avgCriticalSectionSize();
    uint64_t totalInstructionNum();
    void run();
private:
    vector<unique_ptr<File>> files;
};

} // namespace fusion
