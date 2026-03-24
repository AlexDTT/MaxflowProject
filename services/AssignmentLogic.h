/**
 * @file AssignmentLogic.h
 * @brief Logic for assigning reviewers to submissions based on Max Flow.
 */

#ifndef ASSIGNMENTLOGIC_H
#define ASSIGNMENTLOGIC_H

#include <vector>
#include <string>
#include <functional>
#include "data_structures/Graph.h"
#include "models/Submission.h"
#include "models/Reviewer.h"
#include "models/Parameters.h"

/**
 * @enum AlgorithmType
 * @brief Selects which max-flow algorithm to use.
 */
enum class AlgorithmType {
    FordFulkerson, ///< DFS-based Ford-Fulkerson algorithm. O(F * E).
    EdmondsKarp    ///< BFS-based Edmonds-Karp algorithm. O(V * E^2).
};

/**
 * @class AssignmentLogic
 * @brief Static utility class for evaluating and persisting the Max Flow assignment results.
 */
class AssignmentLogic {
public:
    /**
     * @brief Computes the total number of required reviews across all submissions.
     * @param submissions List of all submissions.
     * @param params Global parameters (contains required reviews per submission).
     * @return The total number of required reviews.
     * @complexity O(N) where N is the number of submissions.
     */
    static int totalRequiredReviews(const std::vector<Submission>& submissions, const Parameters& params);
    
    /**
     * @brief Computes the actual total number of assigned reviews from the flow graph.
     * @param flowGraph The computed max flow graph containing assignment edges.
     * @param submissions List of all submissions.
     * @param reviewers List of all reviewers.
     * @return The number of assigned reviews.
     * @complexity O(P * R) iterating over all submission-to-reviewer edges in the flow graph.
     */
    static int totalAssignedReviews(const Graph<int>& flowGraph, const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers);
    
    /**
     * @brief Identifies risky reviewers under K=1 logic.
     * @param submissions List of all submissions.
     * @param reviewers List of all reviewers.
     * @param params Global parameters.
     * @param mode Mode of evaluation.
     * @param algo Which max-flow algorithm to use (default: Ford-Fulkerson).
     * @return Vector of risky reviewer IDs.
     * @complexity For Ford-Fulkerson: O(R * F * E).
     *             For Edmonds-Karp: O(R * V * E^2).
     *             Where R is reviewers, F = P * MinReviewsPerSubmission is the total flow.
     */
    static std::vector<int> findRiskyReviewersK1(const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers, const Parameters& params, int mode, AlgorithmType algo = AlgorithmType::FordFulkerson);
    
    /**
     * @brief Writes the final review assignments to a CSV file.
     * @param flowGraph The final max flow graph.
     * @param submissions Vector of submissions.
     * @param reviewers Vector of reviewers.
     * @param params The global parameters.
     * @param mode Selected assignment mode.
     * @param riskyReviewerIds Pointer to vector of risky reviewer IDs.
     * @param riskValue Calculated risk value.
     * @param outputPath The path where the CSV is written.
     * @return True if write was successful, false otherwise.
     * @complexity O(V + E) for traversing the flow graph.
     */
    static bool writeAssignmentsToFile(const Graph<int>& flowGraph, const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers, const Parameters& params, int mode, const std::vector<int>* riskyReviewerIds = nullptr, int riskValue = 0, const std::string& outputPath = "");
    
    /**
     * @brief Generates reviewer assignments based on max flow and optionally stores them.
     * @param outGraph Output graph populated with the final flows.
     * @param submissions List of all submissions.
     * @param reviewers List of all reviewers.
     * @param params Global parameters defining constraints.
     * @param mode Operation mode mapping (e.g. strict vs relaxed constraints).
     * @param writeOutput Indicates whether to dump the result to a CSV file.
     * @param logger Optional logging callback.
     * @param algo Which max-flow algorithm to use (default: Ford-Fulkerson).
     * @return True if assignment was successful/complete.
     * @complexity For Ford-Fulkerson: O(F * E).
     *             For Edmonds-Karp: O(V * E^2).
     *             Dominated by the underlying max-flow call.
     */
    static bool generateAssignmentsAndStore(Graph<int>& outGraph, const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers, const Parameters& params, int mode, bool writeOutput, std::function<void(const std::vector<int>&, double)> logger = nullptr, AlgorithmType algo = AlgorithmType::FordFulkerson);
};

#endif
