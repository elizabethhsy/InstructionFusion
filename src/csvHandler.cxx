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

}
