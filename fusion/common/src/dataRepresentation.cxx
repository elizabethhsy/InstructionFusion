#include "fileHandler.h"
#include "dataRepresentation.h"
#include "instructions.h"
#include "macros.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <iterator>
#include <optional>
#include <ranges>
#include <regex>
#include <string>
#include <sstream>
#include <unordered_set>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/transformed.hpp>

namespace fusion
{

using namespace std;

uint32_t Operand::getOffset()
{
    regex pattern(R"([0-9]+)\((.*)\)");
    smatch match;
    assert(regex_search(op, match, pattern));
    return stoul(match[0]);
}

string Operand::toString() const
{
    return op;
}

string Addr::toString() const
{
    return fmt::format("{:#04x}", addr);
}

uint BasicBlock::counter = 0;

string BasicBlock::toString() const
{
    return InstrBlock::toString("Basic Block");
}

// check if two instructions belong in the same critical section
bool BasicBlock::sameBasicBlock(
    Instr const& prev,
    Instr const& next
)
{
    return (prev.count == next.count && !branchInstructions.contains(prev.instr)
        && Instr::isContiguous(prev, next)) || (prev == next);
}

bool Instr::dependentOperands(
    Instr const& instruction,
    unordered_set<Operand> const& dependentOperands
)
{
    for (auto const& op : instruction.operands) {
        if (dependentOperands.contains(op)) {
            return true;
        }
    }
    return false;
}

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

    assert(tokens.size() == 5);
    Instr instr(Addr(tokens[0]), stoi(tokens[1]), tokens[3], {}, tokens[2]);
    // instr.addr = Addr(tokens[0]);
    // instr.count = stoi(tokens[1]);
    // instr.label = tokens[2];
    // instr.instr = tokens[3];

    stringstream operands(tokens[4]);
    while (getline(operands, cell, ' ')) {
        if (cell == "") continue;
        instr.operands.push_back(Operand{cell});
    }

    return instr;
}

string Instr::toString() const
{
    return fmt::format(
        "Instr: addr={}, count={}, label={}, instr={}, operands=[{}]",
        addr.toString(),
        count,
        label,
        instr,
        boost::algorithm::join(
            operands |
            boost::adaptors::transformed(
                [](Operand const& operand)
                {
                    return operand.toString();
                }
            ),
            ", "
        )
    );
}

ostream& operator<<(ostream& os, Instr const& instr) {
    os << instr.toString();
    return os;
}

InstrBlock::InstrBlock(
    string label,
    Addr addr,
    uint64_t count,
    vector<shared_ptr<Instr>> instructions
) : label(label), addr(addr), count(count), instructions(instructions)
{
}

string InstrBlock::toString(string name) const
{
    return fmt::format(
        "{}: label={}, addr={}, count={}, instructions={}",
        name,
        label,
        addr.toString(),
        count,
        boost::algorithm::join(
            instructions |
            boost::adaptors::transformed(
                [](shared_ptr<Instr> const& instr)
                {
                    return instr->toString();
                }
            ),
            ", "
        )
    );
}

string FusedBlock::toString() const
{
    return InstrBlock::toString("Fused Block");
}

CriticalSection::CriticalSection(
    uint64_t length, uint64_t count, const Instr* start)
    : length(length), count(count), start(start) {}

FileStats::FileStats(File const* file)
    : file(file)
{
    initialised = false;
    constructCriticalSections(branchInstructions);
}

string FileStats::toString() const
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

void FileStats::constructCriticalSections(unordered_set<string> delimiters)
{
    uint64_t length, count;
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
            delimiters.contains(instruction.instr) || diff >= 1 ||
            (!Instr::isContiguous(prevInstruction, instruction) && i > 0) ||
            (i == n-1) // end of the file
            )
        {
            // LOG_DEBUG(
            //     fmt::format(
            //         "start_instruction={}\nend_instruction={}\n",
            //         startInstr->toString(),
            //         instruction.toString()
            //     )
            // );
            // record new critical section
            criticalSections.push_back(
                make_unique<CriticalSection>(length, count, startInstr)
            );
            reset();
        }
    }

    uint64_t total = 0;
    for (auto& criticalSection : criticalSections) {
        total += criticalSection->length*criticalSection->count;
    }
    // LOG_INFO(fmt::format("Critical Section: before={}, after={}", calculateTotalInstructionNum(), total));
}

void FileStats::constructControlFlowGraph()
{
    // construct the basic blocks unordered map
    // go through instructions, and create basic blocks
    // map the basic blocks to their ID in the unordered map

    // a new basic block is created at instructions which satisfy at least one
    // of the following conditions:
    // 1. instruction following a branch instruction
    // 2. function entry point
    // 3. jump or branch target
    BasicBlock block;
    auto record_basic_block = [&]() {
        controlFlowGraph.basicBlocks[block.id] = block;
        // need to clear basic block
    };

    // go through the instructions, and find all branch targets. if a branch
    // target is in the middle of a basic block, then we split that basic block.
}

optional<Addr> Instr::findBranchTarget(BasicBlock const& block)
{
    assert(branchInstructions.contains(instr));

    if (instr == "beq"  ||
        instr == "bge"  ||
        instr == "bgeu" ||
        instr == "blt"  ||
        instr == "bltu" ||
        instr == "bne")
    {
        // absolute address at position 2
        assert(operands[2]);
        return Addr(operands[2]);
    }

    if (instr == "c.beqz" || instr == "c.bnez" || instr == "cjal") {
        // absolute address at position 1
        assert(operands[1]);
        return Addr(operands[1]);
    }

    if (instr == "beqz" ||
        instr == "bgez" ||
        instr == "bgtz" ||
        instr == "blez" ||
        instr == "bltz" ||
        instr == "bnez")
    {
        // absolute offset at position 1
        assert(operands[1]);
        return addr + Addr(operands[1]);
    }

    if (instr == "j") {
        // absolute offset at position 0
        assert(operands[0]);
        return addr + Addr(operands[0]);
    }

    if (instr == "cjalr" || instr == "jalr" || instr == "jr") {
        // combine with previous auipcc instruction to find target address
        auto const& auipcc = block.instructions.back();
        assert(auipcc->instr == "auipcc");
        assert(auipcc->operands[1]);
        auto base = Addr(auipcc->operands[1]);
        auto offset = Addr(operands[1].getOffset());
        return base + offset;
    }

    // no match was found
    LOG_ERROR(
        fmt::format(
            "could not find branch target from instruction {}",
            toString()
        )
    );
    return nullopt;
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

    instructions = CSVManager::loadInstructions(fullPath);
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
        if (currAddr.diff(prevAddr) != 2 && currAddr.diff(prevAddr) != 4) {
            LOG_ERROR(
                fmt::format(
                    "file {}: instruction {} is more than one instruction away "
                    "from previous instruction {}: {}-{}={}",
                    fileName,
                    instructions[i].instr,
                    instructions[i-1].instr,
                    currAddr.toString(),
                    prevAddr.toString(),
                    currAddr.diff(prevAddr)
                )
            );
            contiguous = false;
        }
    }
    return contiguous;
}

}
