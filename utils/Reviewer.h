//
// Created by alexd on 07/03/2026.
//

#ifndef MAXFLOWPROJECT_REVIEWER_H
#define MAXFLOWPROJECT_REVIEWER_H

#include <string>

class Reviewer {
public:
    int id;
    std::string name;
    std::string email;
    int primaryTopic;
    int secondaryTopic; // -1 if not defined

    Reviewer(int id, const std::string& name, const std::string& email,
             int primaryTopic, int secondaryTopic = -1);

    void print() const;
};

#endif // MAXFLOWPROJECT_REVIEWER_H