#ifndef MAXFLOWPROJECT_PARAMETERS_H
#define MAXFLOWPROJECT_PARAMETERS_H

#include <string>

/**
 * @struct Parameters
 * @brief Struct to hold global configuration parameters.
 * 
 * Contains all parameters extracted from the Control and Parameters sections
 * of the input CSV dataset, dictating assignment logic, capacities, risk analysis mode,
 * and the expected output file name.
 */
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
