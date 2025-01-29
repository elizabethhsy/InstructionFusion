#pragma once

#include <dataRepresentation.h>

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// Convert an instruction histogram into a control flow graph, and split it into
// a list of control flow paths with a count associated with each one,
// representing the number of times that path was executed.

// Each path will then be fed into the pipeline simulator to analyse for fusion
// opportunities.

namespace fusion
{

using namespace std;

struct BasicBlock
{
    string label = "";
    uint32_t addr; // starting address
    uint64_t count;
    vector<shared_ptr<Instr>> instructions;

    string toString() const;
    static bool sameBasicBlock(Instr const& prev, Instr const& next);
};

struct ControlFlowPath
{
    vector<shared_ptr<BasicBlock>> nodes; // list of nodes that form the path
    uint64_t count; // the number of times that path is executed

    string toString() const;
};

struct ControlFlowGraph
{
    vector<BasicBlock> basicBlocks;
    unordered_map<uint32_t, shared_ptr<BasicBlock>> addr_map;
    unordered_map<string, shared_ptr<BasicBlock>> label_map;

    // prev BasicBlock --> next BasicBlock
    unordered_map<shared_ptr<BasicBlock>, vector<shared_ptr<BasicBlock>>> adj_list;
    unordered_map<shared_ptr<BasicBlock>, vector<shared_ptr<BasicBlock>>> rev_adj_list;

    // constructs the control flow graph from the instructions
    ControlFlowGraph(File const& file);
    vector<ControlFlowPath> computePaths();
private:
    void constructBasicBlocks(File const& file);
};

} // fusion
