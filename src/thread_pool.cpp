#include "../include/thread_pool.hpp"

#include <thread>
#include <utility>

namespace detail__ {

std::atomic_bool boolDropped_ = false;

void consumerWork(thread_pool::ThreadPool* threadPool) {
    thread_queue::task_t tsk;
    for (;;) {
        threadPool->queue_.popTask(tsk);
        if (boolDropped_)  break; 
        std::exchange(tsk, nullptr)();
    }
}

} // namespace detail__


namespace thread_pool {

taskID initID() noexcept { return 0; }
taskID getNextId(const taskID& ID) noexcept { return ID + 1; }


ThreadPool::ThreadPool() {
    setConsumers_(recomendConsumersCount_());
    detachConsumers_();
}     

ThreadPool::~ThreadPool() { 
    detail__::boolDropped_ = true;
    queue_.setDone(); 

}

size_t ThreadPool::minConsumersCount_() const noexcept { return 1; }
size_t ThreadPool::recomendConsumersCount_() const noexcept { 
    return std::thread::hardware_concurrency()  - 1;
}

void ThreadPool::setConsumers_(size_t count) {
    for (int i = 0; i < count; ++i) {
        consumers_.emplace_back(detail__::consumerWork, this);
    }
}
void ThreadPool::detachConsumers_() {
    for (auto& t : consumers_) {
        t.detach();
    }
}

taskID ThreadPool::moveID_() { 
    return std::exchange(curTaskID_, getNextId(curTaskID_));
}

template <typename F, typename... Args>
taskID ThreadPool::pushTask(F f, Args&&... args) {
    auto [tsk, fut] = thread_queue::createTask(std::move(f), std::forward<Args>(args)...);
    auto id = moveID_();
    future_[id] = std::move(fut);
    queue_.pushTask(std::move(tsk));
    return id;
}

template <typename T>
void ThreadPool::popResult(const taskID& ID, T& res) {
    auto fut = std::move(future_[ID]);
    res = std::move(fut.get());
    futer_.erase(ID);
} 

} // namespace thread_pool