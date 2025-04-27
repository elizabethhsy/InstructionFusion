#pragma once

#include <fileHandler.h>
#include <experiment.h>
#include <instructionCount.h>

namespace fusion
{

using namespace std;

struct BaselineResult
{
    shared_ptr<File> file = nullptr;

    uint64_t cyclesWithoutStalls = 0;
    uint64_t stalls = 0;
    uint64_t totalCycles = 0;

    BaselineResult() {}
    BaselineResult(shared_ptr<File> file) : file(file) {}
    BaselineResult(
        shared_ptr<File> file,
        uint64_t cyclesWithoutStalls,
        uint64_t stalls,
        uint64_t totalCycles
    )
        :   file(file),
            cyclesWithoutStalls(cyclesWithoutStalls),
            stalls(stalls),
            totalCycles(totalCycles)
    {}

    BaselineResult& operator+=(BaselineResult const& other) {
        this->cyclesWithoutStalls += other.cyclesWithoutStalls;
        this->stalls += other.stalls;
        this->totalCycles += other.totalCycles;
        return *this;
    }

    string toString() const;
};

struct PipelineResult : BaselineResult
{
    shared_ptr<ExperimentRun> run;
    shared_ptr<BaselineResult> baseline;

    PipelineResult(
        shared_ptr<File> file, 
        shared_ptr<ExperimentRun> run
    ) : BaselineResult(file), run(run) {
        baseline = make_shared<BaselineResult>(file);
    }

    string toString() const;
    void addBaseline(shared_ptr<BaselineResult> baseline);
    double cyclePercentage() const;
};

template<typename Pipeline>
struct PipelineExperiment
{
    PipelineExperiment(shared_ptr<ExperimentManager> manager)
        : manager(manager) {}
    
    template<typename... Args>
    ExperimentResults<PipelineResult> run(
        ExperimentResults<FusionResults> const& results,
        Args... args
    )
    {
        ExperimentResults<PipelineResult> experimentResults;

        // compute baseline results
        unordered_map<shared_ptr<File>, shared_ptr<BaselineResult>> baseline;
        for (auto const& file : manager->files) {
            baseline[file] = make_shared<BaselineResult>(
                pipeline.computeBaseline(file)
            );
        }

        for (auto const& runResult : results.runResults) {
            LOG_DEBUG(fmt::format("1: {}", runResult.run.title));
            // start of run
            vector<PipelineResult> pipelineResults;
            PipelineResult aggregateResults(
                manager->files[0],
                make_shared<ExperimentRun>(runResult.run)
            );
            
            for (auto const& result : runResult.results) {
                auto cycleResult = pipeline.computeCycleCount(result);
                assert(baseline[result.file]);
                cycleResult.addBaseline(baseline[result.file]);
                pipelineResults.push_back(std::move(cycleResult));
            }

            for (auto const& res : pipelineResults) {
                *(aggregateResults.baseline) += *(res.baseline);
                aggregateResults.cyclesWithoutStalls += res.cyclesWithoutStalls;
                aggregateResults.stalls += res.stalls;
                aggregateResults.totalCycles += res.totalCycles;
            }

            RunResult<PipelineResult> pipelineRunResult(
                runResult.run,
                std::move(pipelineResults),
                std::move(aggregateResults)
            );
            experimentResults.runResults.push_back(std::move(pipelineRunResult));
        }
        return experimentResults;
    }

    void save(ExperimentResults<PipelineResult> const& results) {
        auto resultsPath = manager->resultsPath;
        FileWriter cyclesWriter(resultsPath + "/cycles.csv");
        cyclesWriter.writeLine("rule_title,rule_description,user_defined_key,"
            "file,cycles_without_stalls,stalls,total_cycles,"
            "cycle_percentage");
        FileWriter aggregateWriter(resultsPath + "/aggregateCycles.csv");
        aggregateWriter.writeLine("rule_title,rule_description,user_defined_key,"
            "cycles_without_stalls,stalls,total_cycles,cycle_percentage");
        
        for (auto const& run : results.runResults) {
            for (auto const& res : run.results) {
                cyclesWriter.writeLine(
                    fmt::format(
                        "{},{},{},{},{},{},{},{}",
                        res.run->title,
                        res.run->description,
                        res.run->userDefinedKey,
                        res.file->fileName,
                        res.cyclesWithoutStalls,
                        res.stalls,
                        res.totalCycles,
                        res.cyclePercentage()
                    )
                );
            }

            aggregateWriter.writeLine(
                fmt::format(
                    "{},{},{},{},{},{},{}",
                    run.aggregateResults.run->title,
                    run.aggregateResults.run->description,
                    run.aggregateResults.run->userDefinedKey,
                    run.aggregateResults.cyclesWithoutStalls,
                    run.aggregateResults.stalls,
                    run.aggregateResults.totalCycles,
                    run.aggregateResults.cyclePercentage()
                )
            );
        }
    }
private:
    Pipeline pipeline;
    shared_ptr<ExperimentManager> manager;
};

} // fusion
