#pragma once

#include <fusion_core_export.h>

#include <dataRepresentation.h>
#include <experiment.h>
#include <macros.h>

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

struct FUSION_CORE_NO_EXPORT FusionResults
{
    shared_ptr<File> file;
    ExperimentRun const& run;

    uint64_t totalInstructions;
    uint64_t instructionsAfterFuse;
    uint64_t fusedInstructions;
    vector<FusedBlock> fusedBlocks;
    vector<pair<uint64_t, uint64_t>> fusionLengths; // count, block length
    double avgFusionLength;
    double fusedPercentage;
    
    string toString() const;
};

struct FUSION_CORE_NO_EXPORT FusionCalculator
{
    FusionResults calculateFusion(
        shared_ptr<File> file,
        ExperimentRun const& run
    );

    static float calcAvgFusionLength(
        vector<pair<uint64_t, uint64_t>> const& fusionLengths
    );
};

}
