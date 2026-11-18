#pragma once

#include "LogEntry.h"
#include <cstddef>
#include <string>
#include <vector>

namespace logpulse {

struct ReadResult {
    std::vector<LogEntry> entries;
    std::size_t invalidEntries{0};
    std::size_t totalLines{0};
};

class LogReader {
public:
    ReadResult readFile(const std::string& path) const;
    ReadResult readFiles(const std::vector<std::string>& paths) const;

    // Chunked reader used by the parallel benchmark. The caller chooses the line range.
    static std::vector<std::string> readLines(const std::string& path);
};

} // namespace logpulse
