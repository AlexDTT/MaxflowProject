/**
 * @file AssignmentLogic.h
 * @brief Logic for assigning reviewers to submissions based on Max Flow.
 */

#ifndef ASSIGNMENTLOGIC_H
#define ASSIGNMENTLOGIC_H

#include <vector>
#include <string>
#include <functional>
#include "../data_structures/Graph.h"
#include "Submission.h"
#include "Reviewer.h"
#include "Parameters.h"

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
     * @complexity O(V + E) representing graph traversal over flow paths.
     */
    static int totalAssignedReviews(const Graph<int>& flowGraph, const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers);
    
    /**
     * @brief Identifies risky reviewers under K=1 logic.
     * @param submissions List of all submissions.
     * @param reviewers List of all reviewers.
     * @param params Global parameters.
     * @param mode Mode of evaluation.
     * @return Vector of risky reviewer IDs.
     * @complexity O(P * R) where P is submissions and R is reviewers.
     */
    static std::vector<int> findRiskyReviewersK1(const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers, const Parameters& params, int mode);
    
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
     * @return True if assignment was successful/complete.
     * @complexity Depends on the underlying Max Flow algorithm, commonly O(V * E^2).
     */
    static bool generateAssignmentsAndStore(Graph<int>& outGraph, const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers, const Parameters& params, int mode, bool writeOutput, std::function<void(const std::vector<int>&, double)> logger = nullptr);
};

#endif
