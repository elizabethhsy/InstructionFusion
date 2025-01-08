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
    vector<ExperimentRun> const& runs
) : runs(runs)
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

ExperimentResults Experiment::run()
{
    FusionCalculator calculator;
    vector<FusionResults> results;
    vector<FusionResults> aggregateResults;
    vector<FusionResults> aggregateTemp;
    
    auto run_experiment = [&](ExperimentRun const& run) {
        aggregateTemp.clear();
        LOG_INFO(run.title);
        for (auto const& file : files) {
            FusionResults result = calculator.calculateFusion(
                *file,
                run
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

            aggregateTemp.push_back(result);
            results.push_back(std::move(result));
        }

        // collect aggregate results across all files for each run
        FusionResults r{
            .file = *files[0],
            .run = run
        };

        for (auto const& res : aggregateTemp) {
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
        aggregateResults.push_back(r);
    };

    for (auto const& run : runs) {
        run_experiment(run);
    }

    return ExperimentResults{
        .results = results,
        .aggregateResults = aggregateResults
    };
}

void Experiment::save(ExperimentResults const& results, string resultsPath)
{
    CSVHandler::writeResultsToCSV(
        results.results,
        results.aggregateResults,
        resultsPath
    );
}

} // namespace fusion
