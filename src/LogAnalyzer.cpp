#include "LogAnalyzer.h"
#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace logpulse {

std::size_t LogAnalyzer::levelIndex(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::DEBUG: return 0;
    case LogLevel::INFO: return 1;
    case LogLevel::WARNING: return 2;
    case LogLevel::ERROR: return 3;
    case LogLevel::CRITICAL: return 4;
    default: return 0;
    }
}

AnalysisResult LogAnalyzer::analyze(const std::vector<LogEntry>& entries, std::size_t invalidEntries) const {
    AnalysisResult result;
    result.totalEntries = entries.size() + invalidEntries;
    result.validEntries = entries.size();
    result.invalidEntries = invalidEntries;
    for (const auto& entry : entries) {
        if (entry.level != LogLevel::UNKNOWN) ++result.levelCounts[levelIndex(entry.level)];
    }
    result.frequentErrors = topErrors(entries);
    return result;
}

AnalysisResult LogAnalyzer::analyzeReadResult(const ReadResult& readResult) const {
    return analyze(readResult.entries, readResult.invalidEntries);
}

static std::string lowerCopy(const std::string& input) {
    std::string result = input;
    for (char& c : result) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return result;
}

std::vector<LogEntry> LogAnalyzer::searchKeyword(const std::vector<LogEntry>& entries, const std::string& keyword,
                                                  bool caseInsensitive) const {
    if (keyword.empty()) return {};
    const std::string needle = caseInsensitive ? lowerCopy(keyword) : keyword;
    std::vector<LogEntry> matches;
    for (const auto& entry : entries) {
        const std::string haystack = caseInsensitive ? lowerCopy(entry.message) : entry.message;
        if (haystack.find(needle) != std::string::npos) matches.push_back(entry);
    }
    return matches;
}

std::vector<LogEntry> LogAnalyzer::filterBySeverity(const std::vector<LogEntry>& entries, LogLevel minimumLevel) const {
    const auto minIndex = levelIndex(minimumLevel);
    std::vector<LogEntry> matches;
    for (const auto& entry : entries) {
        if (entry.level != LogLevel::UNKNOWN && levelIndex(entry.level) >= minIndex) matches.push_back(entry);
    }
    return matches;
}

std::vector<LogEntry> LogAnalyzer::filterByTimestamp(const std::vector<LogEntry>& entries,
                                                      const std::string& from, const std::string& to) const {
    std::vector<LogEntry> matches;
    for (const auto& entry : entries) {
        if (entry.timestamp >= from && entry.timestamp <= to) matches.push_back(entry);
    }
    return matches;
}

std::vector<std::pair<std::string, std::size_t>> LogAnalyzer::topErrors(const std::vector<LogEntry>& entries, std::size_t limit) {
    std::unordered_map<std::string, std::size_t> counts;
    for (const auto& entry : entries) {
        if (entry.level == LogLevel::ERROR || entry.level == LogLevel::CRITICAL) ++counts[entry.message];
    }
    std::vector<std::pair<std::string, std::size_t>> result(counts.begin(), counts.end());
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });
    if (result.size() > limit) result.resize(limit);
    return result;
}

} // namespace logpulse
