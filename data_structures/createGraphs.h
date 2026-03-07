#ifndef SRC_CREATE_GRAPHS_H
#define SRC_CREATE_GRAPHS_H

#include "Graph.h"
#include "../utils/Submission.h"
#include "../utils/Reviewer.h"
#include "../utils/Parameters.h"

#include <vector>

using namespace std;

class createGraphs{
public:

    /**
     * Builds a Max-Flow graph for the review assignment problem.
     *
     * Node ID scheme (stored as int):
     *   0          = super-source
     *   1 .. P     = submission nodes  (index i -> i+1)
     *   P+1..P+R   = reviewer nodes    (index j -> P+1+j)
     *   P+R+1      = super-sink
     *
     * Edges and capacities:
     *   source -> submission_i : MinReviewsPerSubmission
     *   submission_i -> reviewer_j : 1  (when domains match per mode)
     *   reviewer_j -> sink : MaxReviewsPerReviewer
     *
     * Domain matching rules by mode:
     *   0 (same as 1, output suppressed): sub.primary == rev.primary
     *   1 : sub.primary == rev.primary
     *   2 : (sub.primary OR sub.secondary) == rev.primary
     *   3 : (sub.primary OR sub.secondary) == (rev.primary OR rev.secondary)
     *
     * @param submissions  Vector of submissions
     * @param reviewers    Vector of reviewers
     * @param params       Problem parameters
     * @param mode         Value of GenerateAssignments (0, 1, 2 or 3)
     * @return             The constructed flow graph
     */
    static Graph<int> buildReviewFlowGraph(
        const vector<Submission>& submissions,
        const vector<Reviewer>&   reviewers,
        const Parameters&         params,
        int mode);

    /**
     * Returns the topic on which submission and reviewer match in the given mode.
     * Returns -1 if they do not match.
     * Primary topics are preferred over secondary when multiple matches exist.
     *
     * @param sub   Submission
     * @param rev   Reviewer
     * @param mode  GenerateAssignments mode (0-3)
     */
    static int getMatchedDomain(const Submission& sub, const Reviewer& rev, int mode);

    /**
     * Maps a submission vector index to its graph node ID.
     * @param subIndex  0-based index into the submissions vector
     */
    static int submissionNodeId(int subIndex);

    /**
     * Maps a reviewer vector index to its graph node ID.
     * @param revIndex  0-based index into the reviewers vector
     * @param numSubs   number of submissions P
     */
    static int reviewerNodeId(int revIndex, int numSubs);

    /** Returns the super-source node ID (always 0). */
    static int sourceId();

    /**
     * Returns the super-sink node ID.
     * @param numSubs  P
     * @param numRevs  R
     */
    static int sinkId(int numSubs, int numRevs);

    /**
     * Prints a human-readable representation of the review flow graph to stdout.
     * Each edge is shown as:  NodeLabel --> NodeLabel  [cap=C, flow=F]
     * Node labels map internal IDs back to meaningful names (SOURCE, SUB#id,
     * REV#id, SINK).
     *
     * @param g            The flow graph (after or before running max-flow)
     * @param submissions  Vector of submissions (for label lookup)
     * @param reviewers    Vector of reviewers   (for label lookup)
     */
    static void printFlowGraph(
        const Graph<int>&         g,
        const vector<Submission>& submissions,
        const vector<Reviewer>&   reviewers);
};

#endif
