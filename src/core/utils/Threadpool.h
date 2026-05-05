#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <emmintrin.h>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
class ThreadPool
{
  private:
    std::vector<std::thread> m_workers;
    std::function<void(size_t)> m_task;

    std::condition_variable m_cv;
    std::mutex m_mutex;

    std::atomic<int> m_remaining{0};
    std::atomic<size_t> m_nextWorkerIndex{0};

    uint64_t m_generation = 0;
    bool m_shutDown = false;

    void WorkerLoop()
    {
        size_t myIndex = m_nextWorkerIndex.fetch_add(1) + 1;
        uint64_t myGen = 0;

        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv.wait(lock, [this, &myGen]() { return m_generation > myGen || m_shutDown; });
                myGen = m_generation;
            }

            if (m_shutDown) return;

            m_task(myIndex);
            m_remaining.fetch_sub(1, std::memory_order_release);
        }
    }

  public:
    explicit ThreadPool(size_t threadCount)
    {
        m_workers.reserve(threadCount);
        for (size_t i = 0; i < threadCount; ++i)
        {
            m_workers.emplace_back([this]() { WorkerLoop(); });
        }
    }

    ~ThreadPool()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_shutDown = true;
        }
        m_cv.notify_all();

        for (auto& worker : m_workers)
        {
            worker.join();
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadPool(const ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&&) = delete;

    size_t ThreadCount() const
    {
        return m_workers.size();
    }

    template <typename Func>
    void Dispatch(size_t taskCount, Func&& func)
    {
        m_remaining.store((int)taskCount - 1, std::memory_order_release);
        m_task = std::forward<Func>(func);

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            ++m_generation;
        }
        m_cv.notify_all();

        func(0);

        while (m_remaining.load(std::memory_order_acquire) > 0)
        {
            _mm_pause();
        }
    }
};
