#pragma once

#include <fusion_common_export.h>

#include "macros.h"

#include <boost/algorithm/string/join.hpp>
#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <optional>
#include <regex>
#include <set>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fusion
{

using namespace std;
// using Operand = string;
// using Addr = uint32_t;

struct Operand
{
    string op;

    uint32_t getOffset();
    string toString() const;
    bool isImmediate() const;

    bool operator==(Operand const& other) const {
        return op == other.op;
    }

    operator bool() const {
        return true; // is valid
    }
};

struct Addr
{
    uint32_t addr;

    string toString() const;

    uint diff(Addr const& other) {
        return addr - other.addr;
    }
    
    static bool isHexAddress(string const& addr) {
        regex hexPattern(R"(^0[xX][0-9a-fA-F]+$)");
        return regex_match(addr, hexPattern);
    }

    Addr(uint32_t addr) : addr(addr) {}
    Addr(string const& str) {
        if (isHexAddress(str)) {
            addr = stoul(str, nullptr, 16);
        } else {
            assert(stoul(str));
            addr = stoul(str);
        }
    }
    Addr(Operand operand) {
        Addr(operand.op);
    }

    Addr operator+(Addr const& other) const {
        return Addr(addr + other.addr);
    }
    Addr operator-(Addr const& other) const {
        return Addr(addr - other.addr);
    }
    bool operator<(Addr const& other) const {
        return addr < other.addr;
    }
    bool operator==(Addr const& other) const {
        return addr == other.addr;
    }
};

struct BasicBlock;
struct Instr
{
    Addr addr;
    uint64_t count;
    string instr;
    vector<Operand> operands;
    string label;

    Instr(
        Addr addr,
        uint64_t count,
        string instr,
        vector<Operand> operands,
        string label = ""
    ) : addr(addr), count(count), instr(instr), operands(operands), label(label)
    {
    }

    string toString() const;

    static bool dependentOperands(
        Instr const& instruction,
        unordered_set<Operand> const& dependentOperands
    );

    optional<Addr> findBranchTarget(BasicBlock const& block);

    static Instr parseInstruction(string const& str);
    static bool isContiguous(Instr const& prevInstr, Instr const& nextInstr)
    {
        return (nextInstr.addr - prevInstr.addr == Addr(2) ||
            nextInstr.addr - prevInstr.addr == Addr(4));
    }

    bool operator<(Instr const& other) const {
        return addr < other.addr;
    }

    bool operator==(Instr const& other) const {
        bool operands_equal = operands.size() == other.operands.size();
        for (int i = 0; i < min(operands.size(), other.operands.size()); i++) {
            operands_equal &= (operands[i] == other.operands[i]);
        }
        return addr == other.addr && count == other.count &&
            label == other.label && instr == other.instr && operands_equal;
    }
};
ostream& operator<<(ostream& os, Instr const& instr);

struct InstrBlock
{
    string label;
    Addr addr; // starting address
    uint64_t count;
    vector<shared_ptr<Instr>> instructions;

    InstrBlock() : label(""), addr(Addr(0)), count(0), instructions({}) {}
    InstrBlock(
        string label,
        Addr addr,
        uint64_t count,
        vector<shared_ptr<Instr>> instructions
    );

    string toString(string name) const;
};

struct FusedBlock : InstrBlock
{
    FusedBlock(
        string label,
        Addr addr,
        uint64_t count,
        vector<shared_ptr<Instr>> instructions
    ) : InstrBlock(label, addr, count, std::move(instructions))
    {}

    string toString() const;
};

struct CriticalSection
{
    uint64_t length;
    uint64_t count;
    const Instr* start;

    CriticalSection(uint64_t length, uint64_t count, const Instr* start);
};

struct BasicBlock : InstrBlock
{
    static uint counter;
    uint id;
    BasicBlock() : InstrBlock(), id(counter++) {}
    string toString() const;
    static bool sameBasicBlock(Instr const& prev, Instr const& next);
};

struct ControlFlow
{
    unordered_map<uint, BasicBlock> basicBlocks;
    // id of basic block --> id of linked basic block
    unordered_map<uint, set<uint>> adjList;
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

    ControlFlow controlFlowGraph;

    string toString() const;
    void calculateStats();
private:
    bool initialised;

    void constructCriticalSections(unordered_set<string> delimiters);
    void constructControlFlowGraph();
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

} // fusion

template <> struct fmt::formatter<fusion::Addr>: formatter<string_view> {
    auto format(fusion::Addr const& addr, format_context& ctx) const
      -> format_context::iterator {
        return formatter<string_view>::format(addr.toString(), ctx);
    }
};

template <> struct fmt::formatter<fusion::Operand>: formatter<string_view> {
    auto format(fusion::Operand const& operand, format_context& ctx) const
      -> format_context::iterator {
        return formatter<string_view>::format(operand.toString(), ctx);
    }
};


namespace std {
    template <>
    struct hash<fusion::Addr> {
        size_t operator()(fusion::Addr const& addr) const {
            return std::hash<std::uint32_t>{}(addr.addr);
        }
    };

    template <>
    struct hash<fusion::Operand> {
        size_t operator()(fusion::Operand const& operand) const {
            return std::hash<std::string>{}(operand.op);
        }
    };
}
