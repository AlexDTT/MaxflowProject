#ifndef MAXFLOWPROJECT_FILEPARSER_H
#define MAXFLOWPROJECT_FILEPARSER_H

#include <string>
#include <vector>
#include "Submission.h"
#include "Reviewer.h"
#include "Parameters.h"

class FileParser {
public:
    /**
     * Parses a CSV input file and populates submissions, reviewers and params.
     * Returns true on success. On failure prints error messages to stderr.
     */
    static bool parse(const std::string& filename,
                      std::vector<Submission>& submissions,
                      std::vector<Reviewer>&   reviewers,
                      Parameters&              params);
};

#endif // MAXFLOWPROJECT_FILEPARSER_H
