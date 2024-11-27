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

    auto run_experiment = [&](string title, FusionConfig config) {
        LOG_INFO(title);
        for (auto const& file : files) {
            FusionStats result = calculator.calculateFusion(
                *file,
                config
            );

            auto const& totalBefore = result.totalInstructions;
            auto const& totalAfter = result.instructionsAfterFuse;

            LOG_INFO(
                fmt::format(
                    "file {}: number of instructions changed from {} to {}, "
                    "difference={}, percentage={}%, avg_fusion_length={}",
                    file->fileName,
                    totalBefore,
                    totalAfter,
                    totalBefore-totalAfter,
                    100*(totalBefore-totalAfter)/float(totalBefore),
                    result.getAvgFusionLength()
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

    run_experiment(
        "ARITHMETIC + MEMORY INSTRUCTIONS",
        FusionConfig{ .fusable = arithmetic_memory }
    );

    run_experiment(
        "ARITHMETIC INSTRUCTIONS",
        FusionConfig{ .fusable = arithmeticInstructions }
    );

    run_experiment(
        "MEMORY INSTRUCTIONS",
        FusionConfig{ .fusable = memoryInstructions }
    );

    run_experiment(
        "ARITHMETIC + BRANCH INSTRUCTIONS",
        FusionConfig{ .fusable = arithmetic_branch }
    );

    run_experiment(
        "ARITHMETIC + END MEMORY",
        FusionConfig{
            .fusable = arithmeticInstructions,
            .end = memoryInstructions
        }
    );

    run_experiment(
        "ARITHMETIC + END BRANCH",
        FusionConfig{
            .fusable = arithmeticInstructions,
            .end = branchInstructions
        }
    );
}

} // namespace fusion
