#include "LogParser.h"
#include <cassert>

using namespace logpulse;

int main() {
    auto valid = LogParser::parse("2026-11-18 10:20:30 [ERROR] Database connection failed", 7, "sample.log");
    assert(valid.second);
    assert(valid.first.level == LogLevel::ERROR);
    assert(valid.first.message == "Database connection failed");
    assert(valid.first.lineNumber == 7);
    assert(!LogParser::parse("not a log", 1).second);
    assert(!LogParser::parse("2026-99-99 99:99:99 [INFO] bad timestamp", 2).second);
    assert(!LogParser::parse("2026-11-18 10:20:30 [NOPE] unknown", 3).second);
    assert(LogParser::isTimestampValid("2026-11-18 10:20:30"));
    assert(!LogParser::isTimestampValid("2026/11/18 10:20:30"));
    return 0;
}
