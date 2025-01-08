#include "instructions.h"

#include <csvHandler.h>
#include <exampleRules.h>
#include <experiment.h>
#include <fusion.h>

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

    // std::vector<std::string> combinedInstructions;
    // combinedInstructions.reserve(
    //     memoryInstructions.size() +
    //     branchInstructions.size()
    // );
    
    // combinedInstructions.insert(
    //     combinedInstructions.end(),
    //     memoryInstructions.begin(),
    //     memoryInstructions.end()
    // );
    
    // combinedInstructions.insert(
    //     combinedInstructions.end(),
    //     branchInstructions.begin(),
    //     branchInstructions.end()
    // );

    // std::vector<fusion::FusionConfig> configs = {
    //     fusion::FusionConfig {
    //         .fusableName = "ARITHMETIC",
    //         .fusable = arithmeticInstructions,
    //         .independentInstructionsOnly = true
    //     },
    //     fusion::FusionConfig {
    //         .fusableName = "ARITHMETIC",
    //         .fusable = arithmeticInstructions,
    //         .endName = "MEMORY",
    //         .end = memoryInstructions,
    //         .independentInstructionsOnly = true
    //     },
    //     fusion::FusionConfig {
    //         .fusableName = "ARITHMETIC",
    //         .fusable = arithmeticInstructions,
    //         .endName = "BRANCH",
    //         .end = branchInstructions,
    //         .independentInstructionsOnly = true
    //     },
    //     fusion::FusionConfig {
    //         .fusableName = "ARITHMETIC",
    //         .fusable = arithmeticInstructions,
    //         .endName = "MEMORY & BRANCH",
    //         .end = combinedInstructions,
    //         .independentInstructionsOnly = true
    //     }
    // };

    // for (uint i = 1; i <= 6; i++) {
    //     configs.push_back(
    //         fusion::FusionConfig {
    //             .fusableName = "ARITHMETIC",
    //             .fusable = arithmeticInstructions,
    //             .maxFusableLength = i,
    //             .independentInstructionsOnly = true
    //         }
    //     );
        
    //     configs.push_back(
    //         fusion::FusionConfig {
    //             .fusableName = "ARITHMETIC",
    //             .fusable = arithmeticInstructions,
    //             .endName = "MEMORY",
    //             .end = memoryInstructions,
    //             .maxFusableLength = i,
    //             .independentInstructionsOnly = true
    //         }
    //     );

    //     configs.push_back(
    //         fusion::FusionConfig {
    //             .fusableName = "ARITHMETIC",
    //             .fusable = arithmeticInstructions,
    //             .endName = "BRANCH",
    //             .end = branchInstructions,
    //             .maxFusableLength = i,
    //             .independentInstructionsOnly = true
    //         }
    //     );

    //     configs.push_back(
    //         fusion::FusionConfig {
    //             .fusableName = "ARITHMETIC",
    //             .fusable = arithmeticInstructions,
    //             .endName = "MEMORY & BRANCH",
    //             .end = combinedInstructions,
    //             .maxFusableLength = i,
    //             .independentInstructionsOnly = true
    //         }
    //     );
    // }

    std::vector<fusion::ExperimentRun> runs;
    runs.emplace_back(fusion::ExperimentRun{
        .title = "arithmetic only",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(fusion::arithmeticOnly)
        }
    });

    runs.emplace_back(fusion::ExperimentRun{
        .title = "arithmetic end memory",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(fusion::arithmeticEndMemory)
        }
    });

    runs.emplace_back(fusion::ExperimentRun{
        .title = "arithmetic end branch",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(fusion::arithmeticEndBranch)
        }
    });

    runs.emplace_back(fusion::ExperimentRun{
        .title = "arithmetic end memory/branch",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(fusion::arithmeticEndMemory),
            std::make_shared<fusion::FusionRule>(fusion::arithmeticEndBranch)
        }
    });

    fusion::Experiment experiment(fileNames, runs);
    auto results = experiment.run();
    experiment.save(results, resultsPath);
    return 0;
}
