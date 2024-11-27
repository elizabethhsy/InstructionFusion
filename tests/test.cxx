#include <dataRepresentation.h>
#include <fusion.h>
#include <instructions.h>
#include <macros.h>

#include <catch2/catch_test_macros.hpp>
#include <string>

namespace fusion
{

using namespace std;

TEST_CASE("test fusion", "[fusion]") {
    string path = "/Users/elizabeth/Desktop/Cambridge/Dissertation/"
        "tests/test_data.csv";
    File file(path);
    Instr first = {
        .addr = 0xfc830,
        .count = 100,
        .instr = "csc",
        .operands = {"cs6", "64(csp)"}
    };

    REQUIRE(file.instructions[0] == first);

    // once we parsed the file correctly, try to fuse it
    FusionCalculator calculator;
    auto results = calculator.calculateFusion(
        file,
        FusionConfig{ .fusable = { "csc" }, .maxFusableLength=UINT64_MAX }
    ); // only the first 5 instructions should be fused

    REQUIRE(results.totalInstructions >= results.instructionsAfterFuse);
    auto instructionsSaved = results.totalInstructions -
        results.instructionsAfterFuse;
    REQUIRE(instructionsSaved == 400);
}

}
