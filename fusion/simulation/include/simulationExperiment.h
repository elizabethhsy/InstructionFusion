#pragma once

#include "histogramParser.h"

#include <experiment.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace fusion
{

using namespace std;
    
struct SimulationExperiment
{
    SimulationExperiment(shared_ptr<ExperimentManager> manager)
        : manager(manager) {};

    void run();
private:
    shared_ptr<ExperimentManager> manager;
    unordered_map<shared_ptr<File>, vector<ControlFlowPath>> instructionStreams;
};

} // fusion
