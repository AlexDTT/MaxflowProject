#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <fstream>
#include <algorithm>
#include <tuple>
#include <filesystem>
#include <functional>

#include "utils/FileParser.h"
#include "utils/Parameters.h"
#include "utils/Submission.h"
#include "utils/Reviewer.h"
#include "data_structures/createGraphs.h"
#include "utils/EdmondKarp.h" 
#include "utils/AssignmentLogic.h"

namespace fs = std::filesystem;

// --------------------------------------------------------------------------
// Application state
// --------------------------------------------------------------------------
static std::vector<Submission> gSubmissions;
static std::vector<Reviewer>   gReviewers;
static Parameters              gParams;
static bool                    gDataLoaded = false;
static Graph<int>              gFlowGraph; 

// --------------------------------------------------------------------------
// Helper utilities
// --------------------------------------------------------------------------
static void printSep(char c = '-', int n = 57) {
    std::cout << "   " << std::string(n, c) << "\n";
}

static void waitEnter() {
    std::cout << "\nPress Enter to continue...";
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static int readInt(const std::string& prompt) {
    int val;
    while (true) {
        std::cout << "   " << prompt;
        if (std::cin >> val) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return val;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Invalid input. Please enter a number.\n";
    }
}

static void executeAndReportRiskAnalysis(const std::string& outputPath) {
    if (gParams.RiskAnalysis == 0) {
        std::cout << "RiskAnalysis = 0: no risk analysis requested.\n";
        return;
    }
    if (gParams.RiskAnalysis != 1) {
        std::cout << "RiskAnalysis = " << gParams.RiskAnalysis << " is not implemented yet (only K=1).\n";
        return;
    }

    int mode = gParams.GenerateAssignments;
    if (!AssignmentLogic::generateAssignmentsAndStore(gFlowGraph, gSubmissions, gReviewers, gParams, mode, false)) {
        std::cout << "Failed to compute assignment baseline for risk analysis.\n";
        return;
    }

    std::vector<int> risky = AssignmentLogic::findRiskyReviewersK1(gSubmissions, gReviewers, gParams, mode);

    if (!AssignmentLogic::writeAssignmentsToFile(gFlowGraph, gSubmissions, gReviewers, gParams, mode, &risky, 1, outputPath)) {
        std::cout << "Failed to write risk analysis to '" << outputPath << "'.\n";
        return;
    }
    std::cout << "Risk analysis (K=1) written to '" << outputPath << "'.\n";
}

static void loadFile(const std::string& path) {
    std::vector<Submission> subs;
    std::vector<Reviewer>   revs;
    Parameters               p;

    if (FileParser::parse(path, subs, revs, p)) {
        gSubmissions = std::move(subs);
        gReviewers   = std::move(revs);
        gParams      = p;
        gDataLoaded  = true;
        std::cout << "Data loaded: " << gSubmissions.size() << " submissions, " << gReviewers.size() << " reviewers.\n";
    } else {
        std::cout << "Failed to load data. Check error messages above.\n";
    }
}

// --------------------------------------------------------------------------
// Sub-menu actions
// --------------------------------------------------------------------------

// 1. Data Management
static void doLoadSpecificFile() {
    std::string path;
    std::cout << "Enter the path to the input CSV file: ";
    std::getline(std::cin, path);
    while (!path.empty() && (path.front() == ' ' || path.front() == '"')) path.erase(path.begin());
    while (!path.empty() && (path.back() == ' ' || path.back() == '"')) path.pop_back();
    loadFile(path);
}

static void doBrowseInputs() {
    std::string dirPath = "tests/input";
    if (fs::exists("inputs") && fs::is_directory("inputs")) dirPath = "inputs";

    std::vector<std::string> files;
    if (fs::exists(dirPath)) {
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (entry.path().extension() == ".csv") {
                files.push_back(entry.path().string());
            }
        }
    }
    
    if (files.empty()) {
        std::cout << "No CSV files found in " << dirPath << ".\n";
        return;
    }

    std::sort(files.begin(), files.end());
    std::cout << "Available files in '" << dirPath << "':\n";
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "  [" << i + 1 << "] " << files[i] << "\n";
    }
    std::cout << "  [0] Cancel\n";

    int choice = readInt("Select a file: ");
    if (choice > 0 && choice <= (int)files.size()) {
        loadFile(files[choice - 1]);
    }
}

