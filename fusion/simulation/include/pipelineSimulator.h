#pragma once

#include <experiment.h>
#include <instructionCount.h>

namespace fusion
{

using namespace std;

struct PipelineResult
{
    shared_ptr<File> file;
    shared_ptr<ExperimentRun> run;

    uint64_t totalInstructions;

    uint64_t cyclesWithoutStalls = 0;
    uint64_t stalls = 0;
    uint64_t totalCycles = 0;

    string toString() const;
private:
    double cyclePercentage() const;
};

struct PipelineRunResult
{
    vector<PipelineResult> pipelineResults;
};

template<typename Pipeline>
struct PipelineExperiment
{
    PipelineExperiment(shared_ptr<ExperimentManager> manager)
        : manager(manager) {};
    
    // ExperimentResults<PipelineRunResult> run();
    template<typename... Args>
    ExperimentResults<PipelineResult> run(
        ExperimentResults<FusionResults> const& results,
        Args... args
    )
    {
        ExperimentResults<PipelineResult> pipelineResults;
        for (auto const& runResult : results.runResults) {
            RunResult<PipelineResult> pipelineRunResults;
            for (auto const& result : runResult.results) {
                auto cycleResult = pipeline.computeCycleCount(result);
                cycleResult.totalInstructions = result.totalInstructions;
                pipelineRunResults.results.push_back(std::move(cycleResult));
            }
            pipelineResults.runResults.push_back(std::move(pipelineRunResults));
        }
        return pipelineResults;
    }

    void save(ExperimentResults<PipelineResult> const& results) {}
private:
    Pipeline pipeline;
    shared_ptr<ExperimentManager> manager;
};




} // fusion
