/**
 * @file thread_pool.hpp
 * @author Matveev S.A. (mset321@gmail.com)
 * @brief Header file defining a thread pool class for concurrent task execution.
 * @version 0.1
 * @date 2024-11-18
 * 
 * @copyright Copyright (c) 2024 Matveev S.A.
 * Licensed under the MIT License. See LICENSE file for details.
 */

#ifndef INCLUDE_THREAD_POOL_HPP
#define INCLUDE_THREAD_POOL_HPP

#include <latch>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "thread_queue.hpp"

namespace impl__ {
template <typename T>
class ThreadPool_impl;
}

namespace detail__ {

template <typename T>
void consumerWork(thread_queue::UnboundedTaskQueue& queue, std::latch& L) {
    thread_queue::task_t tsk;
    for (;;) {
        queue.popTask(tsk);
        if (tsk)
            std::exchange(tsk, nullptr)();
        else
            break;
    }
    L.count_down();
}

}  // namespace detail__

namespace task_id {
using taskID_t = size_t;
taskID_t initID() noexcept;
taskID_t getNextId(const taskID_t& ID) noexcept;
}  // namespace task_id

namespace impl__ {
template <typename T>
class ThreadPool_impl {

   public:
    /**
     * @brief Pushes a task into the thread pool and returns a task ID.
     * 
     * @tparam F Callable type for the task function.
     * @tparam Args Types of the arguments to be passed to the task function.
     * @param f The task function.
     * @param args Arguments to be passed to the task function.
     * @return task_id::taskID_t Unique identifier for the submitted task.
     */
    template <typename F, typename... Args>
    task_id::taskID_t pushTask(F f, Args&&... args) {
        auto [tsk, fut] =
            thread_queue::createTask(std::move(f), std::forward<Args>(args)...);
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
    std::unordered_map<task_id::taskID_t, std::future<T>> future_;

   private:
    std::vector<std::thread> consumers_;
    thread_queue::UnboundedTaskQueue queue_;
    task_id::taskID_t curtask_id = task_id::initID();
    std::latch threadsWaiter_;

   private:
    size_t minConsumersCount_() const noexcept { return 1; }
    size_t recomendConsumersCount_() const noexcept {
        return std::thread::hardware_concurrency() - 1;
    }

    task_id::taskID_t moveID_() {
        return std::exchange(curtask_id, task_id::getNextId(curtask_id));
    }

    void setConsumers_(size_t count) {
        for (int i = 0; i < count; ++i) {
            consumers_.emplace_back(detail__::consumerWork<T>, std::ref(queue_),
                                    std::ref(threadsWaiter_));
        }
    }

    void detachConsumers_() {
        for (auto& t : consumers_) {
            t.detach();
        }
    }

    void waitAllThreads_() { threadsWaiter_.wait(); }
};

}  // namespace impl__

namespace thread_pool {

/**
 * @brief A class providing an interface for executing tasks in a multithreaded environment.
 * 
 * @tparam T The value returned by the task.
 */
template <class T>
class ThreadPool final : public impl__::ThreadPool_impl<T> {
   public:
    /**
     * @brief Default constructor for the ThreadPool class specialized for void tasks.
     */
    ThreadPool() = default;

    /**
     * @brief Constructs a thread pool for void tasks with a specified number of threads.
     * 
     * @param threadCount Number of threads to initialize in the pool.
     */
    ThreadPool(size_t threadCount) : impl__::ThreadPool_impl<T>(threadCount) {}

    /**
     * @brief Waits for a specific task to complete and retrieves its result.
     * 
     * @tparam T The type of the result produced by the task.
     * @param ID The unique identifier of the task.
     * @param res Reference to store the result of the completed task.
     */
    void waitNPopResult(const task_id::taskID_t& ID, T& res) {
        res = impl__::ThreadPool_impl<T>::future_[ID].get();
        impl__::ThreadPool_impl<T>::future_.erase(ID);
    }
};

/**
 * @brief Specialization of ThreadPool for void.
 * Implies that the task does not return a value.
 * 
 * @tparam void
 */
template <>
class ThreadPool<void> final : public impl__::ThreadPool_impl<void> {
   public:
    /**
     * @brief Default constructor for the ThreadPool class specialized for void tasks.
     */
    ThreadPool() = default;

    /**
     * @brief Constructs a thread pool for void tasks with a specified number of threads.
     * 
     * @param threadCount Number of threads to initialize in the pool.
     */
    ThreadPool(size_t threadCount)
        : impl__::ThreadPool_impl<void>(threadCount) {}

    /**
     * @brief Waits for a specific void task to complete.
     * 
     * @param ID The unique identifier of the task.
     */
    void waitTask(const task_id::taskID_t& ID) {
        impl__::ThreadPool_impl<void>::future_[ID].get();
        impl__::ThreadPool_impl<void>::future_.erase(ID);
    }
};

}  // namespace thread_pool

#endif  // INCLUDE_THREAD_POOL_HPP