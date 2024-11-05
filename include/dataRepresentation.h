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

struct CriticalSection
{
    unsigned int length;
    Instr* start;
};

struct FileStats
{
public:
    File const& file;
    vector<CriticalSection> criticalSections;

    FileStats(File const& file);
private:
    float avgCriticalSectionSize;
    unsigned int totalInstructionNum;
    
    float getAvgCriticalSectionSize();
    unsigned int getTotalInstructionNum();
};

struct File
{
public:
    string fileName;
    FileStats stats;
    vector<Instr> instructions;

    File(string filePath, string fileName);
private:
    void calculateStats();
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