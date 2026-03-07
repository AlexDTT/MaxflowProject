#ifndef MAXFLOWPROJECT_PARAMETERS_H
#define MAXFLOWPROJECT_PARAMETERS_H

#include <string>

struct Parameters {
    int MinReviewsPerSubmission    = 1;
    int MaxReviewsPerReviewer      = 1;
    int PrimaryReviewerExpertise   = 1;
    int SecondaryReviewerExpertise = 0;
    int PrimarySubmissionDomain    = 1;
    int SecondarySubmissionDomain  = 0;
    int GenerateAssignments        = 0;
    int RiskAnalysis               = 0;
    std::string OutputFileName     = "output.csv";
};

#endif // MAXFLOWPROJECT_PARAMETERS_H
