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

    /**
     * @brief 启动线程池
     *
     */
    void start();

    /**
     * @brief 关闭线程池
     *
     */
    void close();

    /**
     * @brief 向线程池中添加任务，不限制函数参数和返回值类型
     *
     * @tparam F 转发一个函数
     * @tparam Args 转发函数参数
     * @param func 待执行的函数
     * @param args 函数参数
     * @return 包裹函数返回值的future
     */
    template <class F, class... Args>
    auto addTask(F&& func, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

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

    /**
     * @brief 线程循环，不断从任务队列中取任务执行
     *
     */
    void loop();
};

// ============================ 实现-Implement ================================

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
    close();
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
        // 创建线程
        // C++中，线程创建即执行
        _threads.emplace_back(&ThreadPool::loop, this);
    }
}

void ThreadPool::close()
{
    //首先将运行状态设置为false
    _is_run = false;
    // 同知所有的正在等待的线程，运行状态发生变化
    _cond.notify_all();
    // 等待剩余线程执行完毕
    for (auto& t : _threads) {
        t.join();
    }
}

// C++17前可以使用注释内的返回值类型
// C++17后result_of被废除
// std::future<typename std::result_of<F(Args...)>::type>
template <class F, class... Args>
auto ThreadPool::addTask(F&& func, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>
{
    // 包裹用户传进来的函数
    using RtnType = typename std::invoke_result<F, Args...>::type;

    // std::packaged_task无法被复制，为了方便lambda捕获，所以使用shared_ptr
    std::shared_ptr<std::packaged_task<RtnType()>> usrTask = std::make_shared<std::packaged_task<RtnType()>>(
        std::bind(std::forward<F>(func), std::forward<Args>(args)...));

    // 准备包裹返回值
    std::future<RtnType> res = usrTask->get_future();

    // 添加到队列
    std::unique_lock<std::mutex> lock(_mtx);
    _tasks.emplace(
        [usrTask]() {
            (*usrTask)();
        });
    lock.unlock();

    // 通知等待的线程
    _cond.notify_one();

    // 返回包裹返回值的future
    return res;
}

void ThreadPool::loop()
{
    // 线程循环
    // 不断取任务执行
    while (_is_run) {
        // 对任务队列加锁
        std::unique_lock<std::mutex> lock(_mtx);
        // 如果任务队列一直为空，则等待
        // 如果后面函数返回true，则停止等待
        while (_tasks.empty()) {
            // 等待时会自动解锁
            _cond.wait(lock, [this]() -> bool {
                // 如果运行状态为false或任务队列不为空，则停止等待
                return !this->_is_run || !this->_tasks.empty();
            });
            // 如果线程池已经关闭则退出线程
            if (!_is_run) {
                return;
            }
        }

        //取出一个任务
        std::function<void()> task = _tasks.front();
        _tasks.pop();
        // 任务队列解锁
        lock.unlock();

        // 执行任务
        task();
    }
}

} // namespace zwn

#endif // _THREAD_POOL_HPP_
