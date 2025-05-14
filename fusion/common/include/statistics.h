#pragma once

#include <chrono>
#include <fmt/format.h>
#include <stdint.h>
#include <string>

namespace fusion
{

using namespace std;

struct RuntimeStats
{
    string title;
    chrono::duration<double> seconds;
    uint64_t numInstructions;
    double instructionsPerSecond;

    RuntimeStats(
        string title,
        chrono::duration<double> seconds,
        uint64_t numInstructions
    )
        : title(title), seconds(seconds), numInstructions(numInstructions)
    {
        instructionsPerSecond =
            static_cast<double>(numInstructions) / seconds.count();
        // LOG_INFO(fmt::format("instructions_per_second={}", instructionsPerSecond));
    }

    string toString() const;
};

} // fusion
