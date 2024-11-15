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
    template <typename T>
    class ThreadPool;
}

namespace detail__ {
    inline std::atomic_bool boolDropped_ = false;

    template <typename T>
    void consumerWork(thread_pool::ThreadPool<T>* threadPool) {
        thread_queue::task_t tsk;
        for (;;) {
            threadPool->queue_.popTask(tsk);
            if (boolDropped_)  break; 
            std::exchange(tsk, nullptr)();
        }
    }

} // namespace detail__

namespace thread_pool {

    using taskID = size_t;

    taskID initID() noexcept;
    taskID getNextId(const taskID& ID) noexcept;
    

    template <typename T>
    class ThreadPool {
    public:
        ThreadPool() {
            setConsumers_(recomendConsumersCount_());
            detachConsumers_();
        }

        ~ThreadPool() {
            detail__::boolDropped_ = true;
            queue_.setDone(); 
        }

        template <typename F, typename... Args>
        taskID pushTask(F f, Args&&... args) {
            auto [tsk, fut] = thread_queue::createTask(std::move(f), std::forward<Args>(args)...);
            queue_.pushTask(std::move(tsk));
            auto id = moveID_();
            future_.emplace(id, std::move(fut));
            return id;
        }

        void popResult(const taskID& ID, T& res) {
            res = future_[ID].get();
            future_.erase(ID);
        } 
    
    private:

        std::vector<std::thread> consumers_;
        std::unordered_map<taskID, std::future<int>> future_;
        thread_queue::UnboundedThreadQueue queue_;
        taskID curTaskID_ = initID();
    
    private:
        friend void detail__::consumerWork<T>(ThreadPool<T>*);

        void setConsumers_(size_t count) {
            for (int i = 0; i < count; ++i) {
                consumers_.emplace_back(detail__::consumerWork<T>, this);
            }
        }

        void detachConsumers_() {
            for (auto& t : consumers_) {
                t.detach();
            }
        }

        size_t minConsumersCount_() const noexcept { return 1; }
        size_t recomendConsumersCount_() const noexcept {
            return std::thread::hardware_concurrency()  - 1;
        }

        taskID moveID_() {
            return std::exchange(curTaskID_, getNextId(curTaskID_));
        }
    };

} // namespace thread_pool


#endif // INCLUDE_THREAD_POOL_HPP