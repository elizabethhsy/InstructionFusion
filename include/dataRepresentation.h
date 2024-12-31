#pragma once

#include "macros.h"

#include <boost/algorithm/string/join.hpp>
#include <fmt/core.h>
#include <stdint.h>
#include <string>
#include <unordered_set>
#include <vector>

namespace fusion
{

using namespace std;
using Operand = string;

struct Instr
{
    uint32_t addr;
    uint64_t count;
    string instr;
    vector<Operand> operands;

    string toString() const;

    static unordered_set<Operand> dependentOperands(
        Instr const& first,
        Instr const& second
    );

    static Instr parseInstruction(string const& str);
    static bool isContiguous(Instr const& prevInstr, Instr const& nextInstr)
    {
        return (nextInstr.addr - prevInstr.addr == 2 ||
            nextInstr.addr - prevInstr.addr == 4);
    }

    bool operator<(Instr const& other) const {
        return addr < other.addr;
    }

    bool operator==(Instr const& other) const {
        bool operands_equal = true;
        for (int i = 0; i < operands.size(); i++) {
            operands_equal &= (operands[i] == other.operands[i]);
        }
        return addr == other.addr && count == other.count &&
            instr == other.instr && operands_equal;
    }
};
ostream& operator<<(ostream& os, Instr const& instr);

struct CriticalSection
{
    uint length;
    uint count;
    const Instr* start;

    CriticalSection(uint length, uint count, const Instr* start);
};

struct File;
struct FileStats
{
public:
    File const* file; // read only
    vector<unique_ptr<CriticalSection>> criticalSections;

    FileStats(File const* file);
    float avgCriticalSectionSize;
    uint64_t totalInstructionNum;

    string toString();
    void calculateStats();
private:
    bool initialised;

    void constructCriticalSections(vector<string> delimiters);
    float calculateAvgCriticalSectionSize();
    uint64_t calculateTotalInstructionNum();
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
    bool checkContiguousInstructions();
    string getNameFromPath(string const& filepath);
};

}
