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

struct BasicBlock : InstrBlock
{
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
    vector<shared_ptr<BasicBlock>> basicBlocks;
    unordered_map<uint32_t, shared_ptr<BasicBlock>> addr_map;
    unordered_map<string, shared_ptr<BasicBlock>> label_map;

    // prev BasicBlock -edge count-> next BasicBlock
    unordered_map<
        shared_ptr<BasicBlock>,
        unordered_set<shared_ptr<BasicBlock>>
    > adjList;

    // next BasicBlock -edge count-> prev BasicBlock
    unordered_map<
        shared_ptr<BasicBlock>,
        unordered_set<shared_ptr<BasicBlock>>
    > revAdjList;

    // constructs the control flow graph from the instructions
    ControlFlowGraph(File const& file);
    vector<ControlFlowPath> computePaths();
private:
    void constructBasicBlocks(File const& file);
    void constructDependencies();
    void findEdges();
};

struct Edge
{
    shared_ptr<BasicBlock> start;
    shared_ptr<BasicBlock> end;
    uint64_t count = 0; // to be populated

    string toString() const;

    bool operator==(Edge const& other) const {
        return start == other.start && end == other.end && count == other.count;
    }
};

struct EdgeHash {
    size_t operator()(const Edge& edge) const {
        size_t h1 = hash<shared_ptr<BasicBlock>>{}(edge.start);
        size_t h2 = hash<shared_ptr<BasicBlock>>{}(edge.end);
        size_t h3 = hash<uint64_t>{}(edge.count);

        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

} // fusion
