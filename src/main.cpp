#include "Benchmark.h"
#include "LogAnalyzer.h"
#include "LogParser.h"
#include "LogReader.h"
#include "ReportGenerator.h"
#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <thread>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace logpulse;

namespace {

class Application {
public:
    Application() : analyzer_(std::make_unique<LogAnalyzer>()) {}

    void run() {
        std::cout << "\n=== LogPulse - High-Performance Multithreaded Log Analyzer ===\n";
        while (true) {
            printMenu();
            const int choice = readInt("Select an option: ", 1, 10);
            try {
                switch (choice) {
                case 1: analyzeFiles(false); break;
                case 2: analyzeFiles(true); break;
                case 3: search(); break;
                case 4: filterSeverity(); break;
                case 5: statistics(); break;
                case 6: benchmarkSequential(); break;
                case 7: benchmarkParallel(); break;
                case 8: comparePerformance(); break;
                case 9: generateReports(); break;
                case 10: std::cout << "Goodbye!\n"; return;
                }
            } catch (const std::exception& ex) {
                std::cerr << "[ERROR] " << ex.what() << "\n";
            }
        }
    }

private:
    std::unique_ptr<LogAnalyzer> analyzer_;
    ReadResult current_;
    std::vector<std::string> currentPaths_;
    std::unique_ptr<BenchmarkResult> lastBenchmark_;

    static void printMenu() {
        std::cout << "\n1. Analyze a log file\n2. Analyze multiple log files\n3. Search logs by keyword\n"
                  << "4. Filter logs by severity\n5. Display log statistics\n6. Run sequential analysis\n"
                  << "7. Run multithreaded analysis\n8. Compare sequential and parallel performance\n"
                  << "9. Generate TXT/CSV reports\n10. Exit\n";
    }

    static std::string readLine(const std::string& prompt) {
        std::cout << prompt;
        std::string value;
        std::getline(std::cin, value);
        return value;
    }

    static int readInt(const std::string& prompt, int min, int max) {
        while (true) {
            const std::string text = readLine(prompt);
            try {
                std::size_t pos = 0;
                const int value = std::stoi(text, &pos);
                if (pos == text.size() && value >= min && value <= max) return value;
            } catch (...) {}
            std::cout << "Please enter a number from " << min << " to " << max << ".\n";
        }
    }

    static std::size_t readThreads() {
        while (true) {
            const std::string text = readLine("Worker threads (1-256, 0 = hardware concurrency): ");
            try {
                std::size_t pos = 0;
                const auto value = std::stoull(text, &pos);
                if (pos == text.size() && value <= 256) {
                    return value == 0 ? std::max(1u, std::thread::hardware_concurrency()) : static_cast<std::size_t>(value);
                }
            } catch (...) {}
            std::cout << "Invalid thread count.\n";
        }
    }

    std::vector<std::string> askPaths(bool multiple) const {
        std::vector<std::string> paths;
        if (!multiple) {
            paths.push_back(readLine("Log file path: "));
        } else {
            const int count = readInt("Number of log files (1-50): ", 1, 50);
            for (int i = 0; i < count; ++i) paths.push_back(readLine("Path " + std::to_string(i + 1) + ": "));
        }
        for (const auto& path : paths) if (!fs::exists(path)) throw std::runtime_error("File not found: " + path);
        return paths;
    }

    void load(const std::vector<std::string>& paths) {
        LogReader reader;
        current_ = reader.readFiles(paths);
        currentPaths_ = paths;
        lastBenchmark_.reset();
        std::cout << "Processed " << current_.totalLines << " lines: " << current_.entries.size()
                  << " valid, " << current_.invalidEntries << " invalid.\n";
    }

    void analyzeFiles(bool multiple) { load(askPaths(multiple)); }

    void ensureData() const {
        if (currentPaths_.empty()) throw std::runtime_error("Analyze a log file first.");
    }

