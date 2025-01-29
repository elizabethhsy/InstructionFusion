#include "csvHandler.h"
#include "dataRepresentation.h"

#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fusion
{

using namespace std;

CSVWriter::CSVWriter(string fullPath) : stream(fullPath)
{
    // create the directory if necessary
    boost::filesystem::path dir(fullPath);
    if(!(boost::filesystem::exists(dir))){
        if (!boost::filesystem::create_directory(dir)) {
            LOG_ERROR("failed to create directory {}");
            return;
        }
    }

    // create the file if necessary
    if (!stream.is_open()) {
        LOG_ERROR(
            fmt::format(
                "{} cannot be opened, check that the file exists.",
                fullPath
            )
        );
    }
}

void CSVWriter::writeLine(string data)
{
    stream << data << "\n";
}

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
