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
        "fusion/tests/test_data.csv";
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
    SECTION("fuse with custom functions", "[fusion]") {
        FusionRule function =
            [&](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
                -> FusableResult
        {
            return (instruction.instr == "csc") ? FusableResult::FUSABLE :
                FusableResult::NOT_FUSABLE;
        };

        unordered_set<FusionRulePtr> functions;
        functions.insert(make_shared<FusionRule>(function));
        ExperimentRun run{"csc", functions};

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
        FusionRule function =
            [&](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
                -> FusableResult
            {
                return (instruction.instr == "csc") ? FusableResult::FUSABLE :
                    FusableResult::NOT_FUSABLE;
            };

        FusionRule functionMaxLength =
            [&](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
                -> FusableResult
            {
                return maxLength(2, make_shared<FusionRule>(FusionRule(function)), block, instruction);
            };

        unordered_set<FusionRulePtr> functions;
        functions.insert(make_shared<FusionRule>(functionMaxLength));
        ExperimentRun run{"csc max length 2", functions};

        auto results = calculator.calculateFusion(
            file,
            run
        );

        // two pairs of csc fused with each other
        REQUIRE(results.fusedInstructions == 200);
    }

    SECTION("fuse with end instructions", "[fusion]") {
        FusionRule function =
            [&](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
                -> FusableResult
            {
                if (instruction.instr == "csc") return FusableResult::FUSABLE;
                if (instruction.instr == "cincoffset") return FusableResult::END_OF_FUSABLE;
                return FusableResult::NOT_FUSABLE;
            }; // fuse the first 6 instructions

        unordered_set<FusionRulePtr> functions;
        functions.insert(make_shared<FusionRule>(function));
        ExperimentRun run{"csc + end cincoffset", functions};

        auto results = calculator.calculateFusion(
            file,
            run
        );

        REQUIRE(results.fusedInstructions == 500);
    }

    SECTION("fuse with independent instructions only", "[fusion]") {
        FusionRule fusable =
            [&](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
                -> FusableResult
            {
                unordered_set<string> fusable = {"snez", "and"};
                return (fusable.contains(instruction.instr))
                    ? FusableResult::FUSABLE : FusableResult::NOT_FUSABLE;
            };
        
        FusionRule independent =
            [&](vector<shared_ptr<Instr>> const& block, Instr const& instruction)
                -> FusableResult
            {
                // LOG_DEBUG(fmt::format("block_size={}", block.size()));
                unordered_set<Operand> operands;
                for (auto instr : block) {
                    // LOG_DEBUG(fmt::format("instruction={}", instr->toString()));
                    for (auto op : instr->operands) {
                        if (operands.contains(op)) {
                            // LOG_DEBUG(fmt::format("prefix block is not fusable"));
                            return FusableResult::NOT_FUSABLE;
                        }
                        operands.insert(op);
                    }
                }

                for (auto op : instruction.operands) {
                    if (operands.contains(op)) return FusableResult::NOT_FUSABLE;
                    operands.insert(op);
                }
                return fusable(block, instruction);
            };
        
        unordered_set<FusionRulePtr> functions1;
        unordered_set<FusionRulePtr> functions2;
        functions1.insert(make_shared<FusionRule>(fusable));
        functions2.insert(make_shared<FusionRule>(independent));
        ExperimentRun run1{"snez/and", functions1};
        ExperimentRun run2{"snez/and, independent", functions2};

        auto results1 = calculator.calculateFusion(
            file,
            run1
        );

        auto results2 = calculator.calculateFusion(
            file,
            run2
        );

        REQUIRE(results1.fusedInstructions == 128);
        REQUIRE(results2.fusedInstructions == 0);
    }
}

}
