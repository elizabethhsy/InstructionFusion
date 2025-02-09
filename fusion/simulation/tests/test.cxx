#include "histogramParser.h"
#include "inOrderPipeline.h"

#include <fusionCalculator.h>

#include <dataRepresentation.h>
#include <fusion.h>
#include <instructions.h>
#include <macros.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <string>

namespace fusion
{

using namespace std;

TEST_CASE("construct control flow graph", "[simulation]") {
    string path = "/Users/elizabeth/Desktop/Cambridge/Dissertation/"
        "fusion/simulation/tests/test_data.csv";
    File file(path);

    // use the file to construct the flow graph
    auto graph = ControlFlowGraph(file);

    // REQUIRE(graph.basicBlocks.size() == 5);

    // auto paths = graph.computePaths();
    // REQUIRE(paths.size() == 4);
}

TEST_CASE("cycle count", "[simulation]") {
    string path = "/Users/elizabeth/Desktop/Cambridge/Dissertation/"
        "fusion/simulation/tests/data.csv";
    File file(path);

    FusionCalculator calculator;
    InOrderPipeline pipeline;

    FusionRule function(
        [&](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
            -> FusableResult
        {
            return (instruction.instr == "csc") ? FusableResult::FUSABLE :
                FusableResult::NOT_FUSABLE;
        });

    unordered_set<FusionRulePtr> functions;
    functions.insert(make_shared<FusionRule>(function));
    ExperimentRun run{
        .title = "csc",
        .rules = functions
    };

    auto instrResults = calculator.calculateFusion(
        file,
        run
    );

    REQUIRE(instrResults.fusedInstructions == 400);

    // get the vector of fused blocks and calculate the cycle count
    auto cycleResults = pipeline.computeCycleCount(instrResults);
    REQUIRE(cycleResults.cyclesWithoutStalls ==
        instrResults.instructionsAfterFuse);
    REQUIRE(cycleResults.stalls == 76);
    REQUIRE(cycleResults.totalCycles == cycleResults.cyclesWithoutStalls +
        cycleResults.stalls);
}

}
