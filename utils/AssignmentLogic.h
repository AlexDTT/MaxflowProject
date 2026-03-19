#ifndef ASSIGNMENTLOGIC_H
#define ASSIGNMENTLOGIC_H

#include <vector>
#include <string>
#include <functional>
#include "../data_structures/Graph.h"
#include "Submission.h"
#include "Reviewer.h"
#include "Parameters.h"

class AssignmentLogic {
public:
    static int totalRequiredReviews(const std::vector<Submission>& submissions, const Parameters& params);
    
    static int totalAssignedReviews(const Graph<int>& flowGraph, const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers);
    
    static std::vector<int> findRiskyReviewersK1(const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers, const Parameters& params, int mode);
    
    static bool writeAssignmentsToFile(const Graph<int>& flowGraph, const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers, const Parameters& params, int mode, const std::vector<int>* riskyReviewerIds = nullptr, int riskValue = 0, const std::string& outputPath = "");
    
    static bool generateAssignmentsAndStore(Graph<int>& outGraph, const std::vector<Submission>& submissions, const std::vector<Reviewer>& reviewers, const Parameters& params, int mode, bool writeOutput, std::function<void(const std::vector<int>&, double)> logger = nullptr);
};

#endif
