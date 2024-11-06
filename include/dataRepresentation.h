#pragma once

#include "macros.h"

#include <fmt/core.h>
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
    static bool isContiguous(Instr const& prevInstr, Instr const& nextInstr)
    {
        return (nextInstr.addr - prevInstr.addr == 2 ||
            nextInstr.addr - prevInstr.addr == 4);
    }

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
    bool checkContiguousInstructions();
    string getNameFromPath(string const& filepath);
};

struct FusionCalculator
{
    uint calculateFusion(
        File const& file,
        vector<string> const& delimiters
    );
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
