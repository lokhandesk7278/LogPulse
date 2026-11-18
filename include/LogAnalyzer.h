#pragma once

#include "LogEntry.h"
#include "LogReader.h"
#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace logpulse {

struct AnalysisResult {
    std::size_t totalEntries{0};
    std::size_t validEntries{0};
    std::size_t invalidEntries{0};
    std::array<std::size_t, 5> levelCounts{}; // DEBUG, INFO, WARNING, ERROR, CRITICAL
    std::vector<std::pair<std::string, std::size_t>> frequentErrors;
};

class LogAnalyzer {
public:
    AnalysisResult analyze(const std::vector<LogEntry>& entries, std::size_t invalidEntries = 0) const;
    AnalysisResult analyzeReadResult(const ReadResult& result) const;

    std::vector<LogEntry> searchKeyword(const std::vector<LogEntry>& entries, const std::string& keyword,
                                        bool caseInsensitive = true) const;
    std::vector<LogEntry> filterBySeverity(const std::vector<LogEntry>& entries, LogLevel minimumLevel) const;
    std::vector<LogEntry> filterByTimestamp(const std::vector<LogEntry>& entries,
                                            const std::string& from, const std::string& to) const;

    static std::size_t levelIndex(LogLevel level) noexcept;
    static std::vector<std::pair<std::string, std::size_t>> topErrors(
        const std::vector<LogEntry>& entries, std::size_t limit = 10);
};

} // namespace logpulse
