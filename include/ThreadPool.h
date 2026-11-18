#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <stdexcept>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

namespace logpulse {

class ThreadPool {
public:
    explicit ThreadPool(std::size_t threadCount = std::thread::hardware_concurrency());
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template <typename F>
    auto enqueue(F&& task) -> std::future<typename std::invoke_result_t<F>> {
        using ReturnType = typename std::invoke_result_t<F>;
        auto packaged = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<F>(task));
        std::future<ReturnType> future = packaged->get_future();
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if (stopping_) {
                throw std::runtime_error("Cannot enqueue work on a stopped ThreadPool");
            }
            tasks_.emplace([packaged]() { (*packaged)(); });
        }
        condition_.notify_one();
        return future;
    }

    std::size_t size() const noexcept { return workers_.size(); }
    void shutdown();

private:
    void workerLoop();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queueMutex_;
    std::condition_variable condition_;
    bool stopping_{false};
};

} // namespace logpulse
