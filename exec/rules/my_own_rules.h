#pragma once

#include <exampleRules.h>
#include <experiment.h>
#include <fusion.h>

#include <fmt/core.h>
#include <string>
#include <unordered_set>
#include <vector>

namespace my_rules
{

std::vector<fusion::ExperimentRun> baseRuns = {
    fusion::ExperimentRun{
        .title = "arithmetic only",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::sameCount.chain(fusion::arithmeticOnly)
            )
        }
    },
    fusion::ExperimentRun{
        .title = "arithmetic end memory",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::sameCount.chain(fusion::arithmeticEndMemory)
            )
        }
    },
    fusion::ExperimentRun{
        .title = "arithmetic end branch",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::sameCount.chain(fusion::arithmeticEndBranch)
            )
        }
    },
    fusion::ExperimentRun{
        .title = "arithmetic end memory/branch",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::sameCount.chain(fusion::arithmeticEndMemory)
            ),
            std::make_shared<fusion::FusionRule>(
                fusion::sameCount.chain(fusion::arithmeticEndBranch)
            )
        }
    },
    fusion::ExperimentRun{
        .title = "arithmetic only (I)",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::independent
                    .chain(fusion::sameCount)
                    .chain(fusion::arithmeticOnly)
            )
        }
    },
    fusion::ExperimentRun{
        .title = "arithmetic end memory (I)",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::independent
                    .chain(fusion::sameCount)
                    .chain(fusion::arithmeticEndMemory)
            )
        }
    },
    fusion::ExperimentRun{
        .title = "arithmetic end branch (I)",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::independent
                    .chain(fusion::sameCount)
                    .chain(fusion::arithmeticEndBranch)
            )
        }
    },
    fusion::ExperimentRun{
        .title = "arithmetic end memory/branch (I)",
        .userDefinedKey = "0",
        .rules = std::unordered_set<fusion::FusionRulePtr>{
            std::make_shared<fusion::FusionRule>(
                fusion::independent
                    .chain(fusion::sameCount)
                    .chain(fusion::arithmeticEndMemory)
            ),
            std::make_shared<fusion::FusionRule>(
                fusion::independent
                    .chain(fusion::sameCount)
                    .chain(fusion::arithmeticEndBranch)
            )
        }
    }
};

} // my_rules
