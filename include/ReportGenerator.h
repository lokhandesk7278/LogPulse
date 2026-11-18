#pragma once

#include "LogAnalyzer.h"
#include <string>

namespace logpulse {

struct BenchmarkResult {
    double sequentialMs{0.0};
    double parallelMs{0.0};
    std::size_t threads{1};
    std::size_t entries{0};

    double speedup() const noexcept { return parallelMs > 0.0 ? sequentialMs / parallelMs : 0.0; }
    double sequentialThroughput() const noexcept;
    double parallelThroughput() const noexcept;
};

class ReportGenerator {
public:
    static void writeTxt(const std::string& path, const AnalysisResult& result,
                         const BenchmarkResult* benchmark = nullptr);
    static void writeCsv(const std::string& path, const AnalysisResult& result,
                         const BenchmarkResult* benchmark = nullptr);
};

} // namespace logpulse
