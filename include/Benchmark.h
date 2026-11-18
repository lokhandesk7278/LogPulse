#pragma once

#include "LogAnalyzer.h"
#include "ReportGenerator.h"
#include <cstddef>
#include <string>
#include <vector>

namespace logpulse {

class Benchmark {
public:
    static BenchmarkResult compare(const std::vector<std::string>& paths, std::size_t workerThreads);

private:
    static ReadResult analyzeSequential(const std::vector<std::string>& paths);
    static ReadResult analyzeParallel(const std::vector<std::string>& paths, std::size_t workerThreads);
};

} // namespace logpulse