    static void printEntries(const std::vector<LogEntry>& entries, std::size_t max = 20) {
        const auto count = std::min(max, entries.size());
        for (std::size_t i = 0; i < count; ++i) {
            std::cout << entries[i].timestamp << " [" << toString(entries[i].level) << "] " << entries[i].message << '\n';
        }
        if (entries.size() > max) std::cout << "... and " << entries.size() - max << " more.\n";
    }

    void search() {
        ensureData();
        const auto keyword = readLine("Keyword: ");
        const auto matches = analyzer_->searchKeyword(current_.entries, keyword);
        std::cout << "Matches: " << matches.size() << "\n";
        printEntries(matches);
    }

    void filterSeverity() {
        ensureData();
        std::cout << "0 DEBUG  1 INFO  2 WARNING  3 ERROR  4 CRITICAL\n";
        const int value = readInt("Minimum severity: ", 0, 4);
        const auto level = static_cast<LogLevel>(value);
        const auto matches = analyzer_->filterBySeverity(current_.entries, level);
        std::cout << "Matches: " << matches.size() << "\n";
        printEntries(matches);
    }

    AnalysisResult stats() const {
        ensureData();
        return analyzer_->analyzeReadResult(current_);
    }

    void display(const AnalysisResult& result) const {
        const char* names[] = {"DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};
        std::cout << "\nTotal entries: " << result.totalEntries << "\nValid entries: " << result.validEntries
                  << "\nInvalid entries: " << result.invalidEntries << '\n';
        for (std::size_t i = 0; i < 5; ++i) std::cout << std::left << std::setw(10) << names[i] << result.levelCounts[i] << '\n';
        std::cout << "\nTop error messages:\n";
        for (const auto& [message, count] : result.frequentErrors) std::cout << count << " | " << message << '\n';
    }

    void statistics() { display(stats()); }

    void benchmarkSequential() {
        ensureData();
        const auto result = Benchmark::compare(currentPaths_, 1);
        std::cout << std::fixed << std::setprecision(3) << "Sequential processing: " << result.sequentialMs << " ms\n";
        lastBenchmark_ = std::make_unique<BenchmarkResult>(result);
    }

    void benchmarkParallel() {
        ensureData();
        const auto threads = readThreads();
        const auto result = Benchmark::compare(currentPaths_, threads);
        std::cout << std::fixed << std::setprecision(3) << "Parallel processing with " << threads << " threads: "
                  << result.parallelMs << " ms\nThroughput: " << result.parallelThroughput() << " entries/s\n";
        lastBenchmark_ = std::make_unique<BenchmarkResult>(result);
    }

    void comparePerformance() {
        ensureData();
        const auto threads = readThreads();
        const auto result = Benchmark::compare(currentPaths_, threads);
        std::cout << std::fixed << std::setprecision(3)
                  << "Sequential: " << result.sequentialMs << " ms (" << result.sequentialThroughput() << " entries/s)\n"
                  << "Parallel:   " << result.parallelMs << " ms (" << result.parallelThroughput() << " entries/s)\n"
                  << "Speedup:    " << result.speedup() << "x\n";
        lastBenchmark_ = std::make_unique<BenchmarkResult>(result);
    }

    void generateReports() {
        ensureData();
        const auto directory = readLine("Report directory [reports]: ");
        const fs::path dir = directory.empty() ? fs::path("reports") : fs::path(directory);
        fs::create_directories(dir);
        const auto result = stats();
        const auto txt = (dir / "logpulse_report.txt").string();
        const auto csv = (dir / "logpulse_report.csv").string();
        ReportGenerator::writeTxt(txt, result, lastBenchmark_.get());
        ReportGenerator::writeCsv(csv, result, lastBenchmark_.get());
        std::cout << "Reports written to:\n  " << txt << "\n  " << csv << '\n';
    }
};

} // namespace

int main() {
    try {
        Application app;
        app.run();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
}
