#pragma once

#include <vector>

namespace fusion
{

using namespace std;

struct Stage
{

};

struct Pipeline
{
    vector<Stage> stages;
};

// a simple in-order pipeline with 5 stages.

struct inOrderPipeline
{
    vector<Instr> instrPool;

    Stage fetch;
    Stage decode;
    Stage execute;
    Stage memory;
    Stage writeback;
};

} // fusion
