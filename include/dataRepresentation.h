#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace fusion
{

using namespace std;

struct Instr
{
    uint32_t addr;
    uint64_t count;
    string instr;
    vector<string> operands;
};

struct FileStats
{
    float avgCriticalSectionSize;
    unsigned int totalInstructionNum;
};

struct File
{
    string fileName;
    FileStats stats;
    vector<Instr> instructions;
    vector<CriticalSection> criticalSections;

    void calculateStats();
};

struct CriticalSection
{
    unsigned int length;
    Instr* start;
};

struct Experiment
{
public:
    // constructor - takes in list of file names and parses the corresponding
    // csv files
    Experiment(vector<string> filenames);
private:
    vector<File> files;
};

}