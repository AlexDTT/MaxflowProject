/**
 * @file FileParser.h
 * @brief Class definition for parsing input CSV files.
 */

#ifndef MAXFLOWPROJECT_FILEPARSER_H
#define MAXFLOWPROJECT_FILEPARSER_H

#include <string>
#include <vector>
#include "Submission.h"
#include "Reviewer.h"
#include "Parameters.h"

/**
 * @class FileParser
 * @brief A utility class to parse configuration and data from a CSV file.
 * 
 * Provides static methods for extracting submissions, reviewers, and parameters
 * from the input dataset files used in the system.
 */
class FileParser {
public:
    /**
     * @brief Parses a CSV input file and populates submissions, reviewers and params.
     * 
     * @param filename The path to the input CSV file.
     * @param submissions A vector to populate with parsed submissions.
     * @param reviewers A vector to populate with parsed reviewers.
     * @param params A Parameters struct to populate with global settings.
     * @return true on success, false on failure (prints error messages to stderr).
     * @complexity O(L) where L is the number of lines in the CSV file.
     */
    static bool parse(const std::string& filename,
                      std::vector<Submission>& submissions,
                      std::vector<Reviewer>&   reviewers,
                      Parameters&              params);
};

#endif // MAXFLOWPROJECT_FILEPARSER_H
