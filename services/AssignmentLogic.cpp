#include "services/AssignmentLogic.h"
#include "data_structures/GraphBuilder.h"
#include "algorithms/EdmondKarp.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <tuple>

int AssignmentLogic::totalRequiredReviews(const std::vector<Submission>& submissions, const Parameters& params) {
    return (int) submissions.size() * params.MinReviewsPerSubmission;
}

int AssignmentLogic::totalAssignedReviews(const Graph<int>& flowGraph, const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers) {
    int total = 0;
    const int numSubs = (int) submissions.size();
    const int numRevs = (int) reviewers.size();
    if (numRevs == 0) return 0;
    int firstRevNode = GraphBuilder::reviewerNodeId(0, numSubs);
    int lastRevNode = GraphBuilder::reviewerNodeId(numRevs - 1, numSubs);
    for (int i = 0; i < numSubs; ++i) {
        int subNode = GraphBuilder::submissionNodeId(i);
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

std::vector<int> AssignmentLogic::findRiskyReviewersK1(const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers, const Parameters& params, int mode) {
    std::vector<int> risky;
    const int needed = totalRequiredReviews(submissions, params);
    Graph<int> baseline = GraphBuilder::buildReviewFlowGraph(submissions, reviewers, params, mode);
    edmondsKarp<int>(&baseline, GraphBuilder::sourceId(), GraphBuilder::sinkId((int) submissions.size(), (int) reviewers.size()));
    const int baselineAchieved = totalAssignedReviews(baseline, submissions, reviewers);

    if (baselineAchieved < needed) {
        return risky;
    }

    for (size_t skip = 0; skip < reviewers.size(); ++skip) {
        std::vector<Reviewer> reduced;
        reduced.reserve(reviewers.size() - 1);
        for (size_t i = 0; i < reviewers.size(); ++i) {
            if (i != skip) reduced.push_back(reviewers[i]);
        }
        Graph<int> g = GraphBuilder::buildReviewFlowGraph(submissions, reduced, params, mode);
        edmondsKarp<int>(&g, GraphBuilder::sourceId(), GraphBuilder::sinkId((int) submissions.size(), (int) reduced.size()));
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

bool AssignmentLogic::writeAssignmentsToFile(const Graph<int>& flowGraph, const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers, const Parameters& params, int mode, const std::vector<int>* riskyReviewerIds, int riskValue, const std::string& outputPath) {
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
        int subNode = GraphBuilder::submissionNodeId(i);
        Vertex<int>* subV = flowGraph.findVertex(subNode);
        if (subV == nullptr) continue;
        for (Edge<int>* e : subV->getAdj()) {
            if (e->getFlow() <= 0.0) continue;
            if (numRevs == 0) continue;

            int destNode = e->getDest()->getInfo();
            if (destNode < GraphBuilder::reviewerNodeId(0, numSubs) ||
                destNode > GraphBuilder::reviewerNodeId(numRevs - 1, numSubs)) continue;

            int revIndex = destNode - GraphBuilder::reviewerNodeId(0, numSubs);
            int matchedDomain = GraphBuilder::getMatchedDomain(submissions[i], reviewers[revIndex], mode);
            bySubmission.emplace_back(submissions[i].id, reviewers[revIndex].id, matchedDomain);
            byReviewer.emplace_back(reviewers[revIndex].id, submissions[i].id, matchedDomain);
        }
    }

    auto sortTuple = [](const std::tuple<int, int, int>& a, const std::tuple<int, int, int>& b) {
        if (std::get<0>(a) != std::get<0>(b)) return std::get<0>(a) < std::get<0>(b);
        return std::get<1>(a) < std::get<1>(b);
    };
    std::sort(bySubmission.begin(), bySubmission.end(), sortTuple);
    std::sort(byReviewer.begin(), byReviewer.end(), sortTuple);

    int totalAssigned = totalAssignedReviews(flowGraph, submissions, reviewers);

    if (mode == 0) {
        out << "Mode 0: Assignment generated but output suppressed.\n";
    } else {
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

    if (totalAssigned < totalRequiredReviews(submissions, params)) {
        out << "#SubmissionId,Domain,MissingReviews\n";
        for (int i = 0; i < numSubs; ++i) {
            int subNode = GraphBuilder::submissionNodeId(i);
            Vertex<int>* subV = flowGraph.findVertex(subNode);
            int assignedForSubmission = 0;
            if (subV != nullptr) {
                for (Edge<int>* e : subV->getAdj()) {
                    if (e->getFlow() > 0.0) assignedForSubmission += (int) e->getFlow();
                }
            }
            int missing = params.MinReviewsPerSubmission - assignedForSubmission;
            if (missing > 0) {
                out << submissions[i].id << ", " << submissions[i].primaryTopic << ", " << missing << "\n";
            }
        }
    }

    if (riskyReviewerIds != nullptr && riskValue > 0) {
        writeRiskSection(out, riskValue, *riskyReviewerIds);
    }
    return true;
}

bool AssignmentLogic::generateAssignmentsAndStore(Graph<int>& outGraph, const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers, const Parameters& params, int mode, bool writeOutput, std::function<void(const std::vector<int>&, double)> logger) {
    outGraph = GraphBuilder::buildReviewFlowGraph(submissions, reviewers, params, mode);
    edmondsKarp<int>(&outGraph, GraphBuilder::sourceId(), GraphBuilder::sinkId((int) submissions.size(), (int) reviewers.size()), logger);

    if (writeOutput) {
        return writeAssignmentsToFile(outGraph, submissions, reviewers, params, mode);
    }
    return true;
}