static void doAddSubmission() {
    std::cout << "--- Add New Submission ---\n";
    int id = readInt("Submission ID: ");
    
    std::string title;
    std::cout << "Title: ";
    std::getline(std::cin, title);

    std::string authors;
    std::cout << "Author(s): ";
    std::getline(std::cin, authors);

    std::string email;
    std::cout << "Email: ";
    std::getline(std::cin, email);

    int primary = readInt("Primary Topic ID: ");
    int secondary = readInt("Secondary Topic ID (-1 for none): ");

    gSubmissions.emplace_back(id, title, authors, email, primary, secondary);
    gDataLoaded = true;
    std::cout << "Submission added successfully.\n";
}

static void doAddReviewer() {
    std::cout << "--- Add New Reviewer ---\n";
    int id = readInt("Reviewer ID: ");
    
    std::string name;
    std::cout << "Name: ";
    std::getline(std::cin, name);

    std::string email;
    std::cout << "Email: ";
    std::getline(std::cin, email);

    int primary = readInt("Primary Topic ID: ");
    int secondary = readInt("Secondary Topic ID (-1 for none): ");

    gReviewers.emplace_back(id, name, email, primary, secondary);
    gDataLoaded = true;
    std::cout << "Reviewer added successfully.\n";
}

static void doViewData() {
    if (!gDataLoaded) { std::cout << "No data loaded.\n"; return; }
    std::cout << "Submissions (" << gSubmissions.size() << "):\n";
    for (const auto& s : gSubmissions) s.print();
    printSep();
    std::cout << "Reviewers (" << gReviewers.size() << "):\n";
    for (const auto& r : gReviewers) r.print();
}

static void menuDataManagement() {
    while (true) {
        printSep('=', 57);
        std::cout << "   [ Data Management ]\n";
        printSep('-', 57);
        std::cout << "   [1] Browse inputs/ folder for CSVs\n"
                  << "   [2] Enter specific CSV path\n"
                  << "   [3] Manually Add Submission\n"
                  << "   [4] Manually Add Reviewer\n"
                  << "   [5] View Loaded Data\n"
                  << "   [0] Back to Main Menu\n";
        printSep('=', 57);
        int choice = readInt("Option: ");
        switch (choice) {
            case 1: doBrowseInputs(); break;
            case 2: doLoadSpecificFile(); break;
            case 3: doAddSubmission(); break;
            case 4: doAddReviewer(); break;
            case 5: doViewData(); waitEnter(); break;
            case 0: return;
            default: std::cout << "Unknown option.\n"; break;
        }
    }
}

// 2. Parameter Configuration
static void menuParameterConfiguration() {
    while (true) {
        printSep('=', 57);
        std::cout << "   [ Parameter Configuration ]\n";
        printSep('-', 57);
        std::cout << "   [1] MinReviewsPerSubmission    : " << gParams.MinReviewsPerSubmission << "\n"
                  << "   [2] MaxReviewsPerReviewer      : " << gParams.MaxReviewsPerReviewer << "\n"
                  << "   [3] PrimaryReviewerExpertise   : " << gParams.PrimaryReviewerExpertise << "\n"
                  << "   [4] SecondaryReviewerExpertise : " << gParams.SecondaryReviewerExpertise << "\n"
                  << "   [5] PrimarySubmissionDomain    : " << gParams.PrimarySubmissionDomain << "\n"
                  << "   [6] SecondarySubmissionDomain  : " << gParams.SecondarySubmissionDomain << "\n"
                  << "   [7] GenerateAssignments (mode) : " << gParams.GenerateAssignments << "\n"
                  << "   [8] RiskAnalysis (K-value)     : " << gParams.RiskAnalysis << "\n"
                  << "   [9] OutputFileName             : " << gParams.OutputFileName << "\n"
                  << "   [0] Back to Main Menu\n";
        printSep('=', 57);
        int choice = readInt("Option (select number to edit): ");
        if (choice == 0) return;
        
        switch(choice) {
            case 1: gParams.MinReviewsPerSubmission = readInt("New value: "); break;
            case 2: gParams.MaxReviewsPerReviewer = readInt("New value: "); break;
            case 3: gParams.PrimaryReviewerExpertise = readInt("New value: "); break;
            case 4: gParams.SecondaryReviewerExpertise = readInt("New value: "); break;
            case 5: gParams.PrimarySubmissionDomain = readInt("New value: "); break;
            case 6: gParams.SecondarySubmissionDomain = readInt("New value: "); break;
            case 7: gParams.GenerateAssignments = readInt("New value (0-3): "); break;
            case 8: gParams.RiskAnalysis = readInt("New value (e.g. 1): "); break;
            case 9: {
                std::cout << "New value: ";
                std::getline(std::cin, gParams.OutputFileName);
                break;
            }
            default: std::cout << "Unknown option.\n"; break;
        }
    }
}

