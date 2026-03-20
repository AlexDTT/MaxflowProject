//
// Created by alexd on 07/03/2026.
//

#ifndef MAXFLOWPROJECT_SUBMISSION_H
#define MAXFLOWPROJECT_SUBMISSION_H

#include <string>

/**
 * @class Submission
 * @brief Represents a scientific paper/article submission.
 * 
 * Contains all metadata relevant to a submission, including its unique identifier,
 * title, authors, contact information, and domain topics (primary and optional secondary).
 */
class Submission {
public:
    int id;
    std::string title;
    std::string authors;
    std::string email;
    int primaryTopic;
    int secondaryTopic; // -1 if not defined

    Submission(int id, const std::string& title, const std::string& authors,
               const std::string& email, int primaryTopic, int secondaryTopic = -1);

    void print() const;
};

#endif // MAXFLOWPROJECT_SUBMISSION_H