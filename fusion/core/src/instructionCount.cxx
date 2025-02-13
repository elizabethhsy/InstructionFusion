#include "instructionCount.h"
#include "fusion.h"

#include <csvHandler.h>
#include <instructions.h>
#include <macros.h>

namespace fusion
{

using namespace std;

ExperimentResults<FusionResults> InstructionCountExperiment::run()
{
    FusionCalculator calculator;
    vector<RunResult<FusionResults>> runResults;
    
    auto run_experiment = [&](ExperimentRun const& run) {
        vector<FusionResults> results;
        // collect aggregate results across all files for each run
        FusionResults aggregateResults{
            .file = (manager->files)[0],
            .run = run
        };

        LOG_INFO(fmt::format("{} ({})", run.title, run.userDefinedKey));
        for (auto const& file : manager->files) {
            FusionResults result = calculator.calculateFusion(
                file,
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
        runResults.push_back(
            RunResult<FusionResults>(
                run,
                std::move(results),
                std::move(aggregateResults)
            )
        );
    };

    for (auto const& run : manager->runs) {
        run_experiment(run);
    }

    return ExperimentResults<FusionResults>{
        .runResults = std::move(runResults)
    };
}

void InstructionCountExperiment::save(
    ExperimentResults<FusionResults> const& results
)
{
    auto resultsPath = manager->resultsPath;
    CSVWriter aggregateWriter(resultsPath + "/aggregate.csv");
    CSVWriter overviewWriter(resultsPath + "/overview.csv");
    CSVWriter fusionLengthsWriter(resultsPath + "/fusionLengths.csv");

    aggregateWriter.writeLine("rule_title,rule_description,user_defined_key,"
        "total_instructions,instructions_after_fuse,instructions_fused,"
        "percentage_fused,average_fusion_length");
    overviewWriter.writeLine("rule_title,rule_description,user_defined_key,"
        "file,total_instructions,instructions_after_fuse,instructions_fused,"
        "percentage_fused,average_fusion_length");
    fusionLengthsWriter.writeLine("rule_title,rule_description,"
        "user_defined_key,file,count,fusion_length");
    
    for (auto const& run : results.runResults) {
        for (auto const& res : run.results) {
            overviewWriter.writeLine(
                fmt::format(
                    "{},{},{},{},{},{},{},{},{}",
                    res.run.title,
                    res.run.description,
                    res.run.userDefinedKey,
                    res.file->fileName,
                    res.totalInstructions,
                    res.instructionsAfterFuse,
                    res.fusedInstructions,
                    res.fusedPercentage,
                    res.avgFusionLength
                )
            );
        }

        aggregateWriter.writeLine(
            fmt::format(
                "{},{},{},{},{},{},{},{}",
                run.aggregateResults.run.title,
                run.aggregateResults.run.description,
                run.aggregateResults.run.userDefinedKey,
                run.aggregateResults.totalInstructions,
                run.aggregateResults.instructionsAfterFuse,
                run.aggregateResults.fusedInstructions,
                run.aggregateResults.fusedPercentage,
                run.aggregateResults.avgFusionLength
            )
        );
    }

    for (auto const& run : results.runResults) {
        for (auto const& res : run.results) {
            for (auto const& pair : res.fusionLengths) { // pair of count, length
                fusionLengthsWriter.writeLine(
                    fmt::format(
                        "{},{},{},{},{},{}",
                        res.run.title,
                        res.run.description,
                        res.run.userDefinedKey,
                        res.file->fileName,
                        pair.first, // count
                        pair.second // length
                    )
                );
            }
        }
    }
}

} // namespace fusion
