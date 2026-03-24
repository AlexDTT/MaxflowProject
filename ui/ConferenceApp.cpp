#include "ConferenceApp.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <limits>
#include <tuple>

#include "io/FileParser.h"
#include "data_structures/GraphBuilder.h"
#include "algorithms/FordFulkerson.h"
#include "algorithms/EdmondKarp.h"
#include "services/AssignmentLogic.h"

namespace fs = std::filesystem;

ConferenceApp::ConferenceApp() : mDataLoaded(false), mAlgorithm(AlgorithmType::FordFulkerson) {}

int ConferenceApp::run(int argc, char* argv[]) {
    // Parse optional -a flag first
    int argIdx = 1;
    if (argc >= 4 && std::string(argv[argIdx]) == "-a") {
        std::string algo = argv[argIdx + 1];
        if (algo == "ek") {
            mAlgorithm = AlgorithmType::EdmondsKarp;
        } else if (algo == "ff") {
            mAlgorithm = AlgorithmType::FordFulkerson;
        } else {
            std::cerr << "Unknown algorithm '" << algo << "'. Use 'ff' (Ford-Fulkerson) or 'ek' (Edmonds-Karp).\n";
            return 1;
        }
        argIdx += 2;
    }

    if (argc > argIdx && std::string(argv[argIdx]) == "-b") {
        if (argIdx + 1 >= argc) {
            std::cerr << "Usage: " << argv[0] << " [-a ff|ek] -b <input.csv> [<risk_output.csv>]\n";
            return 1;
        }

        std::string inputFile = argv[argIdx + 1];
        std::string riskFile = (argIdx + 2 < argc) ? argv[argIdx + 2] : "";
        runBatchMode(inputFile, riskFile);
        return 0;
    }

    if (argc > argIdx) {
        std::cerr << "Usage: " << argv[0] << " [-a ff|ek] -b <input.csv> [<risk_output.csv>]\n";
        return 1;
    }

    runInteractiveMode();
    return 0;
}

// --------------------------------------------------------------------------
// Helper utilities
// --------------------------------------------------------------------------
void ConferenceApp::printSep(char c, int n) const {
    std::cout << "   " << std::string(n, c) << "\n";
}

