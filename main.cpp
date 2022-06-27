#include "ThreadPool.hpp"
#include <iostream>

int test(int a)
{
    return 2 * a;
}

int main()
{
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
        std::cout << "结果：" << i.get() << std::endl;
    }

    return 0;
}
