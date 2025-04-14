#include "experiment.h"
#include "fileHandler.h"

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

void ExperimentManager::saveConfig()
{
    // save a file specifying the config of the experiment within the
    // experiment results folder
    FileWriter configWriter(resultsPath + "/config.txt");
    configWriter.writeLine("FILES:");

    // output all the file names that were passed in
    for (auto file : files) {
        configWriter.writeLine(fmt::format("\t{}", file->fileName));
    }

    configWriter.writeLine("\nRUNS:");
    // output the name and description of all the rules that were used
    // for each run
    for (auto run : runs) {
        configWriter.writeLine(
            fmt::format(
                "\ttitle: {}\n"
                "\t\tdescription: {}\n"
                "\t\tuser defined key: {}",
                run.title,
                run.description,
                run.userDefinedKey
            )
        );
    }
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
