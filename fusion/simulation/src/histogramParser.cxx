#include "histogramParser.h"

#include <instructions.h>

#include <cstdint>
#include <functional>
#include <memory>

namespace fusion
{

// check if two instructions belong in the same critical section
inline bool BasicBlock::sameBasicBlock(
    Instr const& prev,
    Instr const& next
)
{
    return (prev.count == next.count && !branchInstructions.contains(prev.instr)
        && Instr::isContiguous(prev, next)) || prev == next;
}

string BasicBlock::toString() const
{
    return ""; // TODO
}

string ControlFlowPath::toString() const
{
    return ""; // TODO
}

ControlFlowGraph::ControlFlowGraph(File const& file)
{
    constructBasicBlocks(file);

    auto add_edge = [this](BasicBlock const& prev, BasicBlock const& next) {
        this->adj_list[make_shared<BasicBlock>(prev)]
            .push_back(make_shared<BasicBlock>(next));
        this->rev_adj_list[make_shared<BasicBlock>(next)]
            .push_back(make_shared<BasicBlock>(prev));
    };

    // find the control flow graph given the set of basic blocks
    auto n = basicBlocks.size();
    for (int i = 0; i < n; i++) {
        auto const& currBlock = basicBlocks[i];
        auto const& lastInstr = currBlock.instructions.back();

        // connect an edge to the next block if they are contiguous
        if (i < n-1) {
            auto const& nextBlock = basicBlocks[i+1];
            if (Instr::isContiguous(*lastInstr, *nextBlock.instructions[0])) {
                add_edge(currBlock, nextBlock);
            }
        }

        // look at the last instruction in the block. If it is a branch
        // instruction, then extract the targets from the operands, which is
        // anything that is an address (translates to a hex number) or a label
        // (matches <label> string pattern).
        if (branchInstructions.contains(lastInstr->instr)) {
            for (auto const& op : lastInstr->operands) {
                auto intRep = stoi(op, nullptr, 16);
                if (addr_map.contains(intRep)) {
                    add_edge(currBlock, *addr_map[intRep]);
                } else if (label_map.contains(op)) {
                   add_edge(currBlock, *label_map[op]);
                }
            }
        }
    }
}

// from the adjacency lists, compute the list of paths that go from the root
// to a terminal node, which can then be passed into the instruction pipeline
vector<ControlFlowPath> ControlFlowGraph::computePaths()
{
    vector<ControlFlowPath> paths;
    auto graph = adj_list; // make a temporary copy to compute the graphs

    // find the roots of the graph, i.e. the list of nodes that have 0 indegree
    unordered_set<shared_ptr<BasicBlock>> roots;
    for (auto const& block : basicBlocks) {
        if (rev_adj_list[make_shared<BasicBlock>(block)].size() == 0) {
            roots.insert(make_shared<BasicBlock>(block));
        }
    }

    // from the roots, explore the graph until we reach a terminal node. If we
    // have a cycle (we revisited a node that is previously in our path), we go
    // back to the start of the cycle but then make sure to not go down the same
    // path again. TODO: could also terminate the path there
    // Along the way, we keep track of the minimum count edge and the path, and
    // at the end we backtrack along the path and subtract each node's count by
    // that minimum count.
    // We repeatedly find paths from the root until all paths have been found,
    // or we have a disconnected graph at which point we output an error.
    vector<shared_ptr<BasicBlock>> path;
    unordered_set<shared_ptr<BasicBlock>> path_nodes; // for lookup
    uint64_t minCount = UINT64_MAX;

    auto reset_path = [&]() {
        path.clear();
        path_nodes.clear();
        minCount = UINT64_MAX;
    };

    function<void(shared_ptr<BasicBlock>)> iterate_path =
        [&](shared_ptr<BasicBlock> curr) -> void {
            path.push_back(curr);
            path_nodes.insert(curr);
            minCount = min(minCount, curr->count);

            // get the first node that hasn't been visited
            shared_ptr<BasicBlock> next = graph[curr][0]; // get arbitrary node
            // if it has been visited or it has zero outdegree, then terminate
            // the path there
            if (path_nodes.contains(next) || graph[next].size() == 0) {
                return;
            }

            // it has non-zero outdegree, so we continue along the path
            iterate_path(next);
        };

    auto record_path =
        [&paths, &minCount, &graph](
            vector<shared_ptr<BasicBlock>> const& path,
            uint64_t count
        )
        {
            paths.push_back(ControlFlowPath{
                .nodes = path,
                .count = minCount
            });
            
            // go along the path and subtract the minCount from each node. If
            // the count, is zero, remove the node from the graph. There is at
            // least one node that satisfies this condition, from the definition
            // of minCount.
            for (auto const& node : path) {
                node->count -= minCount;
                if (node->count == 0) {
                    graph.erase(node);
                }
            }
        };

    while (!roots.empty()) { // there are still root nodes to traverse from
        auto const& root = *roots.begin(); // TODO: check if this is shared_ptr
        reset_path();
        iterate_path(root);
        record_path(path, minCount);
    }

    if (!graph.empty()) {
        LOG_ERROR(fmt::format("the graph is not well-formed"));
    }

    LOG_DEBUG(fmt::format("{} paths found", paths.size()));

    return paths;
};

// construct the basic blocks from the instructions in the file
void ControlFlowGraph::constructBasicBlocks(File const& file)
{
    BasicBlock temp;
    bool startOfBasicBlock = true;

    auto record_basic_block = [this, temp]() { // copy temp
        // assign basic block to the label and address maps
        if (!temp.label.empty()) {
            this->label_map[temp.label] = make_shared<BasicBlock>(temp);
        }
        // construct address and label lookup of basic blocks
        this->addr_map[temp.addr] = make_shared<BasicBlock>(temp);
        this->basicBlocks.push_back(std::move(temp));
    };

    auto start_basic_block = [&temp](Instr const& startInstr) {
        // create a new basic block by reassigning the temp basic block
        temp.addr = startInstr.addr;
        temp.label = startInstr.label;
        // we check in sameBasicBlock that every instruction in the same block
        // has the same count
        temp.count = startInstr.count;
        temp.instructions = { make_shared<Instr>(startInstr) };
    };

    int n = file.instructions.size();
    for (int i = 0; i < n; i++) {
        auto const& prev = file.instructions[max(i-1, 0)];
        auto const& curr = file.instructions[i];

        if (!BasicBlock::sameBasicBlock(prev, curr) || i == 0) {
            // mark the basic block to end at the boundary between prev and
            // curr
            record_basic_block(); // consecutive basic blocks
            start_basic_block(curr);
        } else {
            temp.instructions.push_back(make_shared<Instr>(curr));
        }
    }
}

} // fusion
