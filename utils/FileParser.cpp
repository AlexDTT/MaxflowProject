#include "FileParser.h"
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>

namespace {

std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Strip everything from an unquoted '#' to the end of the line.
std::string stripInlineComment(const std::string& line) {
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '"')            inQuotes = !inQuotes;
        else if (line[i] == '#' && !inQuotes) return line.substr(0, i);
    }
    return line;
}

// Split a CSV line; double-quoted fields may contain commas.
std::vector<std::string> splitCSV(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    for (char c : line) {
        if (c == '"')                    inQuotes = !inQuotes;
        else if (c == ',' && !inQuotes) { fields.push_back(trim(field)); field.clear(); }
        else                              field += c;
    }
    fields.push_back(trim(field));
    return fields;
}

enum class Section { NONE, SUBMISSIONS, REVIEWERS, PARAMETERS, CONTROL };

Section detectSection(const std::string& tag) {
    if (tag.find("Submissions") != std::string::npos) return Section::SUBMISSIONS;
    if (tag.find("Reviewers")   != std::string::npos) return Section::REVIEWERS;
    if (tag.find("Parameters")  != std::string::npos) return Section::PARAMETERS;
    if (tag.find("Control")     != std::string::npos) return Section::CONTROL;
    return Section::NONE;
}

} // anonymous namespace

bool FileParser::parse(const std::string& filename,
                       std::vector<Submission>& submissions,
                       std::vector<Reviewer>&   reviewers,
                       Parameters&              params) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open '" << filename << "'.\n";
        return false;
    }

    Section current = Section::NONE;
    std::set<int> subIds, revIds;
    std::vector<std::string> errors;
    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        ++lineNum;
        line = trim(line);
        if (line.empty()) continue;

        // Lines starting with '#' are section markers or column-header comments.
        if (line[0] == '#') {
            std::string tag = trim(line.substr(1));
            if (tag.empty()) {
                current = Section::NONE;
            } else {
                Section s = detectSection(tag);
                if (s != Section::NONE) current = s;
                // else: column header comment — skip
            }
            continue;
        }

        // Strip inline comment and skip blank result.
        std::string data = trim(stripInlineComment(line));
        if (data.empty()) continue;

        auto fields = splitCSV(data);

        if (current == Section::SUBMISSIONS) {
            if (fields.size() < 5) {
                errors.push_back("Line " + std::to_string(lineNum) +
                                 ": submission needs at least 5 fields");
                continue;
            }
            try {
                int id = std::stoi(fields[0]);
                if (!subIds.insert(id).second) {
                    errors.push_back("Line " + std::to_string(lineNum) +
                                     ": duplicate submission ID " + std::to_string(id));
                    continue;
                }
                int primary   = std::stoi(fields[4]);
                int secondary = (fields.size() > 5 && !fields[5].empty())
                                    ? std::stoi(fields[5]) : -1;
                submissions.emplace_back(id, fields[1], fields[2], fields[3],
                                         primary, secondary);
            } catch (const std::exception& e) {
                errors.push_back("Line " + std::to_string(lineNum) +
                                 ": invalid submission data (" + e.what() + ")");
            }

        } else if (current == Section::REVIEWERS) {
            if (fields.size() < 4) {
                errors.push_back("Line " + std::to_string(lineNum) +
                                 ": reviewer needs at least 4 fields");
                continue;
            }
            try {
                int id = std::stoi(fields[0]);
                if (!revIds.insert(id).second) {
                    errors.push_back("Line " + std::to_string(lineNum) +
                                     ": duplicate reviewer ID " + std::to_string(id));
                    continue;
                }
                int primary   = std::stoi(fields[3]);
                int secondary = (fields.size() > 4 && !fields[4].empty())
                                    ? std::stoi(fields[4]) : -1;
                reviewers.emplace_back(id, fields[1], fields[2], primary, secondary);
            } catch (const std::exception& e) {
                errors.push_back("Line " + std::to_string(lineNum) +
                                 ": invalid reviewer data (" + e.what() + ")");
            }

        } else if (current == Section::PARAMETERS || current == Section::CONTROL) {
            if (fields.size() < 2) continue;
            const std::string& key = fields[0];
            std::string val        = fields[1];
            // Strip surrounding quotes from string values.
            if (val.size() >= 2 && val.front() == '"' && val.back() == '"')
                val = val.substr(1, val.size() - 2);
            try {
                if      (key == "MinReviewsPerSubmission")    params.MinReviewsPerSubmission    = std::stoi(val);
                else if (key == "MaxReviewsPerReviewer")      params.MaxReviewsPerReviewer      = std::stoi(val);
                else if (key == "PrimaryReviewerExpertise")   params.PrimaryReviewerExpertise   = std::stoi(val);
                else if (key == "SecondaryReviewerExpertise") params.SecondaryReviewerExpertise = std::stoi(val);
                else if (key == "PrimarySubmissionDomain")    params.PrimarySubmissionDomain    = std::stoi(val);
                else if (key == "SecondarySubmissionDomain")  params.SecondarySubmissionDomain  = std::stoi(val);
                else if (key == "GenerateAssignments")        params.GenerateAssignments        = std::stoi(val);
                else if (key == "RiskAnalysis")               params.RiskAnalysis               = std::stoi(val);
                else if (key == "OutputFileName")             params.OutputFileName             = val;
            } catch (const std::exception& e) {
                errors.push_back("Line " + std::to_string(lineNum) +
                                 ": invalid value for '" + key + "' (" + e.what() + ")");
            }
        }
    }

    if (!errors.empty()) {
        std::cerr << "Parse errors in '" << filename << "':\n";
        for (const auto& err : errors) std::cerr << "  " << err << "\n";
        return false;
    }
    return true;
}
