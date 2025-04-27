#include "fileHandler.h"
#include "dataRepresentation.h"

#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fusion
{

using namespace std;

FileWriter::FileWriter(string fullPath)
{
    // create the directory if necessary
    boost::filesystem::path filePath(fullPath);
    auto dirPath = filePath.parent_path();
    if(!(boost::filesystem::exists(dirPath))){
        if (!boost::filesystem::create_directories(dirPath)) {
            LOG_ERROR(
                fmt::format("failed to create directory {}", dirPath.string())
            );
            return;
        }
    }

    // create the file if necessary
    stream.open(fullPath);
    if (!stream.is_open()) {
        LOG_ERROR(
            fmt::format(
                "{} cannot be opened, check that the file exists.",
                fullPath
            )
        );
    }
}

void FileWriter::writeLine(string data)
{
    stream << data << "\n";
}

// TODO: close stream on destructor

vector<Instr> CSVManager::loadInstructions(string fullPath)
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

} // fusion
