#pragma once

#include <fusion_common_export.h>

#include "fusion.h"

#include <boost/range/adaptor/transformed.hpp>

namespace fusion
{

using namespace std;

struct FUSION_COMMON_EXPORT ExperimentRun
{
    string title;
    string description;
    string userDefinedKey; // users can use this to identify rules
    unordered_set<FusionRulePtr> rules;
};

template<typename Result>
struct RunResult
{
    ExperimentRun const run;
    vector<Result> results;
    Result aggregateResults;

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
        vector<string> filepaths,
        vector<ExperimentRun> const& runs
    );

    template<class Experiment, typename Result, typename... Args>
    ExperimentResults<Result> run(Args... args) {
        Experiment experiment(this->shared_from_this());
        auto results = experiment.run(std::forward<Args>(args)...);
        return results;
    }

    template<class Experiment, typename Result>
    void save(ExperimentResults<Result> const& results, string resultsPath)
    {
        Experiment::save(results, resultsPath);
    }

    vector<shared_ptr<File>> files;
    vector<ExperimentRun> const& runs;
private:
    float avgCriticalSectionSize();
    uint64_t totalInstructionNum();
};

} // fusion
