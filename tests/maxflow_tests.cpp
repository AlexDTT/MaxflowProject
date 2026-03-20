#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "data_structures/Graph.h"
#include "data_structures/GraphBuilder.h"
#include "algorithms/EdmondKarp.h"
#include "models/Parameters.h"
#include "models/Reviewer.h"
#include "models/Submission.h"

static double getEdgeFlow(const Graph<int>& g, int from, int to) {
    Vertex<int>* v = g.findVertex(from);
    if (v == nullptr) return 0;
    for (Edge<int>* e : v->getAdj()) {
        if (e->getDest()->getInfo() == to) return e->getFlow();
    }
    return 0;
}

static double totalFlowFromSource(const Graph<int>& g, int source) {
    Vertex<int>* s = g.findVertex(source);
    if (s == nullptr) return 0;
    double total = 0;
    for (Edge<int>* e : s->getAdj()) total += e->getFlow();
    return total;
}

static int assignedToSubmission(const Graph<int>& g, int subIndex, int numSubs, int numRevs) {
    int subNode = GraphBuilder::submissionNodeId(subIndex);
    Vertex<int>* subV = g.findVertex(subNode);
    if (subV == nullptr) return 0;

    int firstRevNode = (numRevs > 0) ? GraphBuilder::reviewerNodeId(0, numSubs) : 1;
    int lastRevNode = (numRevs > 0) ? GraphBuilder::reviewerNodeId(numRevs - 1, numSubs) : 0;

    int total = 0;
    for (Edge<int>* e : subV->getAdj()) {
        int destNode = e->getDest()->getInfo();
        if (numRevs > 0 && destNode >= firstRevNode && destNode <= lastRevNode && e->getFlow() > 0.0) {
            total += (int) e->getFlow();
        }
    }
    return total;
}

static int assignedByReviewer(const Graph<int>& g, int revIndex, int numSubs, int numRevs) {
    if (numRevs == 0) return 0;
    int revNode = GraphBuilder::reviewerNodeId(revIndex, numSubs);
    int sink = GraphBuilder::sinkId(numSubs, numRevs);
    return (int) getEdgeFlow(g, revNode, sink);
}

static Graph<int> runAssignmentFlow(
    const std::vector<Submission>& submissions,
    const std::vector<Reviewer>& reviewers,
    int minReviews,
    int maxReviews,
    int mode) {

    Parameters params;
    params.MinReviewsPerSubmission = minReviews;
    params.MaxReviewsPerReviewer = maxReviews;

    Graph<int> g = GraphBuilder::buildReviewFlowGraph(submissions, reviewers, params, mode);
    edmondsKarp(
        &g,
        GraphBuilder::sourceId(),
        GraphBuilder::sinkId((int) submissions.size(), (int) reviewers.size())
    );
    return g;
}

static bool nearlyEqual(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}