// 3. Algorithms
static void doGenerateAssignments() {
    if (!gDataLoaded) { std::cout << "No data loaded. Load data first.\n"; return; }
    int mode = gParams.GenerateAssignments;
    std::cout << "Building flow graph (mode=" << mode << ")...\n";

    bool writeOutput = (gParams.GenerateAssignments != 0);
    if (!AssignmentLogic::generateAssignmentsAndStore(gFlowGraph, gSubmissions, gReviewers, gParams, mode, writeOutput)) {
        std::cout << "Failed to write output file '" << gParams.OutputFileName << "'.\n";
        return;
    }

    if (writeOutput) std::cout << "Assignment output written to '" << gParams.OutputFileName << "'.\n";
    else std::cout << "GenerateAssignments = 0: assignment computed and kept in memory (no output file written).\n";
}

static void menuAlgorithms() {
    while (true) {
        printSep('=', 57);
        std::cout << "   [ Algorithms ]\n";
        printSep('-', 57);
        std::cout << "   [1] Generate Assignments (Max Flow)\n"
                  << "   [2] Run Risk Analysis\n"
                  << "   [0] Back to Main Menu\n";
        printSep('=', 57);
        int choice = readInt("Option: ");
        switch (choice) {
            case 1: doGenerateAssignments(); waitEnter(); break;
            case 2: {
                if (!gDataLoaded) std::cout << "No data loaded.\n";
                else executeAndReportRiskAnalysis(gParams.OutputFileName);
                waitEnter();
                break;
            }
            case 0: return;
            default: std::cout << "Unknown option.\n"; break;
        }
    }
}

// 4. Visualization
static void doViewGraphTerminal() {
    if (!gDataLoaded) { std::cout << "No data loaded. Load data first.\n"; return; }
    std::cout << "Viewing flow graph (mode=" << gParams.GenerateAssignments << ")...\n";
    createGraphs::printFlowGraph(gFlowGraph, gSubmissions, gReviewers);
}

static void doExportGraphDOT() {
    if (!gDataLoaded) { std::cout << "No data loaded. Load data first.\n"; return; }
    std::string filename;
    std::cout << "Enter output DOT filename (e.g. graph.dot): ";
    std::getline(std::cin, filename);
    if (filename.empty()) filename = "graph.dot";
    createGraphs::exportToDOT(gFlowGraph, gSubmissions, gReviewers, filename);
    std::cout << "Graph exported to " << filename << ". Use 'dot -Tpng " << filename << " -o graph.png' to render.\n";
}

static void doShowAugmentingPaths() {
    if (!gDataLoaded) { std::cout << "No data loaded. Load data first.\n"; return; }
    int mode = gParams.GenerateAssignments;
    std::cout << "Running Edmonds-Karp and showing paths (mode=" << mode << ")...\n\n";

    auto pathLogger = [&](const std::vector<int>& pathNodes, double flow) {
        std::cout << "  Path (+flow " << flow << "): ";
        for (size_t i = 0; i < pathNodes.size(); ++i) {
            std::cout << createGraphs::getNodeLabel(pathNodes[i], gSubmissions, gReviewers);
            if (i + 1 < pathNodes.size()) std::cout << " -> ";
        }
        std::cout << "\n";
    };

    Graph<int> tempGraph = createGraphs::buildReviewFlowGraph(gSubmissions, gReviewers, gParams, mode);
    edmondsKarp<int>(&tempGraph, createGraphs::sourceId(), createGraphs::sinkId((int) gSubmissions.size(), (int) gReviewers.size()), pathLogger);
    
    std::cout << "\nTotal assigned flow: " << AssignmentLogic::totalAssignedReviews(tempGraph, gSubmissions, gReviewers) << "\n";
}

