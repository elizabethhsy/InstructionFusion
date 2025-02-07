#pragma once

#include "histogramParser.h"

#include <experiment.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace fusion
{

using namespace std;

struct SimulationResults
{

};
    
struct SimulationExperiment
{
    SimulationExperiment(shared_ptr<ExperimentManager> manager)
        : manager(manager) {};

    SimulationResults run();
    static void save(SimulationResults const& results, string resultsPath);
private:
    shared_ptr<ExperimentManager> manager;
    unordered_map<shared_ptr<File>, vector<ControlFlowPath>> instructionStreams;
};

} // fusion
