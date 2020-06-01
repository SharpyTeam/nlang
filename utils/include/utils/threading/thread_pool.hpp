#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <future>
#include <type_traits>
#include <utility>

namespace nlang {

/**
 * Thread pool.
 * Execute tasks (functions) using workers, returning std::future
 */
class ThreadPool {
public:
    ThreadPool(size_t threads = 0);

    template<typename F, class ...Args>
    std::future<std::result_of_t<F(Args...)>> Enqueue(F&& f, Args&&... args) {
        auto task = std::make_shared<std::packaged_task<std::result_of_t<F(Args...)>()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        auto future = task->get_future();

        {
            std::lock_guard guard(mutex);
            if (shutdown) {
                throw std::runtime_error("can't enqueue on stopped pool");
            }

            tasks.emplace([task]() { (*task)(); });
        }

        condition.notify_one();

        return future;
    }

    ~ThreadPool();

private:
    std::vector<std::thread> workers;
    std::mutex mutex;
    std::condition_variable condition;

    std::queue<std::function<void()>> tasks;

    bool shutdown;
};

}
