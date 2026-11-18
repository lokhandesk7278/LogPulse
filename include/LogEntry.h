#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace logpulse {

enum class LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL, UNKNOWN };

std::string_view toString(LogLevel level) noexcept;
LogLevel logLevelFromString(std::string_view value) noexcept;

struct LogEntry {
    std::string timestamp;
    LogLevel level{LogLevel::UNKNOWN};
    std::string message;
    std::string source;
    std::size_t lineNumber{0};

    bool isValid() const noexcept { return level != LogLevel::UNKNOWN && !timestamp.empty(); }
};

} // namespace logpulse
