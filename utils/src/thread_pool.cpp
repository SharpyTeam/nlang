#include <utils/threading/thread_pool.hpp>

namespace nlang {

ThreadPool::ThreadPool(size_t threads)
    : shutdown(false)
{
    workers.resize(threads);
    for (auto& worker : workers) {
        worker = std::thread([this]() {
            while (true) {
                std::unique_lock lock(mutex);
                condition.wait(lock, [this]() { return shutdown || !tasks.empty(); });
                if (shutdown && tasks.empty()) {
                    break;
                }
                std::function<void()> task(std::move(tasks.front()));
                tasks.pop();
                lock.unlock();
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard guard(mutex);
        shutdown = true;
    }

    condition.notify_all();
    for (auto& t : workers) {
        t.join();
    }
}

}
