#pragma once

#include <fusion_common_export.h>

#include "fusion.h"
#include "statistics.h"

#include <boost/range/adaptor/transformed.hpp>
#include <boost/type_index.hpp>

namespace fusion
{

using namespace std;

struct FUSION_COMMON_EXPORT ExperimentRun
{
    string title = "";
    string description = "";
    string userDefinedKey = ""; // users can use this to identify rules
    unordered_set<FusionRulePtr> rules = {};
};

template<typename Result>
struct RunResult
{
    ExperimentRun const run;
    vector<Result> results;
    Result aggregateResults;

    RunResult<Result>(
        ExperimentRun const run,
        vector<Result> results,
        Result aggregateResults
    ) : run(run), results(results), aggregateResults(aggregateResults)
    {}

    string toString() const {
        return fmt::format(
            "run_result: results=[{}], aggregate_results={}",
            boost::algorithm::join(
                results |
                boost::adaptors::transformed(
                    [](Result const& res)
                    {
                        return res.toString();
                    }
                ),
                ", "
            ),
            aggregateResults.toString()
        );
    }
};

template<typename Result>
struct ExperimentResults
{
    vector<RunResult<Result>> runResults;
    
    string toString() const {
        return fmt::format(
            "Experiment results: run_results=[\n\t{}\n]",
            boost::algorithm::join(
                runResults |
                boost::adaptors::transformed(
                    [](RunResult<Result> const& runRes)
                    {
                        return runRes.toString();
                    }
                ),
                "\n\t"
            )
        );
    }
};

// an ExperimentManager is able to take the input and run any experiment that
// the user passes into it. It abstracts away the data input stage from the
// processing stage.
struct FUSION_COMMON_EXPORT ExperimentManager :
    public enable_shared_from_this<ExperimentManager>
{
    ExperimentManager(
        string title,
        vector<string> filepaths,
        vector<ExperimentRun> const& runs,
        string resultsPath
    );

    template<class Experiment, typename Result, typename... Args>
    ExperimentResults<Result> run(Args... args) {
        auto t0 = std::chrono::high_resolution_clock::now();
        Experiment experiment(this->shared_from_this());
        auto results = experiment.run(std::forward<Args>(args)...);
        auto t1 = std::chrono::high_resolution_clock::now();
        statistics.push_back(
            RuntimeStats(
                boost::typeindex::type_id<Experiment>().pretty_name(),
                t1-t0,
                totalInstructionNum()
            )
        );
        totalDynamicInstructions += totalInstructionNum();
        return results;
    }

    template<class Experiment, typename Result>
    void save(ExperimentResults<Result> const& results)
    {
        this->saveConfig();
        Experiment experiment(this->shared_from_this());
        experiment.save(results);
    }

    void saveConfig();
    void saveStatistics();

    ~ExperimentManager()
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        statistics.push_back(
            RuntimeStats(title, endTime-startTime, totalDynamicInstructions)
        );
        this->saveStatistics();
    }

    string title;
    vector<shared_ptr<File>> files;
    vector<ExperimentRun> const& runs;
    string resultsPath;
    vector<RuntimeStats> statistics;
    
    std::chrono::high_resolution_clock::time_point startTime =
        std::chrono::high_resolution_clock::now();
    uint64_t totalDynamicInstructions = 0;
private:
    float avgCriticalSectionSize();
    uint64_t totalInstructionNum();
};

} // fusion
