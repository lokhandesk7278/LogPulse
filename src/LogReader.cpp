#include "LogReader.h"
#include "LogParser.h"
#include <fstream>
#include <stdexcept>

namespace logpulse {

ReadResult LogReader::readFile(const std::string& path) const {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Unable to open log file: " + path);
    ReadResult result;
    std::string line;
    while (std::getline(input, line)) {
        ++result.totalLines;
        auto parsed = LogParser::parse(line, result.totalLines, path);
        if (parsed.second) result.entries.push_back(std::move(parsed.first));
        else ++result.invalidEntries;
    }
    return result;
}

ReadResult LogReader::readFiles(const std::vector<std::string>& paths) const {
    if (paths.empty()) throw std::invalid_argument("At least one log file is required");
    ReadResult combined;
    for (const auto& path : paths) {
        auto current = readFile(path);
        combined.totalLines += current.totalLines;
        combined.invalidEntries += current.invalidEntries;
        combined.entries.insert(combined.entries.end(),
                                std::make_move_iterator(current.entries.begin()),
                                std::make_move_iterator(current.entries.end()));
    }
    return combined;
}

std::vector<std::string> LogReader::readLines(const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Unable to open log file: " + path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) lines.push_back(std::move(line));
    return lines;
}

} // namespace logpulse
