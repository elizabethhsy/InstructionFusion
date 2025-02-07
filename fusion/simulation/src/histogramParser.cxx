#include "histogramParser.h"

#include <instructions.h>

#include <cstdint>
#include <functional>
#include <memory>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <Eigen/Dense>
#include "ortools/linear_solver/linear_solver.h"

namespace fusion
{

using namespace std;

string BasicBlock::toString() const
{
    return InstrBlock::toString("Basic Block");
}

string Edge::toString() const
{
    return fmt::format("{}-{}->{}", start->toString(), count, end->toString());
}

// check if two instructions belong in the same critical section
inline bool BasicBlock::sameBasicBlock(
    Instr const& prev,
    Instr const& next
)
{
    return (prev.count == next.count && !branchInstructions.contains(prev.instr)
        && Instr::isContiguous(prev, next)) || (prev == next);
}

string ControlFlowPath::toString() const
{
    return fmt::format(
        "Control Flow Path: count={}, blocks=\n{}",
        count,
        boost::algorithm::join(
            nodes |
            boost::adaptors::transformed(
                [](const shared_ptr<BasicBlock>& node)
                {
                    return fmt::format("\t{}",node->toString());
                }
            ),
            "\n"
        )
    );
}

ControlFlowGraph::ControlFlowGraph(File const& file)
{
    constructBasicBlocks(file);
    LOG_DEBUG(fmt::format("found {} basic blocks", basicBlocks.size()));
    constructDependencies();
    findEdges();
}

// from the adjacency lists, compute the list of paths that go from the root
// to a terminal node, which can then be passed into the instruction pipeline
vector<ControlFlowPath> ControlFlowGraph::computePaths()
{
    vector<ControlFlowPath> paths;
    // temporary storage of counts for path calculation
    unordered_map<shared_ptr<BasicBlock>, uint64_t> tempCount;
    for (auto const& block : basicBlocks) {
        tempCount[block] = block->count;
    }

    auto graph = adjList; // copy the shared pointers
    auto inverseGraph = revAdjList;

    // find the roots of the graph, i.e. the list of nodes that have 0 indegree
    unordered_set<shared_ptr<BasicBlock>> roots;
    for (auto const& block : basicBlocks) {
        if (revAdjList[block].size() == 0) {
            roots.insert(block);
        }
    }

    LOG_DEBUG(fmt::format("{} roots found", roots.size()));

    // from the roots, explore the graph until we reach a terminal node. If we
    // have a cycle (we revisited a node that is previously in our path), we
    // terminate the path here and start a new path.
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
            assert(tempCount[curr] > 0);
            path.push_back(curr);
            path_nodes.insert(curr);
            minCount = min(minCount, tempCount[curr]);

            if (graph[curr].size() == 0) return;
            // get the first node that hasn't been visited
            shared_ptr<BasicBlock> next = *(graph[curr].begin()); // get arbitrary node
            // if it has been visited or it has zero outdegree, then terminate
            // the path there
            if (path_nodes.contains(next)) {
                path.push_back(next); // add the end of the loop
                return;
            }

            // it has non-zero outdegree, so we continue along the path
            iterate_path(next);
        };

    auto record_path =
        [&paths, &minCount, &graph, &inverseGraph, &roots, &tempCount](
            vector<shared_ptr<BasicBlock>> const& path,
            uint64_t count
        )
        {
            auto remove_from_graph =
                [&graph, &inverseGraph, &roots](shared_ptr<BasicBlock> node) {
                    for (auto const& outgress : graph[node]) {
                        inverseGraph[outgress].erase(node);
                    }
                    graph.erase(node);

                    for (auto const& ingress : inverseGraph[node]) {
                        graph[ingress].erase(node);
                    }
                    inverseGraph.erase(node);

                    if (roots.contains(node)) {
                        roots.erase(node);
                    }
                };

            paths.push_back(ControlFlowPath{
                .nodes = path,
                .count = minCount
            });
            
            // go along the path and subtract the minCount from each node. If
            // the count, is zero, remove the node from the graph. There is at
            // least one node that satisfies this condition, from the definition
            // of minCount.
            for (auto const& node : path) {
                tempCount[node] -= minCount;
                // node->count -= minCount;
                if (tempCount[node] == 0) {
                    remove_from_graph(node);
                }
            }
        };