static void menuVisualization() {
    while (true) {
        printSep('=', 57);
        std::cout << "   [ Visualization ]\n";
        printSep('-', 57);
        std::cout << "   [1] Show Flow Graph (Terminal)\n"
                  << "   [2] Export Graph to Graphviz (.dot)\n"
                  << "   [3] Run Max-Flow and Show Augmenting Paths\n"
                  << "   [0] Back to Main Menu\n";
        printSep('=', 57);
        int choice = readInt("Option: ");
        switch (choice) {
            case 1: doViewGraphTerminal(); waitEnter(); break;
            case 2: doExportGraphDOT(); waitEnter(); break;
            case 3: doShowAugmentingPaths(); waitEnter(); break;
            case 0: return;
            default: std::cout << "Unknown option.\n"; break;
        }
    }
}

// --------------------------------------------------------------------------
// Main Interactive Menu
// --------------------------------------------------------------------------
static void printMainMenu() {
    printSep('=', 57);
    std::cout << "   Scientific Conference Organization Tool\n"
              << "   DA Programming Project I  -  Spring 2026\n";
    printSep('=', 57);
    if (gDataLoaded)
        std::cout << "   Status : " << gSubmissions.size() << " submissions, " << gReviewers.size() << " reviewers loaded.\n";
    else
        std::cout << "   Status : No data loaded.\n";
    printSep('-', 57);
    std::cout << "   [1] Data Management\n"
              << "   [2] Parameter Configuration\n"
              << "   [3] Algorithms (Assign & Risk)\n"
              << "   [4] Visualization\n"
              << "   [0] Exit\n";
    printSep('=', 57);
}

static void runInteractiveMode() {
    while (true) {
        printMainMenu();
        int choice = readInt("Option: ");
        std::cout << "\n";
        switch (choice) {
            case 1: menuDataManagement(); break;
            case 2: menuParameterConfiguration(); break;
            case 3: menuAlgorithms(); break;
            case 4: menuVisualization(); break;
            case 0: std::cout << "Goodbye!\n"; return;
            default: std::cout << "Unknown option. Try again.\n"; break;
        }
    }
}

// --------------------------------------------------------------------------
// Batch mode:  myProg -b input.csv [risk_output.csv]
// --------------------------------------------------------------------------
static void runBatchMode(const std::string& inputFile, const std::string& riskOutputFile) {
    if (!FileParser::parse(inputFile, gSubmissions, gReviewers, gParams)) {
        std::cerr << "Error: failed to parse '" << inputFile << "'.\n";
        return;
    }
    gDataLoaded = true;
    std::cerr << "Loaded " << gSubmissions.size() << " submissions and " << gReviewers.size() << " reviewers.\n";

    int mode = gParams.GenerateAssignments;
    bool writeOutput = (gParams.GenerateAssignments != 0);

    std::cerr << "Generating assignments (mode=" << mode << ")";
    if (writeOutput) std::cerr << " -> '" << gParams.OutputFileName << "'";
    std::cerr << "...\n";

    if (!AssignmentLogic::generateAssignmentsAndStore(gFlowGraph, gSubmissions, gReviewers, gParams, mode, writeOutput)) {
        std::cerr << "Error: failed to generate/write assignments.\n";
        return;
    }

    if (gParams.RiskAnalysis != 0) {
        const std::string out = riskOutputFile.empty() ? gParams.OutputFileName : riskOutputFile;
        std::cerr << "Running risk analysis (K=" << gParams.RiskAnalysis << ") -> '" << out << "'...\n";
        executeAndReportRiskAnalysis(out);
    }
}

// --------------------------------------------------------------------------
// Entry point
// --------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc >= 2 && std::string(argv[1]) == "-b") {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " -b <input.csv> [<risk_output.csv>]\n";
            return 1;
        }
        runBatchMode(argv[2], (argc >= 4) ? argv[3] : "");
        return 0;
    }

    if (argc > 1) {
        std::cerr << "Usage: " << argv[0] << " [-b <input.csv> [<risk_output.csv>]]\n";
        return 1;
    }

    runInteractiveMode();
    return 0;
}
