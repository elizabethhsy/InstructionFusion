#include "fusionCalculator.h"
#include "instructionCount.h"

#include <dataRepresentation.h>
#include <exampleRules.h>
#include <experiment.h>
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
        "fusion/core/tests/data.csv";
    auto file = make_shared<File>(path);
    Instr first(
        Addr(0xfc830),
        100,
        "csc",
        {Operand{"cs6"}, Operand{"64(csp)"}},
        "BB_0"
    );

    REQUIRE(file->instructions[0] == first);
    REQUIRE(file->stats->criticalSections[0]->length == 8);

    // once we parsed the file correctly, try to fuse it
    FusionCalculator calculator;
    SECTION("fuse with custom functions", "[fusion]") {
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

        auto results = calculator.calculateFusion(
            file,
            run
        );

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
        FusionRule function(
            [&](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
                -> FusableResult
            {
                return (instruction.instr == "csc") ? FusableResult::FUSABLE :
                    FusableResult::NOT_FUSABLE;
            });

        FusionRule functionMaxLength = maxLength(2).chain(function);

        unordered_set<FusionRulePtr> functions;
        functions.insert(make_shared<FusionRule>(functionMaxLength));
        ExperimentRun run{
            .title = "csc max length 2",
            .rules = functions
        };

        auto results = calculator.calculateFusion(
            file,
            run
        );

        // two pairs of csc fused with each other
        REQUIRE(results.fusedInstructions == 200);
    }

    SECTION("fuse with end instructions", "[fusion]") {
        FusionRule function(
            [&](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
                -> FusableResult
            {
                if (instruction.instr == "csc") return FusableResult::FUSABLE;
                if (instruction.instr == "cincoffset") return FusableResult::END_OF_FUSABLE;
                return FusableResult::NOT_FUSABLE;
            }); // fuse the first 6 instructions

        unordered_set<FusionRulePtr> functions;
        functions.insert(make_shared<FusionRule>(function));
        ExperimentRun run{
            .title = "csc + end cincoffset",
            .rules = functions
        };

        auto results = calculator.calculateFusion(
            file,
            run
        );

        REQUIRE(results.fusedInstructions == 500);
    }

    SECTION("fuse with independent instructions only", "[fusion]") {
        FusionRule fusable(
            [&](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
                -> FusableResult
            {
                unordered_set<string> fusable = {"snez", "and"};
                return (fusable.contains(instruction.instr))
                    ? FusableResult::FUSABLE : FusableResult::NOT_FUSABLE;
            });
        
        unordered_set<FusionRulePtr> functions1;
        functions1.insert(make_shared<FusionRule>(fusable));
        ExperimentRun run1{
            .title = "snez/and",
            .rules = functions1
        };
        auto results1 = calculator.calculateFusion(
            file,
            run1
        );
        REQUIRE(results1.fusedInstructions == 128);

        unordered_set<FusionRulePtr> functions2;
        functions2.insert(make_shared<FusionRule>(
            independent.chain(fusable)
        ));
        ExperimentRun run2{
            .title = "snez/and, independent",
            .rules = functions2
        };
        auto results2 = calculator.calculateFusion(
            file,
            run2
        );
        REQUIRE(results2.fusedInstructions == 0);
    }

    SECTION("multiple fusion rules", "[fusion]") {
        FusionRule cspecialrFunction(
            [&](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
                -> FusableResult
            {
                if (instruction.instr == "cincoffset")
                    return FusableResult::FUSABLE;
                if (instruction.instr == "cspecialr")
                    return FusableResult::END_OF_FUSABLE;
                return FusableResult::NOT_FUSABLE;
            });

        FusionRule cldFunction(
            [&](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
                -> FusableResult
            {
                if (instruction.instr == "cincoffset")
                    return FusableResult::FUSABLE;
                if (instruction.instr == "cld")
                    return FusableResult::END_OF_FUSABLE;
                return FusableResult::NOT_FUSABLE;
            });

        unordered_set<FusionRulePtr> functions = {
            make_shared<FusionRule>(maxLength(3).chain(cspecialrFunction)),
            make_shared<FusionRule>(maxLength(3).chain(cldFunction))
        };

        ExperimentRun run{
            .title = "cincoffset",
            .rules = functions
        };

        auto results = calculator.calculateFusion(
            file,
            run
        );

        REQUIRE(results.fusedInstructions == 112);
    }

    SECTION("same count", "[fusion]") {
        FusionRule function(
            [&](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
                -> FusableResult
            {
                return (instruction.instr == "csc") ? FusableResult::FUSABLE :
                    FusableResult::NOT_FUSABLE;
            });

        unordered_set<FusionRulePtr> functions;
        functions.insert(make_shared<FusionRule>(sameCount.chain(function)));
        ExperimentRun run{
            .title = "csc",
            .rules = functions
        };

        auto results = calculator.calculateFusion(
            file,
            run
        );

        REQUIRE(results.fusedInstructions == 400);
    }
}

}
