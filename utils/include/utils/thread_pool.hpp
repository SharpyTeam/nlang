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

class ThreadPool {
public:
    ThreadPool(size_t threads = 0)
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

    ~ThreadPool() {
        {
            std::lock_guard guard(mutex);
            shutdown = true;
        }

        condition.notify_all();
        for (auto& t : workers) {
            t.join();
        }
    }

private:
    std::vector<std::thread> workers;
    std::mutex mutex;
    std::condition_variable condition;

    std::queue<std::function<void()>> tasks;

    bool shutdown = false;
};

}
