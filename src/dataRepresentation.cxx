#include "dataRepresentation.h"
#include "macros.h"

#include <boost/algorithm/string.hpp>
#include <cassert>
#include <fmt/core.h>
#include <fstream>
#include <regex>
#include <sstream>
#include <vector>

namespace fusion
{

using namespace std;

Instr Instr::parseInstruction(string const& str)
{
    // split string into groups based on csv line:
    // address, count, instruction, operands

    stringstream strStream(str);
    string cell;
    vector<string> tokens;

    while (getline(strStream, cell, ',')) {
        tokens.push_back(cell);
    }

    if (str.back() == ',') { // empty string at the end
        tokens.push_back("");
    }

    assert(tokens.size() == 4);
    Instr instr;
    instr.addr = stoi(tokens[0], nullptr, 16);
    instr.count = stoi(tokens[1]);
    instr.instr = tokens[2];

    stringstream operands(tokens[3]);
    while (getline(operands, cell, ' ')) {
        instr.operands.push_back(cell);
    }

    return instr;
}

CriticalSection::CriticalSection(uint length, uint count, Instr* start)
    : length(length), count(count), start(start) {}

FileStats::FileStats(File const* file)
    : file(file)
{
    initialised = false;
    constructCriticalSections(delimiters);
}

string FileStats::toString()
{
    return fmt::format(
        "avgCriticalSectionSize={}, totalInstructionNum={}",
        avgCriticalSectionSize,
        totalInstructionNum
    );
}

// populate the member variables
void FileStats::calculateStats()
{
    // assume that the instructions are already populated
    assertm(
        !file->instructions.empty(),
        fmt::format(
            "the instructions from file {} have not been loaded",
            file->fileName
        )
    );

    avgCriticalSectionSize = calculateAvgCriticalSectionSize();
    totalInstructionNum = file->instructions.size();
    initialised = true;
}

void FileStats::constructCriticalSections(vector<string> delimiters)
{
    uint length, count;
    bool startOfCriticalSection;

    vector<Instr> instructions = file->instructions;
    if (instructions.empty()) {
        LOG_ERROR(
            fmt::format(
                "No instructions have been loaded into {}",
                file->fileName
            )
        );
    }

    auto reset = [&length, &count, &startOfCriticalSection]() {
        length = 0;
        // we take the minimum count, but print if the counts are different
        count = UINT_MAX;
        startOfCriticalSection = true;
    };

    // iterate through all the instructions. If it's a branch/exception
    // instruction, then record it as the end of a critical section, and
    // record the length
    reset();
    Instr* startInstr;
    int n = instructions.size();
    for (int i = 0; i < n; i++) {
        auto const& prevInstruction = instructions[max(i-1, 0)];
        auto& instruction = instructions[i];
        auto const& nextInstruction = instructions[min(n-1, i+1)];

        length++;

        if (startOfCriticalSection) {
            startInstr = &instruction;
        }

        if (instruction.count < count) {
            if (count != UINT_MAX) {
                LOG_INFO(
                    fmt::format(
                        "instruction count of {} is lower than previous count "
                        "of {}, previous instruction is {}",
                        instruction.count,
                        count,
                        prevInstruction.instr
                    )
                );
            }
            count = instruction.count;
        }

        int diff = abs(static_cast<int>(instruction.count - prevInstruction.count));
        if (
            (find(delimiters.begin(), delimiters.end(), instruction.instr)
            != delimiters.end()) &&
            (diff >= 1)
            )
        {
            // record new critical section
            criticalSections.push_back(
                make_unique<CriticalSection>(length, count, startInstr)
            );
            reset();
        }
    }
}

float FileStats::calculateAvgCriticalSectionSize()
{
    float total = 0;
    float count = 0;
    for (auto const& section : criticalSections) {
        total += section->length * section->count;
        count += section->count;
    }
    return total/count;
}

const vector<string> FileStats::delimiters = {
    "beq",
    "bne",
    "blt",
    "bge",
    "bltu",
    "bgeu",
    "cjal",
    "cjalr",
    "j",
    "jalr",
    "beqz",
    "bnez",
    "blez",
    "bgez",
    "bltz",
    "bgtz",
    "c.bnez",
    "c.beqz",
    "jr",
    "ret",
    "cld",
    "csw",
    "clh",
    "csc",
    "csd",
    "csh",
    "clhu",
    "csb",
    "cfsd",
    "clc",
    "clw",
    "cflw",
    "clbu",
    "fcvt.d.l"
};

File::File(string filename)
{
    fileName = filename;
    // find file with that name in the directory
    // if filePath is not specified, assume we're looking in the same directory
    // otherwise filePath should refer to the absolute path which the file is in
    // if the file doesn't exist, then we throw an exception. Otherwise, we read
    // the CSV and populate the instruction vector.
    loadFromCSV();
    stats = make_unique<FileStats>(this);
}

void File::loadFromCSV()
{
    ifstream file(fileName);

    if (!file.is_open()) {
        LOG_ERROR(
            fmt::format(
                "{} cannot be opened, check that the file exists.",
                fileName
            )
        );
    }

    string line;
    // ignore header line
    getline(file, line);
    while (getline(file, line)) {
        Instr instr = Instr::parseInstruction(line);
        instructions.push_back(move(instr));
    }

    sort(instructions.begin(), instructions.end());
}

Experiment::Experiment(vector<string> filenames)
{
    // create the necessary file objects
    for (auto filename : filenames) {
        files.push_back(make_unique<File>(filename));
    }

    // for each file, calculate the statistics and print them out
    for (auto const& file : files) {
        file->stats->calculateStats();

        // print stats
        LOG_INFO(
            fmt::format(
                "stats for file {}: {}",
                file->fileName,
                file->stats->toString()
            )
        );
    }
}

}
