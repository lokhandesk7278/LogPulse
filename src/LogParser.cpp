#include "LogParser.h"
#include <cctype>
#include <regex>

namespace logpulse {

std::string_view toString(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::DEBUG: return "DEBUG";
    case LogLevel::INFO: return "INFO";
    case LogLevel::WARNING: return "WARNING";
    case LogLevel::ERROR: return "ERROR";
    case LogLevel::CRITICAL: return "CRITICAL";
    default: return "UNKNOWN";
    }
}

LogLevel logLevelFromString(std::string_view value) noexcept {
    if (value == "DEBUG") return LogLevel::DEBUG;
    if (value == "INFO") return LogLevel::INFO;
    if (value == "WARNING" || value == "WARN") return LogLevel::WARNING;
    if (value == "ERROR") return LogLevel::ERROR;
    if (value == "CRITICAL" || value == "FATAL") return LogLevel::CRITICAL;
    return LogLevel::UNKNOWN;
}

bool LogParser::isTimestampValid(std::string_view ts) noexcept {
    if (ts.size() != 19 || ts[4] != '-' || ts[7] != '-' || ts[10] != ' ' || ts[13] != ':' || ts[16] != ':') return false;
    for (std::size_t i = 0; i < ts.size(); ++i) {
        if (i == 4 || i == 7 || i == 10 || i == 13 || i == 16) continue;
        if (!std::isdigit(static_cast<unsigned char>(ts[i]))) return false;
    }
    int month = std::stoi(std::string(ts.substr(5, 2)));
    int day = std::stoi(std::string(ts.substr(8, 2)));
    int hour = std::stoi(std::string(ts.substr(11, 2)));
    int minute = std::stoi(std::string(ts.substr(14, 2)));
    int second = std::stoi(std::string(ts.substr(17, 2)));
    return month >= 1 && month <= 12 && day >= 1 && day <= 31 && hour <= 23 && minute <= 59 && second <= 59;
}

std::pair<LogEntry, bool> LogParser::parse(std::string_view line, std::size_t lineNumber, std::string source) {
    LogEntry entry;
    entry.lineNumber = lineNumber;
    entry.source = std::move(source);
    if (line.size() < 24 || !isTimestampValid(line.substr(0, 19)) || line[19] != ' ' || line[20] != '[') return {entry, false};
    const auto close = line.find(']', 21);
    if (close == std::string_view::npos || close <= 21) return {entry, false};
    const auto level = logLevelFromString(line.substr(21, close - 21));
    if (level == LogLevel::UNKNOWN) return {entry, false};
    if (close + 1 >= line.size() || line[close + 1] != ' ') return {entry, false};
    entry.timestamp = std::string(line.substr(0, 19));
    entry.level = level;
    entry.message = std::string(line.substr(close + 2));
    if (entry.message.empty()) return {LogEntry{}, false};
    entry.lineNumber = lineNumber;
    return {entry, true};
}

} // namespace logpulse
