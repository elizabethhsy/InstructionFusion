#pragma once

#include "dataRepresentation.h"
#include "macros.h"

#include <vector>
#include <string>

namespace fusion
{

using namespace std;

struct FusionConfig
{
    vector<string> const& fusable;
    vector<string> const& end = {};
    uint64_t maxFusableLength = UINT64_MAX;
};

struct FusionStats
{
    uint64_t totalInstructions;
    uint64_t instructionsAfterFuse;
    vector<pair<uint, uint>> fusionLengths;
    float getAvgFusionLength();
};

struct FusionCalculator
{
    FusionStats calculateFusion(
        File const& file,
        FusionConfig const& config
    );
};

}
