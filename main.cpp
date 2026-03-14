#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <tuple>

#include "utils/FileParser.h"
#include "utils/Parameters.h"
#include "utils/Reviewer.h"
#include "utils/Submission.h"
#include "data_structures/createGraphs.h"
#include "utils/EdmondKarp.h" 

// --------------------------------------------------------------------------
// Application state
// --------------------------------------------------------------------------
static std::vector<Submission> gSubmissions;
static std::vector<Reviewer>   gReviewers;
static Parameters               gParams;
static bool                     gDataLoaded = false;
static Graph<int> gFlowGraph; 

// --------------------------------------------------------------------------
// Helper utilities
// --------------------------------------------------------------------------
static void printSep(char c = '-', int n = 57) {
    std::cout << std::string(n, c) << "\n";
}

static void waitEnter() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static int readInt(const std::string& prompt) {
    int v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return v;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Invalid input. Please enter a number.\n";
    }
}

static int totalRequiredReviews(const std::vector<Submission>& submissions,
                               const Parameters& params) {
    return (int) submissions.size() * params.MinReviewsPerSubmission;
}

static int totalAssignedReviews(const Graph<int>& flowGraph,
                               const std::vector<Submission>& submissions,
                               const std::vector<Reviewer>& reviewers) {
    int total = 0;
    const int numSubs = (int) submissions.size();
    const int numRevs = (int) reviewers.size();

    if (numRevs == 0) return 0;

    int firstRevNode = createGraphs::reviewerNodeId(0, numSubs);
    int lastRevNode = createGraphs::reviewerNodeId(numRevs - 1, numSubs);

    for (int i = 0; i < numSubs; ++i) {
        int subNode = createGraphs::submissionNodeId(i);
        Vertex<int>* subV = flowGraph.findVertex(subNode);
        if (subV == nullptr) continue;

        for (Edge<int>* e : subV->getAdj()) {
            int destNode = e->getDest()->getInfo();
            if (destNode >= firstRevNode && destNode <= lastRevNode && e->getFlow() > 0.0) {
                total += (int) e->getFlow();
            }
        }
    }
    return total;
}

static std::vector<int> findRiskyReviewersK1(const std::vector<Submission>& submissions,
                                              const std::vector<Reviewer>& reviewers,
                                              const Parameters& params,
                                              int mode) {
    std::vector<int> risky;
    const int needed = totalRequiredReviews(submissions, params);

    for (size_t skip = 0; skip < reviewers.size(); ++skip) {
        std::vector<Reviewer> reduced;
        reduced.reserve(reviewers.size() - 1);
        for (size_t i = 0; i < reviewers.size(); ++i) {
            if (i != skip) reduced.push_back(reviewers[i]);
        }

        Graph<int> g = createGraphs::buildReviewFlowGraph(submissions, reduced, params, mode);
        edmondsKarp(&g,
                    createGraphs::sourceId(),
                    createGraphs::sinkId((int) submissions.size(), (int) reduced.size()));

        int achieved = totalAssignedReviews(g, submissions, reduced);
        if (achieved < needed) risky.push_back(reviewers[skip].id);
    }

    std::sort(risky.begin(), risky.end());
    return risky;
}

static void writeRiskSection(std::ofstream& out, int riskValue, const std::vector<int>& riskyReviewerIds) {
    out << "#Risk Analysis: " << riskValue << "\n";
    for (size_t i = 0; i < riskyReviewerIds.size(); ++i) {
        if (i > 0) out << ", ";
        out << riskyReviewerIds[i];
    }
    out << "\n";
}

