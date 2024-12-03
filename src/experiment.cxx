#include "experiment.h"
#include "fusion.h"
#include "instructions.h"
#include "macros.h"

namespace fusion
{

using namespace std;

Experiment::Experiment(
    vector<string> filepaths,
    vector<FusionConfig> const& configs
) : configs(configs)
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

vector<FusionResults> Experiment::run()
{
    FusionCalculator calculator;
    vector<FusionResults> results;

    auto run_experiment = [&](FusionConfig const& config) {
        LOG_INFO(config.title());
        for (auto const& file : files) {
            FusionResults result = calculator.calculateFusion(
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
                    result.avgFusionLength
                )
            );

            results.push_back(std::move(result));
        }
    };

    for (auto const& config : configs) {
        run_experiment(config);
    }

    return results;
}

} // namespace fusion
