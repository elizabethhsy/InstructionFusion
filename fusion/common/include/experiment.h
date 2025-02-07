#pragma once

#include <fusion_common_export.h>

#include "fusion.h"

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

    template<class Experiment, typename Result>
    Result run() {
        Experiment experiment(this->shared_from_this());
        auto results = experiment.run();
        return results;
    }

    template<class Experiment, typename Result>
    void save(Result const& results, string resultsPath) {
        Experiment::save(results, resultsPath);
    }

    vector<shared_ptr<File>> files;
    vector<ExperimentRun> const& runs;
private:
    float avgCriticalSectionSize();
    uint64_t totalInstructionNum();
};

} // fusion
