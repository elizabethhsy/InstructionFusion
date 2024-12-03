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
    string fusableName;
    vector<string> const& fusable;
    string endName;
    vector<string> const& end = {};
    uint64_t maxFusableLength = UINT64_MAX;

    string title() const;
    string toString() const;
};

// to be returned to ExperimentResults for the client to parse
struct FusionResults
{
    File const& file;
    FusionConfig const& config;

    uint64_t totalInstructions;
    uint64_t instructionsAfterFuse;
    uint64_t fusedInstructions;
    vector<pair<uint, uint>> fusionLengths; // count, block length
    float avgFusionLength;
    float fusedPercentage;
};

struct FusionCalculator
{
    FusionResults calculateFusion(
        File const& file,
        FusionConfig const& config
    );
};

}
