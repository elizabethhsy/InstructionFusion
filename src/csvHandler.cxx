#include "csvHandler.h"
#include "dataRepresentation.h"

#include <fstream>
#include <string>
#include <vector>

namespace fusion
{

using namespace std;

vector<Instr> CSVHandler::loadInstructionsFromCSV(string fullPath)
{
    ifstream file(fullPath);
    vector<Instr> instructions;

    if (!file.is_open()) {
        LOG_ERROR(
            fmt::format(
                "{} cannot be opened, check that the file exists.",
                fullPath
            )
        );
    }

    string line;
    // ignore header line
    getline(file, line);
    while (getline(file, line)) {
        Instr instr = Instr::parseInstruction(line);
        instructions.push_back(std::move(instr));
    }

    sort(instructions.begin(), instructions.end());
    return instructions;
}

void CSVHandler::writeResultsToCSV(
    vector<FusionResults> const& results,
    vector<FusionResults> const& aggregate_results,
    string aggregateCSV,
    string overviewCSV,
    string fusionLengthsCSV
)
{
    ofstream aggregateFile(aggregateCSV);
    if (!aggregateFile.is_open()) {
        LOG_ERROR(
            fmt::format(
                "{} cannot be opened, check that the file exists.",
                aggregateCSV
            )
        );
        return;
    }
    aggregateFile << "fusable,end,max_fusable_length,total_instructions,"
        "instructions_after_fuse,instructions_fused,percentage_fused,"
        "average_fusion_length\n";
    for (auto result : aggregate_results) {
        aggregateFile << fmt::format(
            "{},{},{},{},{},{},{},{}\n",
            result.config.fusableName,
            result.config.endName,
            result.config.maxFusableLength,
            result.totalInstructions,
            result.instructionsAfterFuse,
            result.fusedInstructions,
            result.fusedPercentage,
            result.avgFusionLength
        );
    }


    ofstream overviewFile(overviewCSV);

    if (!overviewFile.is_open()) {
        LOG_ERROR(
            fmt::format(
                "{} cannot be opened, check that the file exists.",
                overviewCSV
            )
        );
        return;
    }


    // write header line with columns:
    // file, fusable, end, max_fusable_length, total_instructions,
    // instructions_after_fuse, instructions_fused, percentage_fused,
    // average_fusion_length
    overviewFile << "file,fusable,end,max_fusable_length,total_instructions,"
        "instructions_after_fuse,instructions_fused,percentage_fused,"
        "average_fusion_length\n";

    for (auto result : results) {
        overviewFile << fmt::format(
            "{},{},{},{},{},{},{},{},{}\n",
            result.file.fileName,
            result.config.fusableName,
            result.config.endName,
            result.config.maxFusableLength,
            result.totalInstructions,
            result.instructionsAfterFuse,
            result.fusedInstructions,
            result.fusedPercentage,
            result.avgFusionLength
        );
    }

    // write header line with columns:
    // file, fusable, end, max_fusable_length, count, fusion_length
    ofstream fusionLengthFile(fusionLengthsCSV);

    if (!fusionLengthFile.is_open()) {
        LOG_ERROR(
            fmt::format(
                "{} cannot be opened, check that the file exists.",
                fusionLengthsCSV
            )
        );
        return;
    }

    fusionLengthFile << "file,fusable,end,max_fusable_length,count,"
        "fusion_length\n";
    
    for (auto result : results) {
        for (auto pair : result.fusionLengths) { // pair of count, length
            auto count = pair.first;
            auto length = pair.second;

            fusionLengthFile << fmt::format(
                "{},{},{},{},{},{}\n",
                result.file.fileName,
                result.config.fusableName,
                result.config.endName,
                result.config.maxFusableLength,
                count,
                length
            );
        }
    }
}

} // namespace fusion
