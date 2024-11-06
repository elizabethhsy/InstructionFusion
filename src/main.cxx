#include "dataRepresentation.h"

#include <fmt/core.h>
#include <string>
#include <vector>

int main(int argc, char const *argv[])
{
    /* code */
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
    
    for (auto& name : fileNames) {
        name = fmt::format("{}/{}.csv", dirPath, name);
    }

    fusion::Experiment experiment(fileNames);
    return 0;
}