static bool writeAssignmentsToFile(const Graph<int>& flowGraph,
                                   const std::vector<Submission>& submissions,
                                   const std::vector<Reviewer>& reviewers,
                                   const Parameters& params,
                                   int mode,
                                   const std::vector<int>* riskyReviewerIds = nullptr,
                                   int riskValue = 0,
                                   const std::string& outputPath = "") {
    const std::string path = outputPath.empty() ? params.OutputFileName : outputPath;
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "Error: cannot open output file '" << path << "'.\n";
        return false;
    }

    const int numSubs = (int) submissions.size();
    const int numRevs = (int) reviewers.size();

    std::vector<std::tuple<int, int, int>> bySubmission;
    std::vector<std::tuple<int, int, int>> byReviewer;

    for (int i = 0; i < numSubs; ++i) {
        int subNode = createGraphs::submissionNodeId(i);
        Vertex<int>* subV = flowGraph.findVertex(subNode);
        if (subV == nullptr) continue;

        for (Edge<int>* e : subV->getAdj()) {
            if (e->getFlow() <= 0.0) continue;

            if (numRevs == 0) continue;

            int destNode = e->getDest()->getInfo();
            if (destNode < createGraphs::reviewerNodeId(0, numSubs) ||
                destNode > createGraphs::reviewerNodeId(numRevs - 1, numSubs)) {
                continue;
            }

            int revIndex = destNode - createGraphs::reviewerNodeId(0, numSubs);
            int matchedDomain = createGraphs::getMatchedDomain(submissions[i], reviewers[revIndex], mode);
            if (matchedDomain == -1) matchedDomain = submissions[i].primaryTopic;

            bySubmission.emplace_back(submissions[i].id, reviewers[revIndex].id, matchedDomain);
            byReviewer.emplace_back(reviewers[revIndex].id, submissions[i].id, matchedDomain);
        }
    }

    std::sort(bySubmission.begin(), bySubmission.end());
    std::sort(byReviewer.begin(), byReviewer.end());

    int totalRequired = totalRequiredReviews(submissions, params);
    int totalAssigned = (int) bySubmission.size();

    if (!bySubmission.empty()) {
        out << "#SubmissionId,ReviewerId,Match\n";
        for (const auto& row : bySubmission) {
            out << std::get<0>(row) << ", " << std::get<1>(row) << ", " << std::get<2>(row) << "\n";
        }

        out << "#ReviewerId,SubmissionId,Match\n";
        for (const auto& row : byReviewer) {
            out << std::get<0>(row) << ", " << std::get<1>(row) << ", " << std::get<2>(row) << "\n";
        }

        out << "#Total: " << totalAssigned << "\n";
    }

    if (totalAssigned < totalRequired) {
        out << "#SubmissionId,Domain,MissingReviews\n";
        for (int i = 0; i < numSubs; ++i) {
            int subNode = createGraphs::submissionNodeId(i);
            Vertex<int>* subV = flowGraph.findVertex(subNode);
            int assignedForSubmission = 0;
            if (subV != nullptr) {
                for (Edge<int>* e : subV->getAdj()) {
                    if (e->getFlow() > 0.0) assignedForSubmission += (int) e->getFlow();
                }
            }

            int missing = params.MinReviewsPerSubmission - assignedForSubmission;
            if (missing > 0) {
                out << submissions[i].id << ", "
                    << submissions[i].primaryTopic << ", "
                    << missing << "\n";
            }
        }
    }

    if (riskyReviewerIds != nullptr && riskValue > 0) {
        writeRiskSection(out, riskValue, *riskyReviewerIds);
    }

    return true;
}

static bool generateAssignmentsAndStore(Graph<int>& outGraph,
                                        const std::vector<Submission>& submissions,
                                        const std::vector<Reviewer>& reviewers,
                                        const Parameters& params,
                                        int mode,
                                        bool writeOutput) {
    outGraph = createGraphs::buildReviewFlowGraph(submissions, reviewers, params, mode);
    edmondsKarp(&outGraph,
                createGraphs::sourceId(),
                createGraphs::sinkId((int) submissions.size(), (int) reviewers.size()));

    if (writeOutput) {
        return writeAssignmentsToFile(outGraph, submissions, reviewers, params, mode);
    }
    return true;
}

// --------------------------------------------------------------------------
// Menu actions
// --------------------------------------------------------------------------
static void doLoadData() {
    std::cout << "Enter the path to the input CSV file: ";
    std::string path;
    std::getline(std::cin, path);
    // Strip surrounding whitespace and quotes.
    while (!path.empty() && (path.front() == ' ' || path.front() == '"'))
        path.erase(path.begin());
    while (!path.empty() && (path.back() == ' ' || path.back() == '"'))
        path.pop_back();

    std::vector<Submission> subs;
    std::vector<Reviewer>   revs;
    Parameters               p;

    if (FileParser::parse(path, subs, revs, p)) {
        gSubmissions = std::move(subs);
        gReviewers   = std::move(revs);
        gParams      = p;
        gDataLoaded  = true;
        std::cout << "Data loaded: " << gSubmissions.size() << " submissions, "
                  << gReviewers.size() << " reviewers.\n";
    } else {
        std::cout << "Failed to load data. Check error messages above.\n";
    }
}

