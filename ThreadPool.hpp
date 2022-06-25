/**
 * @file ThreadPool.hpp
 * @author zwmain(zwmain@outlook.com)
 * @brief 基于C++17的线程池 -- Thread pool based on C++17
 * @version 1.0
 * @date 2022-06-25
 *
 * @copyright Copyright (c) 2022 zwmain
 *
 */

#ifndef _THREAD_POOL_HPP_
#define _THREAD_POOL_HPP_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace zwn {

/**
 * @brief 线程池类，固定大小
 */
class ThreadPool {
public:
    /**
     * @brief 默认构造函数
     *
     */
    ThreadPool();

    /**
     * @brief 构造函数
     *
     * @param pool_size 线程池大小
     */
    explicit ThreadPool(size_t pool_size);

    /**
     * @brief 析构函数
     *
     */
    ~ThreadPool();

    /**
     * @brief 获取线程池大小
     *
     * @return 线程池大小
     */
    size_t size() const;

    /**
     * @brief 获取正在等待的任务队列大小
     *
     * @return 任务队列大小
     */
    size_t taskQueueSize() const;

    void start();

    void close();

    template <class F, class... Args>
    auto addTask(F&& func, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>>;

private:
    // 线程数组
    std::vector<std::thread> _threads;
    // 任务队列
    std::queue<std::function<void()>> _tasks;
    // 线程池大小
    const size_t _pool_size = 0;
    // 线程是否在运行
    std::atomic<bool> _is_run = false;
    // 任务队列互斥量
    mutable std::mutex _mtx;
    // 条件变量
    std::condition_variable _cond;

    void loop();
};

ThreadPool::ThreadPool()
    : _pool_size(std::thread::hardware_concurrency())
{
}

ThreadPool::ThreadPool(size_t pool_size)
    : _pool_size(pool_size)
{
}

ThreadPool::~ThreadPool()
{
}

size_t ThreadPool::size() const
{
    return _pool_size;
}

size_t ThreadPool::taskQueueSize() const
{
    std::unique_lock<std::mutex> lock(_mtx);
    return _tasks.size();
}

void ThreadPool::start()
{
    _is_run = true;
    for (size_t i = 0; i < _pool_size; ++i) {
        _threads.emplace_back(ThreadPool::loop, this);
    }
}

void ThreadPool::loop()
{
    while (_is_run) {
        // todo
    }
}

} // namespace zwn

#endif // _THREAD_POOL_HPP_
