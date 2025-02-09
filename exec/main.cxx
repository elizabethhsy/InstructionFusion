#include "rules/celio_2016_rules.h"
#include "rules/my_own_rules.h"

#include <csvHandler.h>
#include <exampleRules.h>
#include <experiment.h>
#include <fusion.h>
#include <instructionCount.h>
#include <instructions.h>
#include <simulationExperiment.h>
#include <inOrderPipeline.h>

#include <chrono>
#include <fmt/core.h>
#include <string>
#include <unordered_set>
#include <vector>

int main(int argc, char const *argv[])
{
    std::vector<std::string> fileNames = {
        "astar",
        "bzip2",
        "gobmk",
        "h264ref",
        "hmmer",
        "libquantum",
        "mcf",
        "omnetpp",
        "sjeng",
        "xalancbmk"
    };

    const auto now = std::chrono::system_clock::now();
    std::string dirPath =
            "/Users/elizabeth/Desktop/Cambridge/Dissertation/cleaned data";
    std::string resultsPath =
        fmt::format(
            "/Users/elizabeth/Desktop/Cambridge/Dissertation/data results/{}",
            std::format("{:%Y_%m_%d_%H:%M:%OS}", now)
        );
        
    for (auto& name : fileNames) {
        name = fmt::format("{}/{}.csv", dirPath, name);
    }

    auto const& baseRuns = my_rules::baseRuns;
    std::vector<fusion::ExperimentRun> runs = baseRuns;
    for (int i = 1; i <= 10; i++) {
        for (auto const& run : baseRuns) {
            std::unordered_set<fusion::FusionRulePtr> rules;
            for (auto rule : run.rules) {
                rules.insert(std::make_shared<fusion::FusionRule>(
                    fusion::maxLength(i).chain(*rule)
                ));
            }

            runs.push_back(
                fusion::ExperimentRun{
                    .title = run.title,
                    .userDefinedKey = fmt::format("{}", i),
                    .rules = rules
                }
            );
        }
    }

    auto experimentManager = std::make_shared<fusion::ExperimentManager>(
        fileNames,
        runs
    );
    auto instructionCountResults = experimentManager->run
        <fusion::InstructionCountExperiment, fusion::FusionResults>
        ();
    experimentManager->save
        <fusion::InstructionCountExperiment, fusion::FusionResults>(
            instructionCountResults,
            resultsPath
        );
    auto cycleCountResults = experimentManager->run
        <fusion::PipelineExperiment<fusion::InOrderPipeline>, fusion::PipelineResult>(
            instructionCountResults
        );
    experimentManager->save
        <fusion::PipelineExperiment<fusion::InOrderPipeline>, fusion::PipelineResult>(
            cycleCountResults
        );
    LOG_INFO(cycleCountResults.toString());
    
    // auto simulationResults = experimentManager->run
    //     <fusion::SimulationExperiment, fusion::SimulationResults>();
    // experimentManager->save
    //     <fusion::SimulationExperiment, fusion::SimulationResults>(
    //         simulationResults,
    //         resultsPath
    //     );

    return 0;
}