static void doListSubmissions() {
    if (!gDataLoaded) { std::cout << "No data loaded. Use option 1 first.\n"; return; }
    std::cout << "Submissions (" << gSubmissions.size() << "):\n";
    printSep();
    for (const auto& s : gSubmissions) s.print();
}

static void doListReviewers() {
    if (!gDataLoaded) { std::cout << "No data loaded. Use option 1 first.\n"; return; }
    std::cout << "Reviewers (" << gReviewers.size() << "):\n";
    printSep();
    for (const auto& r : gReviewers) r.print();
}

static void doShowParameters() {
    if (!gDataLoaded) { std::cout << "No data loaded. Use option 1 first.\n"; return; }
    std::cout << "Parameters:\n";
    printSep();
    std::cout << "  MinReviewsPerSubmission    = " << gParams.MinReviewsPerSubmission    << "\n"
              << "  MaxReviewsPerReviewer      = " << gParams.MaxReviewsPerReviewer      << "\n"
              << "  PrimaryReviewerExpertise   = " << gParams.PrimaryReviewerExpertise   << "\n"
              << "  SecondaryReviewerExpertise = " << gParams.SecondaryReviewerExpertise << "\n"
              << "  PrimarySubmissionDomain    = " << gParams.PrimarySubmissionDomain    << "\n"
              << "  SecondarySubmissionDomain  = " << gParams.SecondarySubmissionDomain  << "\n"
              << "  GenerateAssignments        = " << gParams.GenerateAssignments        << "\n"
              << "  RiskAnalysis               = " << gParams.RiskAnalysis               << "\n"
              << "  OutputFileName             = " << gParams.OutputFileName             << "\n";
}

static void doGenerateAssignments() {
    if (!gDataLoaded) { std::cout << "No data loaded. Use option 1 first.\n"; return; }
    int mode = gParams.GenerateAssignments;
    std::cout << "Building flow graph (mode=" << mode << ")...\n";

    bool writeOutput = (gParams.GenerateAssignments != 0);
    if (!generateAssignmentsAndStore(gFlowGraph, gSubmissions, gReviewers, gParams, mode, writeOutput)) {
        std::cout << "Failed to write output file '" << gParams.OutputFileName << "'.\n";
        return;
    }

    if (writeOutput) {
        std::cout << "Assignment output written to '" << gParams.OutputFileName << "'.\n";
    } else {
        std::cout << "GenerateAssignments = 0: assignment computed and kept in memory (no output file written).\n";
    }
}

static void doRiskAnalysis() {
    if (!gDataLoaded) { std::cout << "No data loaded. Use option 1 first.\n"; return; }
    if (gParams.RiskAnalysis == 0) {
        std::cout << "RiskAnalysis = 0: no risk analysis requested.\n";
        return;
    }
    if (gParams.RiskAnalysis != 1) {
        std::cout << "RiskAnalysis = " << gParams.RiskAnalysis << " is not implemented yet (only K=1).\n";
        return;
    }

    int mode = gParams.GenerateAssignments;

    if (!generateAssignmentsAndStore(gFlowGraph, gSubmissions, gReviewers, gParams, mode, false)) {
        std::cout << "Failed to compute assignment baseline for risk analysis.\n";
        return;
    }

    std::vector<int> risky = findRiskyReviewersK1(gSubmissions, gReviewers, gParams, mode);

    if (!writeAssignmentsToFile(gFlowGraph,
                                gSubmissions,
                                gReviewers,
                                gParams,
                                mode,
                                &risky,
                                1,
                                gParams.OutputFileName)) {
        std::cout << "Failed to write risk analysis to '" << gParams.OutputFileName << "'.\n";
        return;
    }

    std::cout << "Risk analysis (K=1) written to '" << gParams.OutputFileName << "'.\n";
}

static void doViewGraph() {
    if (!gDataLoaded) { std::cout << "No data loaded. Use option 1 first.\n"; return; }
    int mode = gParams.GenerateAssignments;
    std::cout << "Building/Viewing flow graph (mode=" << mode << ")...\n";

    createGraphs::printFlowGraph(gFlowGraph, gSubmissions, gReviewers);
}

