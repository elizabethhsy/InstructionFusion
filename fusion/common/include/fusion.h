#pragma once

#include "dataRepresentation.h"

#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <boost/describe/enum.hpp>

namespace fusion
{

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

struct FUSION_COMMON_EXPORT FusionRule
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

} // fusion
