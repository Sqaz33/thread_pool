#ifndef INCLUDE_THREAD_POOL_HPP
#define INCLUDE_THREAD_POOL_HPP

#include <any>
#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <latch>

#include "thread_queue.hpp"

namespace impl__ {
    template <typename T>
    class ThreadPool_impl;
}

namespace detail__ {

    template <typename T>
    void consumerWork(thread_queue::UnboundedThreadQueue& queue, std::latch& L) {
        thread_queue::task_t tsk;
        for (;;) {
            queue.popTask(tsk);
            if (tsk) std::exchange(tsk, nullptr)();
            else break;
        }
        L.count_down();
    }

} // namespace detail__


namespace impl__ {
    
    using taskID = size_t;

    taskID initID() noexcept;
    taskID getNextId(const taskID& ID) noexcept;   

    template <typename T>
    class ThreadPool_impl {

    public:
        template <typename F, typename... Args>
        taskID pushTask(F f, Args&&... args) {
            auto [tsk, fut] = thread_queue::createTask(std::move(f), std::forward<Args>(args)...);
            queue_.pushTask(std::move(tsk));
            auto id = moveID_();
            future_.emplace(id, std::move(fut));
            return id;
        } 

    protected:
        ThreadPool_impl(size_t threadCount) : threadsWaiter_(threadCount) {
            setConsumers_(threadCount);
            detachConsumers_();
        }

        ThreadPool_impl() : threadsWaiter_(recomendConsumersCount_()) {
            setConsumers_(recomendConsumersCount_());
            detachConsumers_();
        }

        ~ThreadPool_impl() {
            queue_.setDone(); 
            waitAllThreads_();
        }

    protected:
        std::unordered_map<taskID, std::future<T>> future_;
        
    private:
        std::vector<std::thread> consumers_;
        thread_queue::UnboundedThreadQueue queue_;
        taskID curTaskID_ = initID();
        std::latch threadsWaiter_;

    private:
        size_t minConsumersCount_() const noexcept { return 1; }
        size_t recomendConsumersCount_() const noexcept {
            return std::thread::hardware_concurrency() - 1;
        }

        taskID moveID_() {
            return std::exchange(curTaskID_, getNextId(curTaskID_));
        }

        void setConsumers_(size_t count) {
            for (int i = 0; i < count; ++i) {
                consumers_.emplace_back(detail__::consumerWork<T>, std::ref(queue_), std::ref(threadsWaiter_));
            }
        }

        void detachConsumers_() {
            for (auto& t : consumers_) {
                t.detach();
            }
        }

        void waitAllThreads_() {
            threadsWaiter_.wait();
        }
    };

} // namespase impr__


namespace thread_pool {
    
    using taskID = impl__::taskID;

    template <class T>
    class ThreadPool final : public impl__::ThreadPool_impl<T> {
    public:
        ThreadPool() = default;

        ThreadPool(size_t threadCount) : 
            impl__::ThreadPool_impl<T>(threadCount) 
        {}

        void waitNPopResult(const taskID& ID, T& res) {
            res = impl__::ThreadPool_impl<T>::future_[ID].get();
            impl__::ThreadPool_impl<T>::future_.erase(ID);
        } 
    };

    template <>
    class ThreadPool<void> final : public impl__::ThreadPool_impl<void> {
    public:
        ThreadPool() = default;

        ThreadPool(size_t threadCount) : 
            impl__::ThreadPool_impl<void>(threadCount) 
        {}

        void waitTask(const taskID& ID) {
            impl__::ThreadPool_impl<void>::future_[ID].get();
            impl__::ThreadPool_impl<void>::future_.erase(ID);
        } 
    };

} // namespace thread_pool


#endif // INCLUDE_THREAD_POOL_HPP