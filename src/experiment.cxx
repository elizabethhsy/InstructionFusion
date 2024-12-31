#include "csvHandler.h"
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

void Experiment::run(string resultsPath)
{
    FusionCalculator calculator;
    vector<FusionResults> results;
    vector<FusionResults> aggregate_results;
    vector<FusionResults> aggregate_temp;

    auto run_experiment = [&](FusionConfig const& config) {
        aggregate_temp.clear();
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

            aggregate_temp.push_back(result);
            results.push_back(std::move(result));
        }

        // collect aggregate results across all files for each config
        FusionResults r{
            .file = *files[0],
            .config = config
        };
        // r.config = config;
        for (auto const& res : aggregate_temp) {
            r.totalInstructions += res.totalInstructions;
            r.instructionsAfterFuse += res.instructionsAfterFuse;
            r.fusedInstructions += res.fusedInstructions;
            r.fusionLengths.insert(
                r.fusionLengths.end(),
                res.fusionLengths.begin(),
                res.fusionLengths.end()
            );
        }
        r.avgFusionLength =
            FusionCalculator::calcAvgFusionLength(r.fusionLengths);
        r.fusedPercentage =
            100*(r.fusedInstructions)/double(r.totalInstructions);
        aggregate_results.push_back(r);
    };

    for (auto const& config : configs) {
        run_experiment(config);
    }

    CSVHandler::writeResultsToCSV(
        results,
        aggregate_results,
        resultsPath
    );
}

} // namespace fusion
