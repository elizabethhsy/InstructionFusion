#include "csvHandler.h"
#include "dataRepresentation.h"
#include "instructions.h"
#include "macros.h"

#include <boost/algorithm/string.hpp>
#include <cassert>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <iterator>
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

string Instr::toString() const
{
    return fmt::format(
        "Instr: addr={:#04x}, count={}, instr={}, operands={}",
        addr,
        count,
        instr,
        boost::algorithm::join(operands, ", ")
    );
}

ostream& operator<<(ostream& os, Instr const& instr) {
    os << instr.toString();
    return os;
}

CriticalSection::CriticalSection(uint length, uint count, const Instr* start)
    : length(length), count(count), start(start) {}

FileStats::FileStats(File const* file)
    : file(file)
{
    initialised = false;
    constructCriticalSections(branchInstructions);
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
    totalInstructionNum = calculateTotalInstructionNum();
    initialised = true;
}

void FileStats::constructCriticalSections(vector<string> delimiters)
{
    uint length, count;
    bool startOfCriticalSection;

    auto& instructions = file->instructions;
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
    const Instr* startInstr;
    int n = instructions.size();
    for (int i = 0; i < n; i++) {
        auto const& prevInstruction = instructions[max(i-1, 0)];
        auto& instruction = instructions[i];
        auto const& nextInstruction = instructions[min(n-1, i+1)];

        length++;

        if (startOfCriticalSection) {
            startInstr = &instruction;
            startOfCriticalSection = false;
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

        int diff = abs(static_cast<int>(instruction.count - nextInstruction.count));
        if (
            (find(delimiters.begin(), delimiters.end(), instruction.instr)
            != delimiters.end()) || diff >= 1 ||
            (!Instr::isContiguous(prevInstruction, instruction) && i > 0) ||
            (i == n-1) // end of the file
            )
        {
            LOG_DEBUG(
                fmt::format(
                    "start_instruction={}\nend_instruction={}\n",
                    startInstr->toString(),
                    instruction.toString()
                )
            );
            // record new critical section
            criticalSections.push_back(
                make_unique<CriticalSection>(length, count, startInstr)
            );
            reset();
        }
    }

    uint total = 0;
    for (auto& criticalSection : criticalSections) {
        total += criticalSection->length*criticalSection->count;
    }
    // LOG_INFO(fmt::format("Critical Section: before={}, after={}", calculateTotalInstructionNum(), total));
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

uint64_t FileStats::calculateTotalInstructionNum()
{
    uint64_t total = 0;

    for (auto const& instruction : file->instructions) {
        total += instruction.count;
    }

    return total;
}

File::File(string filepath)
{
    fullPath = filepath;
    fileName = getNameFromPath(filepath);
    // find file with that name in the directory
    // if filePath is not specified, assume we're looking in the same directory
    // otherwise filePath should refer to the absolute path which the file is in
    // if the file doesn't exist, then we throw an exception. Otherwise, we read
    // the CSV and populate the instruction vector.

    instructions = CSVHandler::loadInstructionsFromCSV(fullPath);
    stats = make_unique<FileStats>(this);
    stats->calculateStats();
}

string File::getNameFromPath(string const& filepath)
{
    filesystem::path path(filepath);
    return path.stem().string();
}

bool File::checkContiguousInstructions()
{
    bool contiguous = true;
    for (int i = 1; i < instructions.size(); i++) {
        auto currAddr = instructions[i].addr;
        auto prevAddr = instructions[i-1].addr;
        if (currAddr - prevAddr != 2 && currAddr - prevAddr != 4) {
            LOG_ERROR(
                fmt::format(
                    "file {}: instruction {} is more than one instruction away "
                    "from previous instruction {}: {:x}-{:x}={}",
                    fileName,
                    instructions[i].instr,
                    instructions[i-1].instr,
                    currAddr,
                    prevAddr,
                    currAddr-prevAddr
                )
            );
            contiguous = false;
        }
    }
    return contiguous;
}

}
