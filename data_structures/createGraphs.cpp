#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <set>

using namespace std;

#include "./createGraphs.h"

// ------------------------------------------------------------
// Review Assignment Flow Graph
// ------------------------------------------------------------

int createGraphs::sourceId() {
    return 0;
}

// Submissions occupy nodes 1..P
int createGraphs::submissionNodeId(int subIndex) {
    return 1 + subIndex;
}

// Reviewers occupy nodes P+1..P+R
int createGraphs::reviewerNodeId(int revIndex, int numSubs) {
    return 1 + numSubs + revIndex;
}

// Sink is node P+R+1
int createGraphs::sinkId(int numSubs, int numRevs) {
    return 1 + numSubs + numRevs;
}

int createGraphs::getMatchedDomain(const Submission& sub, const Reviewer& rev, int mode) {
    // Mode 0 behaves like mode 1 for domain matching (output is just suppressed).
    int m = (mode == 0) ? 1 : mode;

    // Build applicable topic lists for sub and rev according to mode.
    vector<int> subTopics, revTopics;

    subTopics.push_back(sub.primaryTopic);
    if (m >= 2 && sub.secondaryTopic != -1)
        subTopics.push_back(sub.secondaryTopic);

    revTopics.push_back(rev.primaryTopic);
    if (m == 3 && rev.secondaryTopic != -1)
        revTopics.push_back(rev.secondaryTopic);

    // Return first matching topic (primary topics tried first).
    for (int st : subTopics)
        for (int rt : revTopics)
            if (st == rt) return st;

    return -1; // no match
}

Graph<int> createGraphs::buildReviewFlowGraph(
        const vector<Submission>& submissions,
        const vector<Reviewer>&   reviewers,
        const Parameters&         params,
        int mode) {

    int P   = (int)submissions.size();
    int R   = (int)reviewers.size();
    int src = sourceId();
    int snk = sinkId(P, R);

    Graph<int> g;

    // --- Add vertices ---
    g.addVertex(src);
    for (int i = 0; i < P; ++i)
        g.addVertex(submissionNodeId(i));
    for (int j = 0; j < R; ++j)
        g.addVertex(reviewerNodeId(j, P));
    g.addVertex(snk);

    // --- Edges: source -> submission ---
    // Capacity = MinReviewsPerSubmission: bounds the maximum flow (and thus
    // reviews) that can reach each submission. A complete assignment is
    // achieved when total flow == P * MinReviewsPerSubmission.
    for (int i = 0; i < P; ++i)
        g.addEdge(src, submissionNodeId(i), params.MinReviewsPerSubmission);

    // --- Edges: submission -> reviewer (capacity 1) ---
    // An edge exists when the reviewer can review the submission according to
    // the current mode:
    //   Mode 0/1: sub.primary == rev.primary
    //   Mode 2  : (sub.primary OR sub.secondary) == rev.primary
    //   Mode 3  : (sub.primary OR sub.secondary) == (rev.primary OR rev.secondary)
    for (int i = 0; i < P; ++i) {
        int subNode = submissionNodeId(i);
        for (int j = 0; j < R; ++j) {
            if (getMatchedDomain(submissions[i], reviewers[j], mode) != -1)
                g.addEdge(subNode, reviewerNodeId(j, P), 1);
        }
    }

    // --- Edges: reviewer -> sink ---
    // Each reviewer can review at most MaxReviewsPerReviewer submissions.
    for (int j = 0; j < R; ++j)
        g.addEdge(reviewerNodeId(j, P), snk, params.MaxReviewsPerReviewer);

    return g;
}

// ------------------------------------------------------------
// Visual graph printer
// ------------------------------------------------------------

void createGraphs::printFlowGraph(
        const Graph<int>&         g,
        const vector<Submission>& submissions,
        const vector<Reviewer>&   reviewers) {

    int P   = (int)submissions.size();
    int R   = (int)reviewers.size();
    int src = sourceId();
    int snk = sinkId(P, R);

    // Build a label map: node ID -> display name
    auto label = [&](int id) -> string {
        if (id == src) return "SOURCE";
        if (id == snk) return "SINK";
        // Submission node?
        for (int i = 0; i < P; ++i)
            if (id == submissionNodeId(i))
                return "SUB#" + to_string(submissions[i].id);
        // Reviewer node?
        for (int j = 0; j < R; ++j)
            if (id == reviewerNodeId(j, P))
                return "REV#" + to_string(reviewers[j].id);
        return "NODE#" + to_string(id);
    };

    // Determine column width for alignment
    size_t maxLabelLen = 6; // "SOURCE" / "SINK"
    for (const auto& s : submissions)
        maxLabelLen = max(maxLabelLen, string("SUB#").size() + to_string(s.id).size());
    for (const auto& r : reviewers)
        maxLabelLen = max(maxLabelLen, string("REV#").size() + to_string(r.id).size());

    auto pad = [&](const string& s) -> string {
        return s + string(maxLabelLen - s.size() + 1, ' ');
    };

    cout << string(60, '-') << "\n";
    cout << "  Flow Graph  (" << g.getNumVertex() << " nodes)\n";
    cout << string(60, '-') << "\n";
    cout << "  " << pad("FROM") << "--> " << pad("TO") << "  cap   flow\n";
    cout << string(60, '-') << "\n";

    // Iterate vertices in a predictable order: src, subs, revs, sink
    vector<int> order;
    order.push_back(src);
    for (int i = 0; i < P; ++i) order.push_back(submissionNodeId(i));
    for (int j = 0; j < R; ++j) order.push_back(reviewerNodeId(j, P));
    order.push_back(snk);

    for (int nodeId : order) {
        Vertex<int>* v = g.findVertex(nodeId);
        if (v == nullptr) continue;
        for (Edge<int>* e : v->getAdj()) {
            int destId = e->getDest()->getInfo();
            double cap  = e->getWeight();
            double flow = e->getFlow();
            cout << "  " << pad(label(nodeId))
                 << "--> " << pad(label(destId))
                 << "  " << (int)cap
                 << "     " << (int)flow << "\n";
        }
    }
    cout << string(60, '-') << "\n";
}

