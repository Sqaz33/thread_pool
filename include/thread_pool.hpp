#ifndef INCLUDE_THREAD_POOL_HPP
#define INCLUDE_THREAD_POOL_HPP

#include <any>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <queue>

#include "thread_queue.hpp"

namespace thread_pool {
    class ThreadPool;
}

namespace detail__ {

    void consumerWork(thread_pool::ThreadPool*);

} // namespace detail__

namespace thread_pool {

    using taskID = size_t;

    taskID initID() noexcept;
    taskID getNextId(const taskID& ID) noexcept;
    
    
    class ThreadPool {
    public:
        ThreadPool();

        ~ThreadPool();

        template <typename F, typename... Args>
        taskID pushTask(F f, Args&&... args);

        template <typename T>
        void popResult(const taskID& ID, T& res);
    
    private:

        std::vector<std::thread> consumers_;
        std::unordered_map<taskID, std::any> future_;
        thread_queue::UnboundedThreadQueue queue_;
        taskID curTaskID_ = initID();
    
    private:
        friend void detail__::consumerWork(ThreadPool*);

        void setConsumers_(size_t count);

        void detachConsumers_();

        size_t minConsumersCount_() const noexcept;
        size_t recomendConsumersCount_() const noexcept;

        taskID moveID_();
    };

} // namespace thread_pool


#endif // INCLUDE_THREAD_POOL_HPP