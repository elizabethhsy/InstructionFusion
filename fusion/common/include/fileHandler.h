#pragma once

#include "dataRepresentation.h"
#include "experiment.h"
#include "fusion.h"

#include <fstream>
#include <vector>

namespace fusion
{

using namespace std;

struct FileWriter
{
    FileWriter(string fullPath);

    void writeLine(string data); // write data to CSV
private:
    string fullPath;
    ofstream stream;
};

struct CSVWriter : FileWriter
{
    CSVWriter(string fullPath);

    template<typename... Args>
    void writeLine(Args... args);
};

// deal with all things CSV related
struct CSVManager
{
    static vector<Instr> loadInstructions(string fullPath);
};

} // fusion
