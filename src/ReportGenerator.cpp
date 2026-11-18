#include "ReportGenerator.h"
#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace logpulse {

double BenchmarkResult::sequentialThroughput() const noexcept {
    return sequentialMs > 0.0 ? static_cast<double>(entries) / (sequentialMs / 1000.0) : 0.0;
}

double BenchmarkResult::parallelThroughput() const noexcept {
    return parallelMs > 0.0 ? static_cast<double>(entries) / (parallelMs / 1000.0) : 0.0;
}

static void ensureOpen(const std::ofstream& output, const std::string& path) {
    if (!output) throw std::runtime_error("Unable to create report: " + path);
}

void ReportGenerator::writeTxt(const std::string& path, const AnalysisResult& result,
                               const BenchmarkResult* benchmark) {
    std::ofstream out(path);
    ensureOpen(out, path);
    out << "LogPulse Analysis Report\n========================\n";
    out << "Total log entries: " << result.totalEntries << '\n';
    out << "Valid entries: " << result.validEntries << '\n';
    out << "Invalid entries: " << result.invalidEntries << '\n';
    const char* names[] = {"DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};
    for (std::size_t i = 0; i < 5; ++i) out << names[i] << ": " << result.levelCounts[i] << '\n';
    out << "\nMost frequent error messages\n----------------------------\n";
    for (const auto& [message, count] : result.frequentErrors) out << count << " | " << message << '\n';
    if (benchmark) {
        out << "\nPerformance\n-----------\n";
        out << std::fixed << std::setprecision(3);
        out << "Worker threads: " << benchmark->threads << '\n';
        out << "Sequential time (ms): " << benchmark->sequentialMs << '\n';
        out << "Parallel time (ms): " << benchmark->parallelMs << '\n';
        out << "Speedup: " << benchmark->speedup() << "x\n";
        out << "Sequential throughput (entries/s): " << benchmark->sequentialThroughput() << '\n';
        out << "Parallel throughput (entries/s): " << benchmark->parallelThroughput() << '\n';
    }
}

void ReportGenerator::writeCsv(const std::string& path, const AnalysisResult& result,
                               const BenchmarkResult* benchmark) {
    std::ofstream out(path);
    ensureOpen(out, path);
    out << "metric,value\n";
    out << "total_entries," << result.totalEntries << '\n';
    out << "valid_entries," << result.validEntries << '\n';
    out << "invalid_entries," << result.invalidEntries << '\n';
    out << "debug," << result.levelCounts[0] << '\n';
    out << "info," << result.levelCounts[1] << '\n';
    out << "warning," << result.levelCounts[2] << '\n';
    out << "error," << result.levelCounts[3] << '\n';
    out << "critical," << result.levelCounts[4] << '\n';
    if (benchmark) {
        out << std::fixed << std::setprecision(6);
        out << "worker_threads," << benchmark->threads << '\n';
        out << "sequential_ms," << benchmark->sequentialMs << '\n';
        out << "parallel_ms," << benchmark->parallelMs << '\n';
        out << "speedup," << benchmark->speedup() << '\n';
        out << "sequential_entries_per_second," << benchmark->sequentialThroughput() << '\n';
        out << "parallel_entries_per_second," << benchmark->parallelThroughput() << '\n';
    }
    out << "\nerror_message,count\n";
    for (const auto& [message, count] : result.frequentErrors) {
        std::string escaped = message;
        std::string safe;
        safe.reserve(escaped.size());
        for (char c : escaped) { if (c == '"') safe += "\"\""; else safe += c; }
        out << '"' << safe << "\"," << count << '\n';
    }
}

} // namespace logpulse
