#include "histogramParser.h"

#include <dataRepresentation.h>

#include <stdint.h>

namespace fusion
{

using namespace std;

struct InOrderPipeline
{
    uint64_t computeCycleCount(vector<BasicBlock> blocks);
};

} // fusion