// --------------------------------------------------------------------------
// Interactive menu
// --------------------------------------------------------------------------
static void printMenu() {
    printSep('=', 57);
    std::cout << "   Scientific Conference Organization Tool\n"
              << "   DA Programming Project I  -  Spring 2026\n";
    printSep('=', 57);
    if (gDataLoaded)
        std::cout << "   Loaded : " << gSubmissions.size() << " submissions, "
                  << gReviewers.size() << " reviewers\n"
                  << "   Output : " << gParams.OutputFileName << "\n";
    else
        std::cout << "   No data loaded.\n";
    printSep('-', 57);
    std::cout << "   [1] Load data from file\n"
              << "   [2] List submissions\n"
              << "   [3] List reviewers\n"
              << "   [4] Show parameters\n"
              << "   [5] Generate review assignments\n"
              << "   [6] Run risk analysis\n"
              << "   [7] View flow graph\n"
              << "   [0] Exit\n";
    printSep('=', 57);
}

static void runInteractiveMode() {
    while (true) {
        printMenu();
        int choice = readInt("Option: ");
        std::cout << "\n";
        switch (choice) {
            case 1: doLoadData();            break;
            case 2: doListSubmissions();     break;
            case 3: doListReviewers();       break;
            case 4: doShowParameters();      break;
            case 5: doGenerateAssignments(); break;
            case 6: doRiskAnalysis();        break;
            case 7: doViewGraph();           break;
            case 0: std::cout << "Goodbye!\n"; return;
            default: std::cout << "Unknown option. Try again.\n"; break;
        }
        waitEnter();
    }
}

// --------------------------------------------------------------------------
// Batch mode:  myProg -b input.csv [risk_output.csv]
// --------------------------------------------------------------------------
static void runBatchMode(const std::string& inputFile,
                         const std::string& riskOutputFile) {
    if (!FileParser::parse(inputFile, gSubmissions, gReviewers, gParams)) {
        std::cerr << "Error: failed to parse '" << inputFile << "'.\n";
        return;
    }
    gDataLoaded = true;
    std::cerr << "Loaded " << gSubmissions.size() << " submissions and "
              << gReviewers.size() << " reviewers.\n";

    {
        int mode = gParams.GenerateAssignments;
        bool writeOutput = (gParams.GenerateAssignments != 0);

        std::cerr << "Generating assignments (mode=" << mode << ")";
        if (writeOutput) std::cerr << " -> '" << gParams.OutputFileName << "'";
        std::cerr << "...\n";

        if (!generateAssignmentsAndStore(gFlowGraph, gSubmissions, gReviewers, gParams, mode, writeOutput)) {
            std::cerr << "Error: failed to generate/write assignments.\n";
            return;
        }
    }

    if (gParams.RiskAnalysis != 0) {
        const std::string out = riskOutputFile.empty() ? gParams.OutputFileName : riskOutputFile;
        std::cerr << "Running risk analysis (K=" << gParams.RiskAnalysis
                  << ") -> '" << out << "'...\n";

        if (gParams.RiskAnalysis != 1) {
            std::cerr << "Warning: only RiskAnalysis=1 is implemented.\n";
            return;
        }

        std::vector<int> risky = findRiskyReviewersK1(gSubmissions,
                                                      gReviewers,
                                                      gParams,
                                                      gParams.GenerateAssignments);

        if (!writeAssignmentsToFile(gFlowGraph,
                                    gSubmissions,
                                    gReviewers,
                                    gParams,
                                    gParams.GenerateAssignments,
                                    &risky,
                                    1,
                                    out)) {
            std::cerr << "Error: failed to write risk analysis output.\n";
            return;
        }
    }
}

// --------------------------------------------------------------------------
// Entry point
// --------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc >= 2 && std::string(argv[1]) == "-b") {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0]
                      << " -b <input.csv> [<risk_output.csv>]\n";
            return 1;
        }
        std::string inputFile      = argv[2];
        std::string riskOutputFile = (argc >= 4) ? argv[3] : "";
        runBatchMode(inputFile, riskOutputFile);
        return 0;
    }

    if (argc > 1) {
        std::cerr << "Usage: " << argv[0]
                  << " [-b <input.csv> [<risk_output.csv>]]\n";
        return 1;
    }

    runInteractiveMode();
    return 0;
}
