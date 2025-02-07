#include "histogramParser.h"

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

}
