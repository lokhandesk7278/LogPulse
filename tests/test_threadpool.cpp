#include "ThreadPool.h"
#include <atomic>
#include <cassert>
#include <vector>

using namespace logpulse;

int main() {
    ThreadPool pool(4);
    std::atomic<int> counter{0};
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 100; ++i) {
        futures.push_back(pool.enqueue([&counter, i]() { counter.fetch_add(1, std::memory_order_relaxed); return i * i; }));
    }
    long long sum = 0;
    for (auto& f : futures) sum += f.get();
    assert(counter.load() == 100);
    assert(sum == 328350);
    pool.shutdown();
    return 0;
}
