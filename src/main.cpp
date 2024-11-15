#include <iostream>
#include <functional>
#include <future>

#include "../include/thread_pool.hpp"
#include <future>

int sum(int a, int b) {
    return a + b;
}



int main() {
    thread_pool::ThreadPool<int> threadPool;

    auto id = threadPool.pushTask(sum, 1, 2);
    int res = 0;
    threadPool.popResult(id, res);
    std::cout << res << '\n';
}