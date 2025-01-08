#include "csvHandler.h"
#include "dataRepresentation.h"
#include "experiment.h"

#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
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
    ExperimentResults const& results,
    string resultsPath
)
{
    string aggregateCSV = resultsPath + "/aggregate.csv";
    string overviewCSV = resultsPath + "/overview.csv";
    string fusionLengthsCSV = resultsPath + "/fusionLengths.csv";

    boost::filesystem::path dir(resultsPath);
    if(!(boost::filesystem::exists(dir))){
        if (!boost::filesystem::create_directory(dir)) {
            LOG_ERROR("failed to create directory {}");
            return;
        }
    }

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
    aggregateFile << "rule_title,rule_description,total_instructions,"
        "instructions_after_fuse,instructions_fused,percentage_fused,"
        "average_fusion_length\n";
    
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
    overviewFile << "rule_title,rule_description,file,total_instructions,"
        "instructions_after_fuse,instructions_fused,percentage_fused,"
        "average_fusion_length\n";

    for (auto const& run : results.runResults) {
        for (auto const& res : run.fusionResults) {
            overviewFile << fmt::format(
                "{},{},{},{},{},{},{},{}\n",
                res.run.title,
                res.run.description,
                res.file.fileName,
                res.totalInstructions,
                res.instructionsAfterFuse,
                res.fusedInstructions,
                res.fusedPercentage,
                res.avgFusionLength
            );
        }

        aggregateFile << fmt::format(
            "{},{},{},{},{},{},{}\n",
            run.aggregateResults.run.title,
            run.aggregateResults.run.description,
            run.aggregateResults.totalInstructions,
            run.aggregateResults.instructionsAfterFuse,
            run.aggregateResults.fusedInstructions,
            run.aggregateResults.fusedPercentage,
            run.aggregateResults.avgFusionLength
        );
    }

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
    fusionLengthFile << "rule_title,file,rule_description,count,"
        "fusion_length\n";

    for (auto const& run : results.runResults) {
        for (auto const& res : run.fusionResults) {
            for (auto const& pair : res.fusionLengths) { // pair of count, length
                fusionLengthFile << fmt::format(
                    "{},{},{},{},{}\n",
                    res.run.title,
                    res.run.description,
                    res.file.fileName,
                    pair.first, // count
                    pair.second // length
                );
            }
        }
    }
}

} // namespace fusion
