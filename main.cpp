#include "ThreadPool.hpp"
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtxCout;

int test(int a)
{
    std::chrono::milliseconds waitTime(500 - a * 10);
    std::this_thread::sleep_for(waitTime);
    std::unique_lock<std::mutex> lock(mtxCout);
    std::cout << "当前值为：" << a << std::endl;
    return 2 * a;
}

int main()
{
    auto startTime = std::chrono::system_clock::now();
    size_t sys_thread_size = std::thread::hardware_concurrency();
    zwn::ThreadPool pool(sys_thread_size);
    std::cout << "线程池大小：" << pool.size() << std::endl;

    pool.start();

    std::vector<std::future<int>> resVec;
    for (int i = 0; i < 32; ++i) {
        auto res = pool.addTask(test, i);
        resVec.push_back(std::move(res));
    }

    for (auto& i : resVec) {
        i.wait();
    }

    std::cout << "输出结果：" << std::endl;
    for (auto& i : resVec) {
        std::cout << i.get() << std::endl;
    }

    auto endTime = std::chrono::system_clock::now();
    auto totalTime = endTime - startTime;
    long long int val = std::chrono::duration_cast<std::chrono::milliseconds>(totalTime).count();
    std::cout << "总用时：" << val << "ms" << std::endl;
    return 0;
}