    while (!roots.empty()) { // there are still root nodes to traverse from
        auto const& root = *roots.begin();
        reset_path();
        iterate_path(root);
        record_path(path, minCount);
    }

    if (!graph.empty()) {
        LOG_ERROR(
            fmt::format(
                "the graph is not well-formed, {} remaining nodes out of {}",
                graph.size(),
                adjList.size()
            )
        );
    }

    return paths;
};

// construct the basic blocks from the instructions in the file
void ControlFlowGraph::constructBasicBlocks(File const& file)
{
    BasicBlock temp;
    bool startOfBasicBlock = true;

    auto record_basic_block = [this, &temp]() {
        if (temp.instructions.size() == 0) return;

        auto tempPtr = make_shared<BasicBlock>(temp); // copy temp
        this->basicBlocks.push_back(tempPtr);
        // assign basic block to the label and address maps
        if (!temp.label.empty()) {
            this->label_map[temp.label] = tempPtr;
        }
        // construct address and label lookup of basic blocks
        this->addr_map[temp.addr] = tempPtr;
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

    start_basic_block(file.instructions[0]);
    for (int i = 1; i < n; i++) {
        auto const& prev = file.instructions[i-1];
        auto const& curr = file.instructions[i];

        if (!BasicBlock::sameBasicBlock(prev, curr)) {
            // mark the basic block to end at the boundary between prev and
            // curr
            record_basic_block(); // consecutive basic blocks
            start_basic_block(curr);
        } else {
            temp.instructions.push_back(make_shared<Instr>(curr));
        }
    }
    record_basic_block();
}

void ControlFlowGraph::constructDependencies()
{
    auto add_edge =
        [this](shared_ptr<BasicBlock> prev, shared_ptr<BasicBlock> next) {
            this->adjList[prev].insert(next);
            this->revAdjList[next].insert(prev);
        };

    // find the control flow graph given the set of basic blocks
    auto n = basicBlocks.size();
    for (int i = 0; i < n; i++) {
        auto const& currBlock = basicBlocks[i]; // shared_ptr
        assert(currBlock->instructions.size() > 0);
        auto const& lastInstr = currBlock->instructions.back();

        // connect an edge to the next block if they are contiguous
        if (i < n-1) {
            auto const& nextBlock = basicBlocks[i+1];
            if (Instr::isContiguous(*lastInstr, *(nextBlock->instructions[0])))
            {
                add_edge(currBlock, nextBlock);
            }
        }

        // look at the last instruction in the block. If it is a branch
        // instruction, then extract the targets from the operands, which is
        // anything that is an address (translates to a hex number) or a label
        // (matches <label> string pattern).
        if (branchInstructions.contains(lastInstr->instr)) {
            for (auto const& op : lastInstr->operands) {
                // TODO: cleanup
                try {
                    auto intRep = stoi(op, nullptr, 16);
                    if (addr_map.contains(intRep)) {
                        add_edge(currBlock, addr_map[intRep]);
                    } else if (label_map.contains(op)) {
                        add_edge(currBlock, label_map[op]);
                    }
                } catch (exception const&) {
                    if (label_map.contains(op)) {
                        add_edge(currBlock, label_map[op]);
                    }
                }
            }
        }
    }
}

// find the values of each edge in the adjacency list. We model the whole
// problem as a linear programming problem, satisfying the constraints that
// the sum of the indegree must be equal to the count, and the sum of the
// outdegree must also be equal to the count, and these constraints must be
// satisfied for every basic block.
// We do this by constructing a matrix A and a vector b such that the solution
// vector x satisfies Ax=b.
// void ControlFlowGraph::findEdges()
// {
//     struct Edge
//     {
//         shared_ptr<BasicBlock> start;
//         shared_ptr<BasicBlock> end;
//         uint64_t count = 0; // to be populated
//     };
//     vector<Edge> edges;

//     unordered_map<shared_ptr<BasicBlock>, uint> nodeIdMap;
//     auto numNodes = basicBlocks.size();
//     for (int i = 0; i < numNodes; i++) {
//         nodeIdMap[basicBlocks[i]] = i;
//     }

//     for (auto const& start : basicBlocks) {
//         for (auto const& end : adjList[start]) {
//             edges.push_back(Edge{
//                 .start = start,
//                 .end = end
//             });
//         }
//     }

//     // construct matrix A
//     auto numRows = 2*numNodes; // the nodes (twice)
//     auto numCols = edges.size(); // the edges
//     LOG_INFO(fmt::format("matrix A has {} rows and {} cols", numRows, numCols));

//     Eigen::MatrixXd A(numRows, numCols);
//     for (int j = 0; j < numCols; j++) {
//         auto const& edge = edges[j];
//         // LOG_INFO(fmt::format("{}->{}", nodeIdMap[edge.start], nodeIdMap[edge.end]));
        
//         // iterate through every edge, and populate the matrix A
//         // outwards node
//         A(nodeIdMap[edge.start], j) = 1;

//         // inwards node
//         A(numNodes+nodeIdMap[edge.end], j) = 1;
//     }

//     // populate vector b
//     Eigen::VectorXd b(numRows);
//     for (int i = 0; i < basicBlocks.size(); i++) {
//         // check if the corresponding row in A is a zero row. If so, override
//         // with 0.
//         if (!A.row(i).isZero()) {
//             b(i) = basicBlocks[i]->count;
//         }

//         if (!A.row(numNodes+i).isZero()) {
//             b(numNodes+i) = basicBlocks[i]->count;
//         }
//     }

//     // calculate x
//     // Eigen::VectorXd x = A.colPivHouseholderQr().solve(b);
//     LOG_INFO("solving for x");
//     Eigen::VectorXd x = (A.transpose() * A).ldlt().solve(A.transpose() * b);
//     LOG_INFO(A);
//     LOG_INFO("--------");
//     LOG_INFO(b);
//     LOG_INFO("--------");
//     LOG_INFO(x);
// }

void ControlFlowGraph::findEdges()
{
    LOG_DEBUG("finding edges");
    using namespace operations_research;
    std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("SCIP"));
    if (!solver) {
        LOG_ERROR("SCIP solver unavailable.");
        return;
    }

    const double infinity = solver->infinity();

    vector<Edge> edges;
    unordered_map<Edge, int, EdgeHash> edgeIdMap;
    for (auto const& start : basicBlocks) {
        for (auto const& end : adjList[start]) {
            auto edge = Edge{
                .start = start,
                .end = end
            };
            edges.push_back(edge);
            edgeIdMap[edge] = edges.size()-1;
        }
    }
    LOG_DEBUG(fmt::format("{} edges found", edges.size()));

    for (auto const& edge : edges) {
        LOG_DEBUG(fmt::format("edge: {}", edge.toString()));
    }

    vector<const MPVariable*> vars(edges.size());

    for (int i = 0; i < edges.size(); i++) {
        vars[i] = solver->MakeIntVar(0.0, infinity, edges[i].toString());
    }

    for (int i = 0; i < basicBlocks.size(); i++) {
        auto const& node = basicBlocks[i];

        if (adjList[node].size() > 0) {
            LOG_DEBUG(fmt::format("constraint: outwards on {} sums to {}", node->toString(), node->count));
            // MPVariable* slack = solver->MakeNumVar(0, 10, "slack");
            MPConstraint* const outward = solver->MakeRowConstraint(
                0, node->count, fmt::format("{}-o", i));
            for (auto const& end : adjList[node]) {
                auto index = edgeIdMap[Edge{ .start=node, .end=end }];
                LOG_DEBUG(fmt::format("setting edge {} to 1", edges[index].toString()));
                outward->SetCoefficient(vars[index], 1);
            }
        }

        if (revAdjList[node].size() > 0) {
            LOG_DEBUG(fmt::format("constraint: inwards on {} sums to {}", node->toString(), node->count));
            MPConstraint* const inward = solver->MakeRowConstraint(
                0, node->count, fmt::format("{}-i", i));
            for (auto const& start : revAdjList[node]) {
                auto index = edgeIdMap[Edge{ .start=start, .end=node }];
                LOG_DEBUG(fmt::format("setting edge {} to 1", edges[index].toString()));
                inward->SetCoefficient(vars[index], 1);
            }
        }
    }

    MPObjective* const objective = solver->MutableObjective();
    for (int i = 0; i < edges.size(); i++) {
        objective->SetCoefficient(vars[i], 1); // maximise sum
    }
    objective->SetMaximization();

    const MPSolver::ResultStatus result_status = solver->Solve();
    for (int i = 0; i < edges.size(); i++) {
        edges[i].count = vars[i]->solution_value();
        LOG_INFO(fmt::format("edge={}", edges[i].toString()));
    }
    LOG_DEBUG("finished");
}

} // fusion
