#include "inOrderPipeline.h"

namespace fusion
{

// take a basic block at a time, and see if there are any stalls. In a simple
// in-order pipeline (with good forwarding), a stall only occurs if there is a
// load-after-use.
uint64_t InOrderPipeline::computeCycleCount(vector<FusedBlock> const& blocks)
{
    for (auto const& block : blocks) {
        
    }
}

};
