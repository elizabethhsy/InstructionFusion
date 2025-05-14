#include "rules/celio_2016_rules.h"
#include "rules/my_own_rules.h"

#include <fileHandler.h>
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
    
    // std::vector<std::string> keys = {"l", "r", "w"};
    // std::vector<fusion::ExperimentRun> runs = {};
    // for (auto key : keys) {
    //     for (auto const& run : baseRuns) {
    //         runs.push_back(
    //             fusion::ExperimentRun{
    //                 .title = run.title,
    //                 .userDefinedKey = fmt::format("{}-0", key),
    //                 .rules = run.rules
    //             }
    //         );
    //     }

    //     for (int i = 1; i <= 10; i++) {
    //         for (auto const& run : baseRuns) {
    //             std::unordered_set<fusion::FusionRulePtr> rules;
    //             for (auto rule : run.rules) {
    //                 if (key == "l") {
    //                     rules.insert(std::make_shared<fusion::FusionRule>(
    //                         fusion::maxLength(i).chain(*rule)
    //                     ));
    //                 } else if (key == "r") {
    //                     rules.insert(std::make_shared<fusion::FusionRule>(
    //                         fusion::maxReadPorts(i).chain(*rule)
    //                     ));
    //                 } else if (key == "w") {
    //                     rules.insert(std::make_shared<fusion::FusionRule>(
    //                         fusion::maxWritePorts(i).chain(*rule)
    //                     ));
    //                 }
    //             }
    
    //             runs.push_back(
    //                 fusion::ExperimentRun{
    //                     .title = run.title,
    //                     .userDefinedKey = fmt::format("{}-{}", key, i),
    //                     .rules = rules
    //                 }
    //             );
    //         }
    //     }
    // }

    auto experimentManager = std::make_shared<fusion::ExperimentManager>(
        "performance analysis with pipeline starting from scratch",
        fileNames,
        runs,
        resultsPath
    );
    // auto instructionCountResults = experimentManager->run
    //     <fusion::InstructionCountExperiment, fusion::FusionResults>
    //     ();
    // experimentManager->save
    //     <fusion::InstructionCountExperiment, fusion::FusionResults>(
    //         instructionCountResults
    //     );
    for (int i = 0; i < 100; i++) {
        auto instructionCountResults = experimentManager->run
            <fusion::InstructionCountExperiment, fusion::FusionResults>
            ();
        experimentManager->save
            <fusion::InstructionCountExperiment, fusion::FusionResults>(
                instructionCountResults
            );
    
        auto cycleCountResults = experimentManager->run
            <fusion::PipelineExperiment<fusion::InOrderPipeline>,
            fusion::PipelineResult>
            ();
        experimentManager->save
            <fusion::PipelineExperiment<fusion::InOrderPipeline>,
            fusion::PipelineResult>
            (
                cycleCountResults
            );
    }
    
    // auto simulationResults = experimentManager->run
    //     <fusion::SimulationExperiment, fusion::SimulationResults>();
    // experimentManager->save
    //     <fusion::SimulationExperiment, fusion::SimulationResults>(
    //         simulationResults,
    //         resultsPath
    //     );

    return 0;
}
