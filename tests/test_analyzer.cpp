#include "LogAnalyzer.h"
#include "LogParser.h"
#include <cassert>
#include <vector>

using namespace logpulse;

static LogEntry make(const char* ts, LogLevel level, const char* msg) {
    LogEntry e;
    e.timestamp = ts; e.level = level; e.message = msg;
    return e;
}

int main() {
    std::vector<LogEntry> entries{
        make("2026-11-18 10:00:00", LogLevel::INFO, "startup complete"),
        make("2026-11-18 10:00:01", LogLevel::ERROR, "database failed"),
        make("2026-11-18 10:00:02", LogLevel::ERROR, "database failed"),
        make("2026-11-18 10:00:03", LogLevel::CRITICAL, "database failed"),
        make("2026-11-18 11:00:00", LogLevel::WARNING, "high latency")
    };
    LogAnalyzer analyzer;
    auto result = analyzer.analyze(entries, 2);
    assert(result.totalEntries == 7 && result.validEntries == 5 && result.invalidEntries == 2);
    assert(result.levelCounts[1] == 1 && result.levelCounts[2] == 1);
    assert(result.levelCounts[3] == 2 && result.levelCounts[4] == 1);
    assert(result.frequentErrors.front().first == "database failed");
    assert(result.frequentErrors.front().second == 3);
    assert(analyzer.searchKeyword(entries, "DATABASE").size() == 3);
    assert(analyzer.filterBySeverity(entries, LogLevel::ERROR).size() == 3);
    assert(analyzer.filterByTimestamp(entries, "2026-11-18 10:00:00", "2026-11-18 10:00:03").size() == 4);
    return 0;
}
