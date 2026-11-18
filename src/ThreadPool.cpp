#include "ThreadPool.h"
#include <algorithm>
#include <stdexcept>

namespace logpulse {

ThreadPool::ThreadPool(std::size_t threadCount) {
    if (threadCount == 0) threadCount = 1;
    workers_.reserve(threadCount);
    for (std::size_t i = 0; i < threadCount; ++i) workers_.emplace_back(&ThreadPool::workerLoop, this);
}

ThreadPool::~ThreadPool() { shutdown(); }

void ThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            condition_.wait(lock, [this] { return stopping_ || !tasks_.empty(); });
            if (stopping_ && tasks_.empty()) return;
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task();
    }
}

void ThreadPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        if (stopping_) return;
        stopping_ = true;
    }
    condition_.notify_all();
    for (auto& worker : workers_) if (worker.joinable()) worker.join();
}

} // namespace logpulse
