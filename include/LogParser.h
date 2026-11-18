#pragma once

#include "LogEntry.h"
#include <string>
#include <string_view>
#include <utility>

namespace logpulse {

class LogParser {
public:
    // Expected format: YYYY-MM-DD HH:MM:SS [LEVEL] message
    static std::pair<LogEntry, bool> parse(std::string_view line, std::size_t lineNumber = 0,
                                           std::string source = {});
    static bool isTimestampValid(std::string_view timestamp) noexcept;
};

} // namespace logpulse
