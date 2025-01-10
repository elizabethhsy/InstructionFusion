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

    std::vector<fusion::ExperimentRun> runs = {
        fusion::ExperimentRun{
            .title = "arithmetic only",
            .userDefinedKey = "0",
            .rules = std::unordered_set<fusion::FusionRulePtr>{
                std::make_shared<fusion::FusionRule>(fusion::arithmeticOnly)
            }
        },
        fusion::ExperimentRun{
            .title = "arithmetic end memory",
            .userDefinedKey = "0",
            .rules = std::unordered_set<fusion::FusionRulePtr>{
                std::make_shared<fusion::FusionRule>(fusion::arithmeticEndMemory)
            }
        },
        fusion::ExperimentRun{
            .title = "arithmetic end branch",
            .userDefinedKey = "0",
            .rules = std::unordered_set<fusion::FusionRulePtr>{
                std::make_shared<fusion::FusionRule>(fusion::arithmeticEndBranch)
            }
        },
        fusion::ExperimentRun{
            .title = "arithmetic end memory/branch",
            .userDefinedKey = "0",
            .rules = std::unordered_set<fusion::FusionRulePtr>{
                std::make_shared<fusion::FusionRule>(fusion::arithmeticEndMemory),
                std::make_shared<fusion::FusionRule>(fusion::arithmeticEndBranch)
            }
        },
        fusion::ExperimentRun{
            .title = "arithmetic only (I)",
            .userDefinedKey = "0",
            .rules = std::unordered_set<fusion::FusionRulePtr>{
                std::make_shared<fusion::FusionRule>(
                    fusion::independent.chain(fusion::arithmeticOnly)
                )
            }
        },
        fusion::ExperimentRun{
            .title = "arithmetic end memory (I)",
            .userDefinedKey = "0",
            .rules = std::unordered_set<fusion::FusionRulePtr>{
                std::make_shared<fusion::FusionRule>(
                    fusion::independent.chain(fusion::arithmeticEndMemory)
                )
            }
        },
        fusion::ExperimentRun{
            .title = "arithmetic end branch (I)",
            .userDefinedKey = "0",
            .rules = std::unordered_set<fusion::FusionRulePtr>{
                std::make_shared<fusion::FusionRule>(
                    fusion::independent.chain(fusion::arithmeticEndBranch)
                )
            }
        },
        fusion::ExperimentRun{
            .title = "arithmetic end memory/branch (I)",
            .userDefinedKey = "0",
            .rules = std::unordered_set<fusion::FusionRulePtr>{
                std::make_shared<fusion::FusionRule>(
                    fusion::independent.chain(fusion::arithmeticEndMemory)
                ),
                std::make_shared<fusion::FusionRule>(
                    fusion::independent.chain(fusion::arithmeticEndBranch)
                )
            }
        }
    };

    for (int i = 1; i <= 10; i++) {
        runs.push_back(
            fusion::ExperimentRun{
                .title = "arithmetic only",
                .userDefinedKey = fmt::format("{}", i),
                .rules = std::unordered_set<fusion::FusionRulePtr>{
                    std::make_shared<fusion::FusionRule>(
                        fusion::maxLength(i).chain(fusion::arithmeticOnly)
                    )
                }
            }
        );

        runs.push_back(
            fusion::ExperimentRun{
                .title = "arithmetic end memory",
                .userDefinedKey = fmt::format("{}", i),
                .rules = std::unordered_set<fusion::FusionRulePtr>{
                    std::make_shared<fusion::FusionRule>(
                        fusion::maxLength(i).chain(fusion::arithmeticEndMemory)
                    )
                }
            }
        );

        runs.push_back(
            fusion::ExperimentRun{
                .title = "arithmetic end branch",
                .userDefinedKey = fmt::format("{}", i),
                .rules = std::unordered_set<fusion::FusionRulePtr>{
                    std::make_shared<fusion::FusionRule>(
                        fusion::maxLength(i).chain(fusion::arithmeticEndBranch)
                    )
                }
            }
        );

        runs.push_back(
            fusion::ExperimentRun{
                .title = "arithmetic end memory/branch",
                .userDefinedKey = fmt::format("{}", i),
                .rules = std::unordered_set<fusion::FusionRulePtr>{
                    std::make_shared<fusion::FusionRule>(
                        fusion::maxLength(i).chain(fusion::arithmeticEndMemory)
                    ),
                    std::make_shared<fusion::FusionRule>(
                        fusion::maxLength(i).chain(fusion::arithmeticEndBranch)
                    )
                }
            }
        );

        runs.push_back(
            fusion::ExperimentRun{
                .title = "arithmetic only (I)",
                .userDefinedKey = fmt::format("{}", i),
                .rules = std::unordered_set<fusion::FusionRulePtr>{
                    std::make_shared<fusion::FusionRule>(
                        fusion::independent.chain(
                            fusion::maxLength(i).chain(fusion::arithmeticOnly)
                        )
                    )
                }
            }
        );

        runs.push_back(
            fusion::ExperimentRun{
                .title = "arithmetic end memory (I)",
                .userDefinedKey = fmt::format("{}", i),
                .rules = std::unordered_set<fusion::FusionRulePtr>{
                    std::make_shared<fusion::FusionRule>(
                        fusion::independent
                            .chain(fusion::maxLength(i))
                            .chain(fusion::arithmeticEndMemory)
                    )
                }
            }
        );

        runs.push_back(
            fusion::ExperimentRun{
                .title = "arithmetic end branch (I)",
                .userDefinedKey = fmt::format("{}", i),
                .rules = std::unordered_set<fusion::FusionRulePtr>{
                    std::make_shared<fusion::FusionRule>(
                        fusion::independent
                            .chain(fusion::maxLength(i))
                            .chain(fusion::arithmeticEndBranch)
                    )
                }
            }
        );

        runs.push_back(
            fusion::ExperimentRun{
                .title = "arithmetic end memory/branch (I)",
                .userDefinedKey = fmt::format("{}", i),
                .rules = std::unordered_set<fusion::FusionRulePtr>{
                    std::make_shared<fusion::FusionRule>(
                        fusion::independent
                            .chain(fusion::maxLength(i))
                            .chain(fusion::arithmeticEndMemory)
                    ),
                    std::make_shared<fusion::FusionRule>(
                        fusion::independent
                            .chain(fusion::maxLength(i))
                            .chain(fusion::arithmeticEndBranch)
                    )
                }
            }
        );
    }

    fusion::Experiment experiment(fileNames, runs);
    auto results = experiment.run();
    experiment.save(results, resultsPath);
    return 0;
}
