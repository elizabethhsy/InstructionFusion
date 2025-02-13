#include "experiment.h"

#include <boost/range/adaptor/transformed.hpp>

namespace fusion
{

using namespace std;

ExperimentManager::ExperimentManager(
    vector<string> filepaths,
    vector<ExperimentRun> const& runs,
    string resultsPath
) : runs(runs), resultsPath(resultsPath)
{
    // create the necessary file objects
    for (auto filepath : filepaths) {
        files.push_back(make_shared<File>(filepath));
    }

    // for each file, calculate the statistics and print them out
    for (auto const& file : files) {
        // print stats
        LOG_INFO(
            fmt::format(
                "stats for file {}: {}",
                file->fileName,
                file->stats->toString()
            )
        );
    }

    LOG_INFO(
        fmt::format(
            "Across all files, avg_critical_section_size_={}, "
            "total_instruction_num={}",
            avgCriticalSectionSize(),
            totalInstructionNum()
        )
    );
}

float ExperimentManager::avgCriticalSectionSize()
{
    float avgCriticalSectionSize = 0;

    for (auto const& file : files) { // file is a unique pointer
        avgCriticalSectionSize += file->stats->avgCriticalSectionSize;
    }

    avgCriticalSectionSize /= files.size();

    return avgCriticalSectionSize;
}

uint64_t ExperimentManager::totalInstructionNum()
{
    uint64_t totalInstructionNum = 0;

    for (auto const& file : files) { // file is a unique pointer
        totalInstructionNum += file->stats->totalInstructionNum;
    }

    return totalInstructionNum;
}

} // fusion