static void expect(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

static std::vector<Submission> makeComplexSubmissions() {
    return {
        Submission(101, "S1", "A", "a@x", 1, 2),
        Submission(102, "S2", "B", "b@x", 2, 3),
        Submission(103, "S3", "C", "c@x", 3, 1),
        Submission(104, "S4", "D", "d@x", 4)
    };
}

static std::vector<Reviewer> makeComplexReviewers() {
    return {
        Reviewer(201, "R1", "r1@x", 1, 3),
        Reviewer(202, "R2", "r2@x", 2, 4),
        Reviewer(203, "R3", "r3@x", 3, 2),
        Reviewer(204, "R4", "r4@x", 4)
    };
}

static std::vector<std::vector<int>> compatibilityMatrix(
    const std::vector<Submission>& submissions,
    const std::vector<Reviewer>& reviewers,
    int mode) {

    std::vector<std::vector<int>> canAssign(
        submissions.size(),
        std::vector<int>(reviewers.size(), 0)
    );

    for (size_t i = 0; i < submissions.size(); ++i) {
        for (size_t j = 0; j < reviewers.size(); ++j) {
            canAssign[i][j] =
                (GraphBuilder::getMatchedDomain(submissions[i], reviewers[j], mode) != -1) ? 1 : 0;
        }
    }

    return canAssign;
}

static int bruteForceMaxAssignmentsRec(
    const std::vector<std::vector<int>>& canAssign,
    int edgeIdx,
    int minReviewsPerSubmission,
    int maxReviewsPerReviewer,
    std::vector<int>& submissionAssigned,
    std::vector<int>& reviewerAssigned,
    int current,
    int& best) {

    int subCount = (int) canAssign.size();
    int revCount = subCount == 0 ? 0 : (int) canAssign[0].size();
    int totalEdges = subCount * revCount;

    if (edgeIdx == totalEdges) {
        if (current > best) best = current;
        return best;
    }

    int remaining = totalEdges - edgeIdx;
    if (current + remaining <= best) return best;

    int s = edgeIdx / revCount;
    int r = edgeIdx % revCount;

    bruteForceMaxAssignmentsRec(
        canAssign, edgeIdx + 1, minReviewsPerSubmission, maxReviewsPerReviewer,
        submissionAssigned, reviewerAssigned, current, best
    );

    if (canAssign[s][r] == 1 &&
        submissionAssigned[s] < minReviewsPerSubmission &&
        reviewerAssigned[r] < maxReviewsPerReviewer) {

        submissionAssigned[s]++;
        reviewerAssigned[r]++;
        bruteForceMaxAssignmentsRec(
            canAssign, edgeIdx + 1, minReviewsPerSubmission, maxReviewsPerReviewer,
            submissionAssigned, reviewerAssigned, current + 1, best
        );
        submissionAssigned[s]--;
        reviewerAssigned[r]--;
    }

    return best;
}

static int bruteForceMaxAssignments(
    const std::vector<Submission>& submissions,
    const std::vector<Reviewer>& reviewers,
    int mode,
    int minReviewsPerSubmission,
    int maxReviewsPerReviewer) {

    std::vector<std::vector<int>> canAssign = compatibilityMatrix(submissions, reviewers, mode);
    std::vector<int> submissionAssigned(submissions.size(), 0);
    std::vector<int> reviewerAssigned(reviewers.size(), 0);

    int best = 0;
    bruteForceMaxAssignmentsRec(
        canAssign,
        0,
        minReviewsPerSubmission,
        maxReviewsPerReviewer,
        submissionAssigned,
        reviewerAssigned,
        0,
        best
    );

    return best;
}

static void verifyFlowRespectsCaps(
    const Graph<int>& g,
    int subCount,
    int revCount,
    int minReviewsPerSubmission,
    int maxReviewsPerReviewer) {

    for (int i = 0; i < subCount; ++i) {
        int subNode = GraphBuilder::submissionNodeId(i);
        Vertex<int>* subV = g.findVertex(subNode);
        expect(subV != nullptr, "Submission node missing from graph");

        double subOut = 0;
        for (Edge<int>* e : subV->getAdj()) {
            if (e->getWeight() > 0) subOut += e->getFlow();
        }
        expect(subOut <= minReviewsPerSubmission + 1e-9, "Submission exceeds configured minimum-cap edge");
    }

    for (int j = 0; j < revCount; ++j) {
        int revNode = GraphBuilder::reviewerNodeId(j, subCount);
        int sinkNode = GraphBuilder::sinkId(subCount, revCount);
        double flowToSink = getEdgeFlow(g, revNode, sinkNode);
        expect(flowToSink <= maxReviewsPerReviewer + 1e-9, "Reviewer exceeds maximum allowed reviews");
    }
}

static void testAllModesAllMinMaxCombinations() {
    std::vector<Submission> submissions = makeComplexSubmissions();
    std::vector<Reviewer> reviewers = makeComplexReviewers();

    const std::vector<int> modes = {0, 1, 2, 3};
    const std::vector<int> mins = {1, 2, 3};
    const std::vector<int> maxs = {1, 2, 3};

    for (int mode : modes) {
        for (int minR : mins) {
            for (int maxR : maxs) {
                Parameters params;
                params.MinReviewsPerSubmission = minR;
                params.MaxReviewsPerReviewer = maxR;

                Graph<int> g = GraphBuilder::buildReviewFlowGraph(submissions, reviewers, params, mode);
                int source = GraphBuilder::sourceId();
                int sink = GraphBuilder::sinkId((int) submissions.size(), (int) reviewers.size());

                edmondsKarp(&g, source, sink);

                int expected = bruteForceMaxAssignments(submissions, reviewers, mode, minR, maxR);
                double observed = totalFlowFromSource(g, source);

                std::ostringstream caseLabel;
                caseLabel << "(mode=" << mode << ", min=" << minR << ", max=" << maxR << ")";

                expect(nearlyEqual(observed, (double) expected),
                       "Flow mismatch " + caseLabel.str());

                verifyFlowRespectsCaps(
                    g,
                    (int) submissions.size(),
                    (int) reviewers.size(),
                    minR,
                    maxR
                );
            }
        }
    }
}

static void testModeMonotonicity() {
    std::vector<Submission> submissions = makeComplexSubmissions();
    std::vector<Reviewer> reviewers = makeComplexReviewers();

    const std::vector<int> mins = {1, 2, 3};
    const std::vector<int> maxs = {1, 2, 3};

    for (int minR : mins) {
        for (int maxR : maxs) {
            Parameters params;
            params.MinReviewsPerSubmission = minR;
            params.MaxReviewsPerReviewer = maxR;

            double flowByMode[4] = {0, 0, 0, 0};

            for (int mode = 0; mode <= 3; ++mode) {
                Graph<int> g = GraphBuilder::buildReviewFlowGraph(submissions, reviewers, params, mode);
                int source = GraphBuilder::sourceId();
                int sink = GraphBuilder::sinkId((int) submissions.size(), (int) reviewers.size());
                edmondsKarp(&g, source, sink);
                flowByMode[mode] = totalFlowFromSource(g, source);
            }

            std::ostringstream base;
            base << "for min=" << minR << " max=" << maxR;

            expect(nearlyEqual(flowByMode[0], flowByMode[1]), "Mode 0 must equal mode 1 " + base.str());
            expect(flowByMode[2] + 1e-9 >= flowByMode[1], "Mode 2 should be >= mode 1 " + base.str());
            expect(flowByMode[3] + 1e-9 >= flowByMode[2], "Mode 3 should be >= mode 2 " + base.str());
        }
    }
}

static void testExample1FullySatisfiableHappyPath() {
    std::vector<Submission> submissions = {
        Submission(10, "Paper Alpha", "Alice Smith", "alice@uni.edu", 1),
        Submission(20, "Paper Beta", "Bob Jones", "bob@uni.edu", 1),
        Submission(30, "Paper Gamma", "Carol Lee", "carol@uni.edu", 2)
    };

    std::vector<Reviewer> reviewers = {
        Reviewer(1, "John Doe", "jdoe@uni.edu", 1),
        Reviewer(2, "Jane Roe", "jroe@uni.edu", 1),
        Reviewer(3, "Mark Tan", "mtan@uni.edu", 1),
        Reviewer(4, "Sara Kim", "skim@uni.edu", 2),
        Reviewer(5, "Paul Cruz", "pcruz@uni.edu", 2)
    };

    const int minReviews = 2;
    const int maxReviews = 2;
    const int mode = 1;

    Graph<int> g = runAssignmentFlow(submissions, reviewers, minReviews, maxReviews, mode);
    int source = GraphBuilder::sourceId();

    expect(nearlyEqual(totalFlowFromSource(g, source), 6), "Example 1 should assign all 6 required reviews");

    for (size_t i = 0; i < submissions.size(); ++i) {
        expect(assignedToSubmission(g, (int) i, (int) submissions.size(), (int) reviewers.size()) == 2,
               "Example 1: each submission must receive exactly 2 reviews");
    }

    for (size_t j = 0; j < reviewers.size(); ++j) {
        expect(assignedByReviewer(g, (int) j, (int) submissions.size(), (int) reviewers.size()) <= 2,
               "Example 1: no reviewer can exceed 2 reviews");
    }
}

static void testExample2PartiallyUnsatisfiable() {
    std::vector<Submission> submissions = {
        Submission(5, "Paper One", "Alice Smith", "alice@uni.edu", 1),
        Submission(6, "Paper Two", "Bob Jones", "bob@uni.edu", 3)
    };

    std::vector<Reviewer> reviewers = {
        Reviewer(1, "John Doe", "jdoe@uni.edu", 1),
        Reviewer(2, "Jane Roe", "jroe@uni.edu", 1)
    };

    const int minReviews = 2;
    const int maxReviews = 2;
    const int mode = 1;

    Graph<int> g = runAssignmentFlow(submissions, reviewers, minReviews, maxReviews, mode);
    int source = GraphBuilder::sourceId();

    expect(nearlyEqual(totalFlowFromSource(g, source), 2), "Example 2 should assign only 2 reviews in total");

    int sub0Assigned = assignedToSubmission(g, 0, (int) submissions.size(), (int) reviewers.size());
    int sub1Assigned = assignedToSubmission(g, 1, (int) submissions.size(), (int) reviewers.size());

    expect(sub0Assigned == 2, "Example 2: submission 5 should receive 2 reviews");
    expect(sub1Assigned == 0, "Example 2: submission 6 should receive 0 reviews");

    int sub1Missing = minReviews - sub1Assigned;
    expect(sub1Missing == 2, "Example 2: submission 6 should report 2 missing reviews");
}

static void testExample3RiskAnalysisOneCriticalReviewer() {
    std::vector<Submission> submissions = {
        Submission(10, "Paper Alpha", "Alice Smith", "alice@uni.edu", 1),
        Submission(20, "Paper Beta", "Bob Jones", "bob@uni.edu", 2)
    };

    std::vector<Reviewer> reviewers = {
        Reviewer(1, "John Doe", "jdoe@uni.edu", 1),
        Reviewer(2, "Jane Roe", "jroe@uni.edu", 1),
        Reviewer(3, "Mark Tan", "mtan@uni.edu", 2)
    };

    const int minReviews = 1;
    const int maxReviews = 2;
    const int mode = 1;
    const int totalRequired = (int) submissions.size() * minReviews;

    Graph<int> baseline = runAssignmentFlow(submissions, reviewers, minReviews, maxReviews, mode);
    expect((int) totalFlowFromSource(baseline, GraphBuilder::sourceId()) == totalRequired,
           "Example 3 baseline should be feasible");

    for (int removedId : {1, 2, 3}) {
        std::vector<Reviewer> reduced;
        for (const Reviewer& r : reviewers) {
            if (r.id != removedId) reduced.push_back(r);
        }

        Graph<int> gReduced = runAssignmentFlow(submissions, reduced, minReviews, maxReviews, mode);
        int flow = (int) totalFlowFromSource(gReduced, GraphBuilder::sourceId());

        if (removedId == 3) {
            expect(flow < totalRequired, "Example 3: removing reviewer 3 must make assignment infeasible");
        } else {
            expect(flow == totalRequired, "Example 3: removing reviewer 1 or 2 should keep assignment feasible");
        }
    }
}

static void testClassicEdmondsKarpMaxFlow() {
    Graph<int> g;
    for (int i = 1; i <= 6; ++i) g.addVertex(i);

    g.addEdge(1, 2, 3);
    g.addEdge(1, 3, 2);
    g.addEdge(2, 5, 4);
    g.addEdge(2, 4, 3);
    g.addEdge(2, 3, 1);
    g.addEdge(3, 5, 2);
    g.addEdge(4, 6, 2);
    g.addEdge(5, 6, 3);

    edmondsKarp(&g, 1, 6);

    expect(nearlyEqual(totalFlowFromSource(g, 1), 5), "Classic graph max-flow should be 5");
    expect(nearlyEqual(getEdgeFlow(g, 1, 2), 3), "Edge 1->2 should carry flow 3");
    expect(nearlyEqual(getEdgeFlow(g, 1, 3), 2), "Edge 1->3 should carry flow 2");
}

static void testInvalidSourceThrows() {
    Graph<int> g;
    g.addVertex(1);
    g.addVertex(2);
    g.addEdge(1, 2, 1);

    bool thrown = false;
    try {
        edmondsKarp(&g, 99, 2);
    } catch (const std::logic_error&) {
        thrown = true;
    }
    expect(thrown, "Invalid source should throw std::logic_error");
}

static void testPrimaryModeAssignmentFlow() {
    std::vector<Submission> submissions = {
        Submission(101, "S1", "A", "a@x", 1),
        Submission(102, "S2", "B", "b@x", 2)
    };

    std::vector<Reviewer> reviewers = {
        Reviewer(201, "R1", "r1@x", 1),
        Reviewer(202, "R2", "r2@x", 2)
    };

    Parameters params;
    params.MinReviewsPerSubmission = 1;
    params.MaxReviewsPerReviewer = 1;

    Graph<int> g = GraphBuilder::buildReviewFlowGraph(submissions, reviewers, params, 1);
    int source = GraphBuilder::sourceId();
    int sink = GraphBuilder::sinkId((int) submissions.size(), (int) reviewers.size());

    edmondsKarp(&g, source, sink);

    expect(nearlyEqual(totalFlowFromSource(g, source), 2), "Primary mode should assign both submissions");
}

static void testSecondarySubmissionModeEnablesMatch() {
    std::vector<Submission> submissions = {
        Submission(301, "S3", "A", "a@x", 3, 1),
        Submission(302, "S1", "B", "a@x", 2, 1),
        Submission(303, "S2", "C", "a@x", 1, 2)
    };

    std::vector<Reviewer> reviewers = {
        Reviewer(401, "R4", "r4@x", 1),
        Reviewer(402, "R3", "r4@x", 2),
        Reviewer(403, "R2", "r4@x", 1)
    };

    Parameters params;
    params.MinReviewsPerSubmission = 1;
    params.MaxReviewsPerReviewer = 1;

    Graph<int> gMode1 = GraphBuilder::buildReviewFlowGraph(submissions, reviewers, params, 1);
    Graph<int> gMode2 = GraphBuilder::buildReviewFlowGraph(submissions, reviewers, params, 2);

    int source = GraphBuilder::sourceId();
    int sink = GraphBuilder::sinkId((int) submissions.size(), (int) reviewers.size());

    edmondsKarp(&gMode1, source, sink);
    edmondsKarp(&gMode2, source, sink);

    expect(nearlyEqual(totalFlowFromSource(gMode1, source), 2), "Mode 1 should assign 2 reviews with primary-only matching");
    expect(nearlyEqual(totalFlowFromSource(gMode2, source), 3), "Mode 2 should assign all 3 reviews by using secondary submission topics");
}

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"Classic Edmonds-Karp max-flow", testClassicEdmondsKarpMaxFlow},
        {"Invalid source throws", testInvalidSourceThrows},
        {"Primary mode assignment flow", testPrimaryModeAssignmentFlow},
        {"Mode 2 secondary submission matching", testSecondarySubmissionModeEnablesMatch},
        {"Example 1 fully satisfiable", testExample1FullySatisfiableHappyPath},
        {"Example 2 partially unsatisfiable", testExample2PartiallyUnsatisfiable},
        {"Example 3 risk critical reviewer", testExample3RiskAnalysisOneCriticalReviewer},
        {"All modes x min/max combinations", testAllModesAllMinMaxCombinations},
        {"Mode monotonicity checks", testModeMonotonicity}
    };

    int failures = 0;
    for (const auto& test : tests) {
        try {
            test.second();
            std::cout << "[PASS] " << test.first << "\n";
        } catch (const std::exception& ex) {
            ++failures;
            std::cout << "[FAIL] " << test.first << " - " << ex.what() << "\n";
        } catch (...) {
            ++failures;
            std::cout << "[FAIL] " << test.first << " - unknown error\n";
        }
    }

    std::cout << "\nTotal: " << tests.size() << ", Failures: " << failures << "\n";
    return failures == 0 ? 0 : 1;
}
