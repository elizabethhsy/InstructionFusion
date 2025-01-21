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
    vector<ExperimentRunResults> runResults;
    
    auto run_experiment = [&](ExperimentRun const& run) {
        vector<FusionResults> results;
        // collect aggregate results across all files for each run
        FusionResults aggregateResults{
            .file = *files[0],
            .run = run
        };

        LOG_INFO(fmt::format("{} ({})", run.title, run.userDefinedKey));
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

            results.push_back(std::move(result));
        }

        for (auto const& res : results) {
            aggregateResults.totalInstructions += res.totalInstructions;
            aggregateResults.instructionsAfterFuse += res.instructionsAfterFuse;
            aggregateResults.fusedInstructions += res.fusedInstructions;
            aggregateResults.fusionLengths.insert(
                aggregateResults.fusionLengths.end(),
                res.fusionLengths.begin(),
                res.fusionLengths.end()
            );
        }
        aggregateResults.avgFusionLength =
            FusionCalculator::calcAvgFusionLength(
                aggregateResults.fusionLengths
            );
        aggregateResults.fusedPercentage =
            100*(aggregateResults.fusedInstructions)/
            double(aggregateResults.totalInstructions);
        
        // add run results to the overall experiment results vector
        runResults.push_back(ExperimentRunResults{
            .run = run,
            .fusionResults = results,
            .aggregateResults = aggregateResults
        });
    };

    for (auto const& run : runs) {
        run_experiment(run);
    }

    return ExperimentResults{
        .runResults = std::move(runResults)
    };
}

void Experiment::save(ExperimentResults const& results, string resultsPath)
{
    CSVHandler::writeResultsToCSV(
        results,
        resultsPath
    );
}

} // namespace fusion
