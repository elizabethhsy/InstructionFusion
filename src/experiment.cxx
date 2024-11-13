#include "experiment.h"
#include "fusion.h"
#include "instructions.h"
#include "macros.h"

namespace fusion
{

using namespace std;

Experiment::Experiment(vector<string> filepaths)
{
    // create the necessary file objects
    for (auto filepath : filepaths) {
        files.push_back(make_unique<File>(filepath));
    }

    // for each file, calculate the statistics and print them out
    for (auto const& file : files) {
        file->stats->calculateStats();

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

float Experiment::avgCriticalSectionSize()
{
    float avgCriticalSectionSize = 0;

    for (auto const& file : files) { // file is a unique pointer
        avgCriticalSectionSize += file->stats->avgCriticalSectionSize;
    }

    avgCriticalSectionSize /= size(files);

    return avgCriticalSectionSize;
}

uint64_t Experiment::totalInstructionNum()
{
    uint64_t totalInstructionNum = 0;

    for (auto const& file : files) { // file is a unique pointer
        totalInstructionNum += file->stats->totalInstructionNum;
    }

    return totalInstructionNum;
}

void Experiment::run()
{
    FusionCalculator calculator;

    auto run_experiment = [&](string title, vector<string> fusable) {
        LOG_INFO(title);
        for (auto const& file : files) {
            uint64_t result = calculator.calculateFusion(
                *file,
                fusable
            );
            uint64_t total = file->stats->totalInstructionNum;
            LOG_INFO(
                fmt::format(
                    "file {}: number of instructions changed from {} to {}, "
                    "difference={}, percentage={}%",
                    file->fileName,
                    file->stats->totalInstructionNum,
                    result,
                    total-result,
                    100*(total-result)/float(total)
                )
            );
        }
    };

    vector<string> arithmetic_memory;
    arithmetic_memory.reserve(
        arithmeticInstructions.size() + memoryInstructions.size()
    );
    arithmetic_memory.insert(
        arithmetic_memory.end(),
        arithmeticInstructions.begin(),
        arithmeticInstructions.end()
    );
    arithmetic_memory.insert(
        arithmetic_memory.end(),
        memoryInstructions.begin(),
        memoryInstructions.end()
    );
    run_experiment("ARITHMETIC + MEMORY INSTRUCTIONS", arithmetic_memory);

    run_experiment("ARITHMETIC INSTRUCTIONS", arithmeticInstructions);

    run_experiment("MEMORY INSTRUCTIONS", memoryInstructions);

    vector<string> arithmetic_branch;
    arithmetic_branch.reserve(
        arithmeticInstructions.size() + branchInstructions.size()
    );
    arithmetic_branch.insert(
        arithmetic_branch.end(),
        arithmeticInstructions.begin(),
        arithmeticInstructions.end()
    );
    arithmetic_branch.insert(
        arithmetic_branch.end(),
        branchInstructions.begin(),
        branchInstructions.end()
    );
    run_experiment("ARITHMETIC + BRANCH INSTRUCTIONS", arithmetic_branch);
}

} // namespace fusion
