#include "Benchmark.h"
#include "LogParser.h"
#include "LogReader.h"
#include "ThreadPool.h"
#include <chrono>
#include <future>
#include <fstream>
#include <stdexcept>
#include <thread>

namespace logpulse {

ReadResult Benchmark::analyzeSequential(const std::vector<std::string>& paths) {
    LogReader reader;
    return reader.readFiles(paths);
}

ReadResult Benchmark::analyzeParallel(const std::vector<std::string>& paths, std::size_t workerThreads) {
    if (workerThreads == 0) workerThreads = 1;
    ThreadPool pool(workerThreads);
    std::vector<std::future<ReadResult>> futures;
    constexpr std::size_t chunkSize = 10000;

    // Each worker receives a bounded chunk. The input file is never loaded completely into memory.
    for (const auto& path : paths) {
        std::ifstream input(path);
        if (!input) throw std::runtime_error("Unable to open log file: " + path);
        std::vector<std::string> chunk;
        chunk.reserve(chunkSize);
        std::size_t firstLineNumber = 1;
        std::string line;
        while (std::getline(input, line)) {
            chunk.push_back(std::move(line));
            if (chunk.size() == chunkSize) {
                auto work = std::move(chunk);
                const auto startLine = firstLineNumber;
                futures.push_back(pool.enqueue([path, work = std::move(work), startLine]() mutable {
                    ReadResult result;
                    result.totalLines = work.size();
                    result.entries.reserve(work.size());
                    for (std::size_t i = 0; i < work.size(); ++i) {
                        auto parsed = LogParser::parse(work[i], startLine + i, path);
                        if (parsed.second) result.entries.push_back(std::move(parsed.first));
                        else ++result.invalidEntries;
                    }
                    return result;
                }));
                firstLineNumber += chunkSize;
                chunk.clear();
                chunk.reserve(chunkSize);
            }
        }
        if (!chunk.empty()) {
            auto work = std::move(chunk);
            const auto startLine = firstLineNumber;
            futures.push_back(pool.enqueue([path, work = std::move(work), startLine]() mutable {
                ReadResult result;
                result.totalLines = work.size();
                result.entries.reserve(work.size());
                for (std::size_t i = 0; i < work.size(); ++i) {
                    auto parsed = LogParser::parse(work[i], startLine + i, path);
                    if (parsed.second) result.entries.push_back(std::move(parsed.first));
                    else ++result.invalidEntries;
                }
                return result;
            }));
        }
    }

    ReadResult combined;
    for (auto& future : futures) {
        auto current = future.get();
        combined.totalLines += current.totalLines;
        combined.invalidEntries += current.invalidEntries;
        combined.entries.insert(combined.entries.end(),
                                std::make_move_iterator(current.entries.begin()),
                                std::make_move_iterator(current.entries.end()));
    }
    pool.shutdown();
    return combined;
}

BenchmarkResult Benchmark::compare(const std::vector<std::string>& paths, std::size_t workerThreads) {
    using clock = std::chrono::steady_clock;
    const auto seqStart = clock::now();
    auto sequential = analyzeSequential(paths);
    const auto seqEnd = clock::now();

    const auto parStart = clock::now();
    auto parallel = analyzeParallel(paths, workerThreads);
    const auto parEnd = clock::now();

    // Keep the benchmark honest: both implementations process the same input and are checked for identical results.
    if (sequential.totalLines != parallel.totalLines || sequential.invalidEntries != parallel.invalidEntries ||
        sequential.entries.size() != parallel.entries.size()) {
        throw std::runtime_error("Sequential and parallel results differ; benchmark aborted");
    }

    BenchmarkResult result;
    result.sequentialMs = std::chrono::duration<double, std::milli>(seqEnd - seqStart).count();
    result.parallelMs = std::chrono::duration<double, std::milli>(parEnd - parStart).count();
    result.threads = workerThreads == 0 ? 1 : workerThreads;
    result.entries = sequential.entries.size();
    return result;
}

} // namespace logpulse
