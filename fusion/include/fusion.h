#pragma once

#include <fusion_export.h>

#include "dataRepresentation.h"
#include "macros.h"

#include <functional>
#include <vector>
#include <string>
#include <unordered_set>

#include <boost/describe/enum.hpp>

namespace fusion
{

using namespace std;

BOOST_DEFINE_ENUM_CLASS(
    FusableResult,
    ERROR, // should not be output
    NOT_FUSABLE, // end the fusable block before current instruction
    END_OF_FUSABLE, // end the fusable block after the current instruction
    FUSABLE // can continue expanding the fusable block
);

using FusionRule =
    function<FusableResult(vector<shared_ptr<Instr>> const&, Instr const&)>;
using FusionRulePtr = shared_ptr<FusionRule>;

struct ExperimentRun;
struct FUSION_EXPORT FusionResults
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

struct FUSION_NO_EXPORT FusionCalculator
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
