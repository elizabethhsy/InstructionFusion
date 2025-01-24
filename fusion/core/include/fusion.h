#pragma once

#include <fusion_core_export.h>

#include "dataRepresentation.h"
#include "macros.h"

#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <boost/describe/enum.hpp>
#include <boost/functional/hash.hpp>

namespace fusion
{

using namespace std;

BOOST_DEFINE_ENUM_CLASS(
    FusableResult,
    ERROR, // should not be output
    NOT_FUSABLE, // end the fusable block before current instruction
    START_OF_FUSABLE, // start the fusable block with the current instruction
    END_OF_FUSABLE, // end the fusable block after the current instruction
    FUSABLE // can continue expanding the fusable block
);

using RuleT =
    function<FusableResult(vector<shared_ptr<Instr>> const&, Instr const&)>;

struct FusionRule
{
    RuleT rule;

    // Constructor accepting a custom rule
    FusionRule(RuleT function) : rule(std::move(function)) {}

    FusableResult apply(
        vector<shared_ptr<Instr>> const& block,
        Instr const& instruction
    ) const
    {
        return rule(block, instruction);
    }

    FusionRule chain(FusionRule const& nextRule) const
    {
        FusionRule copy = *this; // copy the struct from the stack
        return FusionRule(
            [=](
                vector<shared_ptr<Instr>> const& block,
                Instr const& instruction) -> FusableResult
            {
                auto firstResult = copy.apply(block, instruction);
                auto nextResult = nextRule.apply(block, instruction);
                return min(firstResult, nextResult); // return the worst result
            }
        );
    }
};
using FusionRulePtr = shared_ptr<FusionRule>;

struct ExperimentRun;
struct FUSION_CORE_NO_EXPORT FusionResults
{
    File const& file;
    ExperimentRun const& run;

    uint64_t totalInstructions;
    uint64_t instructionsAfterFuse;
    uint64_t fusedInstructions;
    vector<pair<uint64_t, uint64_t>> fusionLengths; // count, block length
    double avgFusionLength;
    double fusedPercentage;
    
    string toString() const;
};

struct FUSION_CORE_NO_EXPORT FusionCalculator
{
    FusionResults calculateFusion(
        File const& file,
        ExperimentRun const& run
    );

    static float calcAvgFusionLength(
        vector<pair<uint64_t, uint64_t>> const& fusionLengths
    );
};

}
