#include "macros.h"
#include "statistics.h"

#include <chrono>
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <utility>

namespace fusion
{

string RuntimeStats::toString() const
{
    return fmt::format(
        fmt::runtime(
            "RuntimeStats: title={}, seconds={:.6f}, num_instructions={}, "
            "instructions_per_second={:.3f}"
        ),
        title,
        seconds.count(),
        numInstructions,
        instructionsPerSecond
    );
}

} // fusion
