#pragma once

#include "dataRepresentation.h"
#include "macros.h"

namespace fusion
{

using namespace std;

struct FusionCalculator
{
    uint64_t calculateFusion(
        File const& file,
        vector<string> const& delimiters
    );
};

}
