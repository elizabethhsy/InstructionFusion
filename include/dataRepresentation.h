#pragma once

#include "macros.h"

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

    static Instr parseInstruction(string const& str);

    bool operator<(const Instr& other) const {
        return addr < other.addr;
    }
};

struct CriticalSection
{
    uint length;
    uint count;
    Instr* start;

    CriticalSection(uint length, uint count, Instr* start);
};

struct File;
struct FileStats
{
public:
    File const* file; // read only
    vector<unique_ptr<CriticalSection>> criticalSections;

    FileStats(File const* file);

    void calculateStats();
    string toString();
private:
    bool initialised;
    float avgCriticalSectionSize;
    unsigned int totalInstructionNum;
    static const vector<string> delimiters;

    void constructCriticalSections(vector<string> delimiters);
    float calculateAvgCriticalSectionSize();
};

struct File
{
public:
    string fileName;
    string fullPath;
    unique_ptr<FileStats> stats;
    vector<Instr> instructions;

    File(string filename);
    bool fileExists();
private:
    void loadFromCSV();
};

struct Experiment
{
public:
    // constructor - takes in list of file names and parses the corresponding
    // csv files
    Experiment(vector<string> filenames);
private:
    vector<unique_ptr<File>> files;
};

}
