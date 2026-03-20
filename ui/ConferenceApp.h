#ifndef CONFERENCEAPP_H
#define CONFERENCEAPP_H

#include <vector>
#include <string>

#include "models/Submission.h"
#include "models/Reviewer.h"
#include "models/Parameters.h"
#include "data_structures/Graph.h"

/**
 * @class ConferenceApp
 * @brief Main application controller handling CLI interactions and batch mode.
 */
class ConferenceApp {
public:
    ConferenceApp();
    int run(int argc, char* argv[]);

private:
    std::vector<Submission> mSubmissions;
    std::vector<Reviewer>   mReviewers;
    Parameters              mParams;
    bool                    mDataLoaded;
    Graph<int>              mFlowGraph;

    // Helper Utilities
    void printSep(char c = '-', int n = 57) const;
    void waitEnter() const;
    int readInt(const std::string& prompt) const;

    // Core Logic
    void loadFile(const std::string& path);
    void executeAndReportRiskAnalysis(const std::string& outputPath);

    // Menus
    void runInteractiveMode();
    void printMainMenu() const;
    void menuDataManagement();
    void menuParameterConfiguration();
    void menuAlgorithms();
    void menuVisualization();

    // Data Management Actions
    void doBrowseInputs();
    void doLoadSpecificFile();
    void doAddSubmission();
    void doAddReviewer();
    void doViewData() const;

    // Algorithm Actions
    void doGenerateAssignments();

    // Visualization Actions
    void doViewGraphTerminal() const;
    void doExportGraphDOT() const;
    void doShowAugmentingPaths() const;

    // Batch Mode
    void runBatchMode(const std::string& inputFile, const std::string& riskOutputFile);
};

#endif // CONFERENCEAPP_H
