#include "instructions.h"
#include "rules.h"

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
            "/Users/elizabeth/Desktop/Cambridge/Dissertation/cleaned_data";
    std::string resultsPath =
        fmt::format(
            "/Users/elizabeth/Desktop/Cambridge/Dissertation/data results/{}",
            std::format("{:%Y_%m_%d_%H:%M:%OS}", now)
        );
        
    for (auto& name : fileNames) {
        name = fmt::format("{}/{}.csv", dirPath, name);
    }

    // std::vector<fusion::ExperimentRun> baseRuns = {
    //     fusion::ExperimentRun{
    //         .title = "arithmetic only",
    //         .userDefinedKey = "0",
    //         .rules = std::unordered_set<fusion::FusionRulePtr>{
    //             std::make_shared<fusion::FusionRule>(
    //                 fusion::arithmeticOnly.chain(fusion::sameCount)
    //             )
    //         }
    //     },
    //     fusion::ExperimentRun{
    //         .title = "arithmetic end memory",
    //         .userDefinedKey = "0",
    //         .rules = std::unordered_set<fusion::FusionRulePtr>{
    //             std::make_shared<fusion::FusionRule>(
    //                 fusion::sameCount.chain(fusion::arithmeticEndMemory)
    //             )
    //         }
    //     },
    //     fusion::ExperimentRun{
    //         .title = "arithmetic end branch",
    //         .userDefinedKey = "0",
    //         .rules = std::unordered_set<fusion::FusionRulePtr>{
    //             std::make_shared<fusion::FusionRule>(
    //                 fusion::sameCount.chain(fusion::arithmeticEndBranch)
    //             )
    //         }
    //     },
    //     fusion::ExperimentRun{
    //         .title = "arithmetic end memory/branch",
    //         .userDefinedKey = "0",
    //         .rules = std::unordered_set<fusion::FusionRulePtr>{
    //             std::make_shared<fusion::FusionRule>(
    //                 fusion::sameCount.chain(fusion::arithmeticEndMemory)
    //             ),
    //             std::make_shared<fusion::FusionRule>(
    //                 fusion::sameCount.chain(fusion::arithmeticEndBranch)
    //             )
    //         }
    //     },
    //     fusion::ExperimentRun{
    //         .title = "arithmetic only (I)",
    //         .userDefinedKey = "0",
    //         .rules = std::unordered_set<fusion::FusionRulePtr>{
    //             std::make_shared<fusion::FusionRule>(
    //                 fusion::independent
    //                     .chain(fusion::sameCount)
    //                     .chain(fusion::arithmeticOnly)
    //             )
    //         }
    //     },
    //     fusion::ExperimentRun{
    //         .title = "arithmetic end memory (I)",
    //         .userDefinedKey = "0",
    //         .rules = std::unordered_set<fusion::FusionRulePtr>{
    //             std::make_shared<fusion::FusionRule>(
    //                 fusion::independent
    //                     .chain(fusion::sameCount)
    //                     .chain(fusion::arithmeticEndMemory)
    //             )
    //         }
    //     },
    //     fusion::ExperimentRun{
    //         .title = "arithmetic end branch (I)",
    //         .userDefinedKey = "0",
    //         .rules = std::unordered_set<fusion::FusionRulePtr>{
    //             std::make_shared<fusion::FusionRule>(
    //                 fusion::independent
    //                     .chain(fusion::sameCount)
    //                     .chain(fusion::arithmeticEndBranch)
    //             )
    //         }
    //     },
    //     fusion::ExperimentRun{
    //         .title = "arithmetic end memory/branch (I)",
    //         .userDefinedKey = "0",
    //         .rules = std::unordered_set<fusion::FusionRulePtr>{
    //             std::make_shared<fusion::FusionRule>(
    //                 fusion::independent
    //                     .chain(fusion::sameCount)
    //                     .chain(fusion::arithmeticEndMemory)
    //             ),
    //             std::make_shared<fusion::FusionRule>(
    //                 fusion::independent
    //                     .chain(fusion::sameCount)
    //                     .chain(fusion::arithmeticEndBranch)
    //             )
    //         }
    //     }
    // };

    std::vector<fusion::ExperimentRun> baseRuns = {
        fusion::ExperimentRun{
            .title = "load effective address",
            .userDefinedKey = "0",
            .rules = std::unordered_set<fusion::FusionRulePtr>{
                std::make_shared<fusion::FusionRule>(
                    fusion::sameCount.chain(loadEffectiveAddress)
                )
            }
        },
        fusion::ExperimentRun{
            .title = "indexed load",
            .userDefinedKey = "0",
            .rules = std::unordered_set<fusion::FusionRulePtr>{
                std::make_shared<fusion::FusionRule>(
                    fusion::sameCount.chain(indexedLoad)
                )
            }
        },
        // fusion::ExperimentRun{
        //     .title = "fused indexed load",
        //     .userDefinedKey = "0",
        //     .rules = std::unordered_set<fusion::FusionRulePtr>{
        //         std::make_shared<fusion::FusionRule>(
        //             fusion::sameCount.chain(fusedIndexedLoad)
        //         )
        //     }
        // },
        fusion::ExperimentRun{
            .title = "clear upper word",
            .userDefinedKey = "0",
            .rules = std::unordered_set<fusion::FusionRulePtr>{
                std::make_shared<fusion::FusionRule>(
                    fusion::sameCount.chain(clearUpperWord)
                )
            }
        },
        // fusion::ExperimentRun{
        //     .title = "load global",
        //     .userDefinedKey = "0",
        //     .rules = std::unordered_set<fusion::FusionRulePtr>{
        //         std::make_shared<fusion::FusionRule>(
        //             fusion::sameCount.chain(loadGlobal)
        //         )
        //     }
        // },
        fusion::ExperimentRun{
            .title = "overall results",
            .userDefinedKey = "0",
            .rules = std::unordered_set<fusion::FusionRulePtr>{
                std::make_shared<fusion::FusionRule>(
                    fusion::sameCount.chain(loadEffectiveAddress)
                ),
                std::make_shared<fusion::FusionRule>(
                    fusion::sameCount.chain(indexedLoad)
                ),
                // std::make_shared<fusion::FusionRule>(
                //     fusion::sameCount.chain(fusedIndexedLoad)
                // ),
                std::make_shared<fusion::FusionRule>(
                    fusion::sameCount.chain(clearUpperWord)
                ),
                // std::make_shared<fusion::FusionRule>(
                //     fusion::sameCount.chain(loadGlobal)
                // )
            }
        }
    };

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

    fusion::Experiment experiment(fileNames, runs);
    auto results = experiment.run();
    experiment.save(results, resultsPath);
    return 0;
}
