#include "Benchmark.h"
#include "LogAnalyzer.h"
#include "LogReader.h"
#include "ReportGenerator.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace logpulse;
namespace fs = std::filesystem;

int main() {
    const fs::path root = fs::temp_directory_path() / "logpulse_test_data";
    fs::create_directories(root);
    const auto file = root / "integration.log";
    {
        std::ofstream out(file);
        for (int i = 0; i < 5000; ++i) {
            out << "2026-11-18 12:00:00 [INFO] worker heartbeat " << i << '\n';
            if (i % 10 == 0) out << "bad entry " << i << '\n';
            if (i % 100 == 0) out << "2026-11-18 12:00:00 [ERROR] database timeout\n";
        }
    }
    LogReader reader;
    try {
        reader.readFile((root / "missing.log").string());
        assert(false && "missing file should throw");
    } catch (const std::runtime_error&) {
        // expected
    }
    auto rr = reader.readFile(file.string());
    assert(rr.totalLines == 5550);
    assert(rr.invalidEntries == 500);
    auto analysis = LogAnalyzer{}.analyzeReadResult(rr);
    assert(analysis.validEntries == 5000 + 50);
    assert(analysis.levelCounts[1] == 5000 && analysis.levelCounts[3] == 50);
    auto bench = Benchmark::compare({file.string()}, 4);
    assert(bench.entries == analysis.validEntries);
    const auto txt = (root / "report.txt").string();
    const auto csv = (root / "report.csv").string();
    ReportGenerator::writeTxt(txt, analysis, &bench);
    ReportGenerator::writeCsv(csv, analysis, &bench);
    assert(fs::exists(txt) && fs::exists(csv));
    fs::remove_all(root);
    return 0;
}