void ConferenceApp::waitEnter() const {
    std::cout << "\nPress Enter to continue...";
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int ConferenceApp::readInt(const std::string& prompt) const {
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

// --------------------------------------------------------------------------
// Core Logic
// --------------------------------------------------------------------------
void ConferenceApp::loadFile(const std::string& path) {
    std::vector<Submission> subs;
    std::vector<Reviewer>   revs;
    Parameters               p;

    if (FileParser::parse(path, subs, revs, p)) {
        mSubmissions = std::move(subs);
        mReviewers   = std::move(revs);
        mParams      = p;
        mDataLoaded  = true;
        std::cout << "Data loaded: " << mSubmissions.size() << " submissions, " << mReviewers.size() << " reviewers.\n";
    } else {
        std::cout << "Failed to load data. Check error messages above.\n";
    }
}

void ConferenceApp::executeAndReportRiskAnalysis(const std::string& outputPath) {
    if (mParams.RiskAnalysis == 0) {
        std::cout << "RiskAnalysis = 0: no risk analysis requested.\n";
        return;
    }
    if (mParams.RiskAnalysis != 1) {
        std::cout << "RiskAnalysis = " << mParams.RiskAnalysis << " is not implemented yet (only K=1).\n";
        return;
    }

    int mode = mParams.GenerateAssignments;
    if (!AssignmentLogic::generateAssignmentsAndStore(mFlowGraph, mSubmissions, mReviewers, mParams, mode, false, nullptr, mAlgorithm)) {
        std::cout << "Failed to compute assignment baseline for risk analysis.\n";
        return;
    }

    std::vector<int> risky = AssignmentLogic::findRiskyReviewersK1(mSubmissions, mReviewers, mParams, mode, mAlgorithm);

    if (!AssignmentLogic::writeAssignmentsToFile(mFlowGraph, mSubmissions, mReviewers, mParams, mode, &risky, 1, outputPath)) {
        std::cout << "Failed to write risk analysis to '" << outputPath << "'.\n";
        return;
    }
    std::cout << "Risk analysis (K=1) written to '" << outputPath << "'.\n";
}

// --------------------------------------------------------------------------
// Sub-menu actions
// --------------------------------------------------------------------------
void ConferenceApp::doLoadSpecificFile() {
    std::string path;
    std::cout << "Enter the path to the input CSV file: ";
    std::getline(std::cin, path);
    while (!path.empty() && (path.front() == ' ' || path.front() == '"')) path.erase(path.begin());
    while (!path.empty() && (path.back() == ' ' || path.back() == '"')) path.pop_back();
    loadFile(path);
}

void ConferenceApp::doBrowseInputs() {
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

void ConferenceApp::doAddSubmission() {
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

    mSubmissions.emplace_back(id, title, authors, email, primary, secondary);
    mDataLoaded = true;
    std::cout << "Submission added successfully.\n";
}

void ConferenceApp::doAddReviewer() {
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

    mReviewers.emplace_back(id, name, email, primary, secondary);
    mDataLoaded = true;
    std::cout << "Reviewer added successfully.\n";
}

void ConferenceApp::doViewData() const {
    if (!mDataLoaded) { std::cout << "No data loaded.\n"; return; }
    std::cout << "Submissions (" << mSubmissions.size() << "):\n";
    for (const auto& s : mSubmissions) s.print();
    printSep();
    std::cout << "Reviewers (" << mReviewers.size() << "):\n";
    for (const auto& r : mReviewers) r.print();
}

void ConferenceApp::menuDataManagement() {
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

void ConferenceApp::menuParameterConfiguration() {
    while (true) {
        printSep('=', 57);
        std::cout << "   [ Parameter Configuration ]\n";
        printSep('-', 57);
        std::cout << "   [1] MinReviewsPerSubmission    : " << mParams.MinReviewsPerSubmission << "\n"
                  << "   [2] MaxReviewsPerReviewer      : " << mParams.MaxReviewsPerReviewer << "\n"
                  << "   [3] PrimaryReviewerExpertise   : " << mParams.PrimaryReviewerExpertise << "\n"
                  << "   [4] SecondaryReviewerExpertise : " << mParams.SecondaryReviewerExpertise << "\n"
                  << "   [5] PrimarySubmissionDomain    : " << mParams.PrimarySubmissionDomain << "\n"
                  << "   [6] SecondarySubmissionDomain  : " << mParams.SecondarySubmissionDomain << "\n"
                  << "   [7] GenerateAssignments (mode) : " << mParams.GenerateAssignments << "\n"
                  << "   [8] RiskAnalysis (K-value)     : " << mParams.RiskAnalysis << "\n"
                  << "   [9] OutputFileName             : " << mParams.OutputFileName << "\n"
                  << "   [0] Back to Main Menu\n";
        printSep('=', 57);
        int choice = readInt("Option (select number to edit): ");
        if (choice == 0) return;
        
        switch(choice) {
            case 1: mParams.MinReviewsPerSubmission = readInt("New value: "); break;
            case 2: mParams.MaxReviewsPerReviewer = readInt("New value: "); break;
            case 3: mParams.PrimaryReviewerExpertise = readInt("New value: "); break;
            case 4: mParams.SecondaryReviewerExpertise = readInt("New value: "); break;
            case 5: mParams.PrimarySubmissionDomain = readInt("New value: "); break;
            case 6: mParams.SecondarySubmissionDomain = readInt("New value: "); break;
            case 7: mParams.GenerateAssignments = readInt("New value (0-3): "); break;
            case 8: mParams.RiskAnalysis = readInt("New value (e.g. 1): "); break;
            case 9: {
                std::cout << "New value: ";
                std::getline(std::cin, mParams.OutputFileName);
                break;
            }
            default: std::cout << "Unknown option.\n"; break;
        }
    }
}

void ConferenceApp::doGenerateAssignments() {
    if (!mDataLoaded) { std::cout << "No data loaded. Load data first.\n"; return; }

    std::cout << "   Select max-flow algorithm:\n"
              << "   [1] Ford-Fulkerson (DFS-based)\n"
              << "   [2] Edmonds-Karp (BFS-based)\n";
    int algoChoice = readInt("Algorithm: ");
    if (algoChoice == 1) mAlgorithm = AlgorithmType::FordFulkerson;
    else if (algoChoice == 2) mAlgorithm = AlgorithmType::EdmondsKarp;
    else { std::cout << "Invalid choice.\n"; return; }

    int mode = mParams.GenerateAssignments;
    std::string algoName = (mAlgorithm == AlgorithmType::FordFulkerson) ? "Ford-Fulkerson" : "Edmonds-Karp";
    std::cout << "Building flow graph (mode=" << mode << ", algorithm=" << algoName << ")...\n";

    bool writeOutput = (mParams.GenerateAssignments != 0);
    if (!AssignmentLogic::generateAssignmentsAndStore(mFlowGraph, mSubmissions, mReviewers, mParams, mode, writeOutput, nullptr, mAlgorithm)) {
        std::cout << "Failed to write output file '" << mParams.OutputFileName << "'.\n";
        return;
    }

    if (writeOutput) std::cout << "Assignment output written to '" << mParams.OutputFileName << "'.\n";
    else std::cout << "GenerateAssignments = 0: assignment computed and kept in memory (no output file written).\n";
}

void ConferenceApp::menuAlgorithms() {
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
                if (!mDataLoaded) std::cout << "No data loaded.\n";
                else executeAndReportRiskAnalysis(mParams.OutputFileName);
                waitEnter();
                break;
            }
            case 0: return;
            default: std::cout << "Unknown option.\n"; break;
        }
    }
}

void ConferenceApp::doViewGraphTerminal() const {
    if (!mDataLoaded) { std::cout << "No data loaded. Load data first.\n"; return; }
    std::cout << "Viewing flow graph (mode=" << mParams.GenerateAssignments << ")...\n";
    GraphBuilder::printFlowGraph(mFlowGraph, mSubmissions, mReviewers);
}

void ConferenceApp::doExportGraphDOT() const {
    if (!mDataLoaded) { std::cout << "No data loaded. Load data first.\n"; return; }
    std::string filename;
    std::cout << "Enter output DOT filename (e.g. graph.dot): ";
    std::getline(std::cin, filename);
    if (filename.empty()) filename = "graph.dot";
    GraphBuilder::exportToDOT(mFlowGraph, mSubmissions, mReviewers, filename);
    std::cout << "Graph exported to " << filename << ". Use 'dot -Tpng " << filename << " -o graph.png' to render.\n";
}

void ConferenceApp::doShowAugmentingPaths() const {
    if (!mDataLoaded) { std::cout << "No data loaded. Load data first.\n"; return; }
    int mode = mParams.GenerateAssignments;
    std::string algoName = (mAlgorithm == AlgorithmType::FordFulkerson) ? "Ford-Fulkerson" : "Edmonds-Karp";
    std::cout << "Running " << algoName << " and showing paths (mode=" << mode << ")...\n\n";

    auto pathLogger = [&](const std::vector<int>& pathNodes, double flow) {
        std::cout << "  Path (+flow " << flow << "): ";
        for (size_t i = 0; i < pathNodes.size(); ++i) {
            std::cout << GraphBuilder::getNodeLabel(pathNodes[i], mSubmissions, mReviewers);
            if (i + 1 < pathNodes.size()) std::cout << " -> ";
        }
        std::cout << "\n";
    };

    Graph<int> tempGraph = GraphBuilder::buildReviewFlowGraph(mSubmissions, mReviewers, mParams, mode);
    int source = GraphBuilder::sourceId();
    int sink = GraphBuilder::sinkId((int) mSubmissions.size(), (int) mReviewers.size());

    if (mAlgorithm == AlgorithmType::EdmondsKarp) {
        edmondsKarp<int>(&tempGraph, source, sink, pathLogger);
    } else {
        fordFulkerson<int>(&tempGraph, source, sink, pathLogger);
    }
    
    std::cout << "\nTotal assigned flow: " << AssignmentLogic::totalAssignedReviews(tempGraph, mSubmissions, mReviewers) << "\n";
}

void ConferenceApp::menuVisualization() {
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

void ConferenceApp::printMainMenu() const {
    printSep('=', 57);
    std::cout << "   Scientific Conference Organization Tool\n"
              << "   DA Programming Project I  -  Spring 2026\n";
    printSep('=', 57);
    if (mDataLoaded)
        std::cout << "   Status : " << mSubmissions.size() << " submissions, " << mReviewers.size() << " reviewers loaded.\n";
    else
        std::cout << "   Status : No data loaded.\n";
    std::cout << "   Algorithm: " << (mAlgorithm == AlgorithmType::FordFulkerson ? "Ford-Fulkerson (DFS)" : "Edmonds-Karp (BFS)") << "\n";
    printSep('-', 57);
    std::cout << "   [1] Data Management\n"
              << "   [2] Parameter Configuration\n"
              << "   [3] Algorithms (Assign & Risk)\n"
              << "   [4] Visualization\n"
              << "   [0] Exit\n";
    printSep('=', 57);
}

void ConferenceApp::runInteractiveMode() {
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

void ConferenceApp::runBatchMode(const std::string& inputFile, const std::string& riskOutputFile) {
    if (!FileParser::parse(inputFile, mSubmissions, mReviewers, mParams)) {
        std::cerr << "Error: failed to parse '" << inputFile << "'.\n";
        return;
    }
    mDataLoaded = true;
    std::string algoName = (mAlgorithm == AlgorithmType::FordFulkerson) ? "Ford-Fulkerson" : "Edmonds-Karp";
    std::cerr << "Loaded " << mSubmissions.size() << " submissions and " << mReviewers.size() << " reviewers.\n";
    std::cerr << "Algorithm: " << algoName << "\n";

    int mode = mParams.GenerateAssignments;
    bool writeOutput = (mParams.GenerateAssignments != 0);

    std::cerr << "Generating assignments (mode=" << mode << ")";
    if (writeOutput) std::cerr << " -> '" << mParams.OutputFileName << "'";
    std::cerr << "...\n";

    if (!AssignmentLogic::generateAssignmentsAndStore(mFlowGraph, mSubmissions, mReviewers, mParams, mode, writeOutput, nullptr, mAlgorithm)) {
        std::cerr << "Error: failed to generate/write assignments.\n";
        return;
    }

    if (mParams.RiskAnalysis != 0) {
        const std::string out = riskOutputFile.empty() ? mParams.OutputFileName : riskOutputFile;
        std::cerr << "Running risk analysis (K=" << mParams.RiskAnalysis << ") -> '" << out << "'...\n";
        executeAndReportRiskAnalysis(out);
    }
}
