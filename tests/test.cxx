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

TEST_CASE("test fusion", "[fusion]") {
    string path = "/Users/elizabeth/Desktop/Cambridge/Dissertation/"
        "tests/test_data.csv";
    File file(path);
    Instr first = {
        .addr = 0xfc830,
        .count = 100,
        .instr = "csc",
        .operands = {Operand{"cs6"}, Operand{"64(csp)"}}
    };

    REQUIRE(file.instructions[0] == first);
    REQUIRE(file.stats->criticalSections[0]->length == 8);

    // once we parsed the file correctly, try to fuse it
    FusionCalculator calculator;
    SECTION("basic fusion", "[fusion]") {
        auto results = calculator.calculateFusion(
            file,
            FusionConfig{
                .fusableName = "csc",
                .fusable = { "csc" }
            }
        ); // only the first 5 instructions should be fused

        REQUIRE(results.totalInstructions >= results.instructionsAfterFuse);
        REQUIRE(results.fusedInstructions ==
            results.totalInstructions - results.instructionsAfterFuse);
        REQUIRE(results.fusedInstructions == 400);
        // floating point comparison
        REQUIRE_THAT(
            results.fusedPercentage,
            Catch::Matchers::WithinRel(400.0/2810*100)
        );
    }

    SECTION("limit maximum fusable length", "[fusion]") {
        auto results = calculator.calculateFusion(
            file,
            FusionConfig {
                .fusableName = "csc",
                .fusable = { "csc" },
                .maxFusableLength = 2
            }
        );

        // two pairs of csc fused with each other
        REQUIRE(results.fusedInstructions == 200);
    }

    SECTION("fuse with end instructions", "[fusion]") {
        auto results = calculator.calculateFusion(
            file,
            FusionConfig {
                .fusableName = "csc",
                .fusable = { "csc" },
                .endName = "cincoffset",
                .end = { "cincoffset" }
            }
        ); // fuse the first 6 instructions

        REQUIRE(results.fusedInstructions == 500);
    }

    SECTION("fuse with independent instructions only", "[fusion]") {
        auto results1 = calculator.calculateFusion(
            file,
            FusionConfig {
                .fusableName = "snez, and",
                .fusable = { "snez", "and" },
                .independentInstructionsOnly = true
            }
        );

        auto results2 = calculator.calculateFusion(
            file,
            FusionConfig {
                .fusableName = "snez, and",
                .fusable = { "snez", "and" }
            }
        );

        REQUIRE(results1.fusedInstructions == 64);
        REQUIRE(results2.fusedInstructions == 128);
    }
}

}
