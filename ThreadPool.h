#pragma once
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <future>
#include <iostream>

typedef std::function<void()> task_type;
typedef std::future<void> res_type;

class ThreadPool {
public:
    ThreadPool(int numThreads = std::thread::hardware_concurrency()) : m_work(true) {
        for (int i = 0; i < numThreads; ++i) {
            m_threads.emplace_back([this] { threadFunc(); });
        }
    }

    ~ThreadPool() {
        stop();
    }

    void start() {
        m_work = true;
    }

    void stop() {
        {
            std::unique_lock<std::mutex> lock(m_locker);
            m_work = false;
        }

        m_event_holder.notify_all();

        for (std::thread& thread : m_threads) {
            thread.join();
        }
    }

    template <typename FuncType, typename... Args>
    auto push_task(FuncType&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<FuncType>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();

        {
            std::lock_guard<std::mutex> lock(m_locker);
            if (!m_work) {
                throw std::runtime_error("push_task on stopped ThreadPool");
            }

            m_task_queue.emplace([task]() { (*task)(); });
        }

        m_event_holder.notify_one();

        return res;
    }

private:
    struct TaskWithPromise {
        task_type task;
        std::shared_ptr<std::promise<void>> prom;
    };

    std::vector<std::thread> m_threads;
    std::queue<TaskWithPromise> m_task_queue;
    std::mutex m_locker;
    std::condition_variable m_event_holder;
    volatile bool m_work;

    void threadFunc() {
        while (true) {
            TaskWithPromise task_to_do;
            {
                std::unique_lock<std::mutex> l(m_locker);
                if (m_task_queue.empty() && !m_work) {
                    return;
                }
                if (m_task_queue.empty()) {
                    m_event_holder.wait(l, [this] { return !m_task_queue.empty() || !m_work; });
                }
                if (!m_task_queue.empty()) {
                    task_to_do = std::move(m_task_queue.front());
                    m_task_queue.pop();
                }
            }
            if (task_to_do.task) {
                task_to_do.task();
                if (task_to_do.prom) {
                    task_to_do.prom->set_value();
                }
            }
        }
    }
};