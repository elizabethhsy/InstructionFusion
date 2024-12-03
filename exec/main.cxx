#include <csvHandler.h>
#include <experiment.h>
#include <fusion.h>
#include <instructions.h>

#include <fmt/core.h>
#include <string>
#include <vector>

int main(int argc, char const *argv[])
{
    std::vector<std::string> fileNames = {
        "astar",
        "bzip2",
        "gobmk",
        "h264ref",
        "hmmer",
        "libquantum",
        "mcf",
        "omnetpp",
        "sjeng",
        "xalancbmk"
    };

    std::string dirPath =
        "/Users/elizabeth/Desktop/Cambridge/Dissertation/cleaned data";
    std::string resultsPath =
        "/Users/elizabeth/Desktop/Cambridge/Dissertation/data results";
        
    for (auto& name : fileNames) {
        name = fmt::format("{}/{}.csv", dirPath, name);
    }

    std::vector<fusion::FusionConfig> configs = {
        fusion::FusionConfig {
            .fusableName = "ARITHMETIC",
            .fusable = fusion::arithmeticInstructions,
            .endName = "MEMORY",
            .end = fusion::memoryInstructions
        },
        fusion::FusionConfig {
            .fusableName = "ARITHMETIC",
            .fusable = fusion::arithmeticInstructions,
            .endName = "BRANCH",
            .end = fusion::branchInstructions
        }
    };

    fusion::Experiment experiment(fileNames, configs);
    std::vector<fusion::FusionResults> results = experiment.run();
    fusion::CSVHandler::writeResultsToCSV(
        results,
        resultsPath + "/overview.csv",
        resultsPath + "/fusionLengths.csv"
    );
    return 0;
}
