#include "Reviewer.h"
#include <iostream>

Reviewer::Reviewer(int id, const std::string& name, const std::string& email,
                   int primaryTopic, int secondaryTopic)
    : id(id), name(name), email(email),
      primaryTopic(primaryTopic), secondaryTopic(secondaryTopic) {}

void Reviewer::print() const {
    std::cout << "  ID: " << id
              << " | Name: " << name
              << " | E-mail: " << email
              << " | Primary Topic: " << primaryTopic;
    if (secondaryTopic != -1)
        std::cout << " | Secondary Topic: " << secondaryTopic;
    std::cout << "\n";
}