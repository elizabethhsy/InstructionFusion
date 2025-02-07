#include "simulationExperiment.h"

#include <macros.h>

namespace fusion
{

SimulationResults SimulationExperiment::run()
{

    SimulationResults results;
    // parse the histogram
    for (auto const& file : manager->files) {
        auto graph = ControlFlowGraph(*file);

        // auto paths = graph.computePaths();
        // LOG_INFO(fmt::format("number of paths found: {}", paths.size()));

        // for (auto const& path : paths) {
        //     LOG_INFO(path.toString());
        // }
    }

    return results;
}

} // fusion
