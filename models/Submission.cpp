#include "models/Submission.h"
#include <iostream>

Submission::Submission(int id, const std::string& title, const std::string& authors,
                       const std::string& email, int primaryTopic, int secondaryTopic)
    : id(id), title(title), authors(authors), email(email),
      primaryTopic(primaryTopic), secondaryTopic(secondaryTopic) {}

void Submission::print() const {
    std::cout << "  ID: " << id
              << " | Title: " << title
              << " | Author(s): " << authors
              << " | E-mail: " << email
              << " | Primary Topic: " << primaryTopic;
    if (secondaryTopic != -1)
        std::cout << " | Secondary Topic: " << secondaryTopic;
    std::cout << "\n";
}