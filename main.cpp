#include <iostream>
#include <limits>
#include <string>
#include <vector>

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
    if (gParams.GenerateAssignments == 0) {
        std::cout << "GenerateAssignments = 0: nothing to generate.\n";
        return;
    }
    // TODO T2.1/T2.4: implement Max-Flow assignment


    int mode = gParams.GenerateAssignments;
    std::cout << "Building flow graph (mode=" << mode << ")...\n";
    Graph<int> g = createGraphs::buildReviewFlowGraph(gSubmissions, gReviewers, gParams, mode);

    edmondsKarp(&g, createGraphs::sourceId(), createGraphs::sinkId((int)gSubmissions.size(), (int)gReviewers.size()));

    std::cout << "[Not yet implemented] Generate assignments (mode="
              << gParams.GenerateAssignments << ").\n";
}

static void doRiskAnalysis() {
    if (!gDataLoaded) { std::cout << "No data loaded. Use option 1 first.\n"; return; }
    if (gParams.RiskAnalysis == 0) {
        std::cout << "RiskAnalysis = 0: no risk analysis requested.\n";
        return;
    }
    // TODO T2.2/T2.3: implement risk analysis
    std::cout << "[Not yet implemented] Risk analysis (K=" << gParams.RiskAnalysis << ").\n";
}

static void doViewGraph() {
    if (!gDataLoaded) { std::cout << "No data loaded. Use option 1 first.\n"; return; }
    int mode = gParams.GenerateAssignments;
    std::cout << "Building flow graph (mode=" << mode << ")...\n";
    Graph<int> g = createGraphs::buildReviewFlowGraph(gSubmissions, gReviewers, gParams, mode);
    createGraphs::printFlowGraph(g, gSubmissions, gReviewers);
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

    if (gParams.GenerateAssignments != 0) {
        std::cerr << "Generating assignments (mode=" << gParams.GenerateAssignments
                  << ") -> '" << gParams.OutputFileName << "'...\n";
        // TODO T2.1/T2.4
        std::cerr << "Warning: assignment generation not yet implemented.\n";
    }

    if (gParams.RiskAnalysis != 0) {
        const std::string& out = riskOutputFile.empty()
                                     ? gParams.OutputFileName : riskOutputFile;
        std::cerr << "Running risk analysis (K=" << gParams.RiskAnalysis
                  << ") -> '" << out << "'...\n";
        // TODO T2.2/T2.3
        std::cerr << "Warning: risk analysis not yet implemented.\n";
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
