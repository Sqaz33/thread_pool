/**
 * @file queue_pool.hpp
 * @author Matveev S.A. (mset321@gmail.com)
 * 
 * @brief Provides an interface for creating and managing tasks in a multithreaded queue system.
 * 
 * @version 0.1
 * @date 2024-11-18
 * 
 * @copyright Copyright (c) 2024 Matveev S.A.
 * Licensed under the MIT License. See LICENSE file for details.
 * 
 * This file contains the declaration of the `UnboundedTaskQueue` class, which represents an unbounded queue
 * for handling tasks in a thread-safe manner. Additionally, it provides the `createTask` function template
 * for generating tasks that can be pushed into the queue.
 */

#ifndef INCLUDE_QUEUE_POOL_HPP
#define INCLUDE_QUEUE_POOL_HPP

#include <functional>
#include <future>
#include <queue>
#include <tuple>
#include <type_traits>
#include <utility>

namespace thread_queue {

/**
 * @typedef task_t
 * @brief Represents a move-only function type that returns an `int`.
 */
using task_t = std::move_only_function<int()>;

/**
 * @class UnboundedTaskQueue
 * @brief A thread-safe unbounded queue for managing tasks.
 *
 * The `UnboundedTaskQueue` class provides a lock-based mechanism for
 * pushing and popping tasks in a multithreaded environment. It ensures
 * thread safety by using a mutex and condition variable.
 */
class UnboundedTaskQueue {
   public:
    /**
     * @brief Pushes a new task into the queue.
     * @param tsk The task to be added to the queue.
     */
    void pushTask(task_t&& tsk);

    /**
     * @brief Pops a task from the queue.
     * @param tsk Reference to the task that will be set when popped.
     */
    void popTask(task_t& tsk);

    /**
     * @brief Marks the queue as done, indicating no more tasks will be added.
     */
    void setDone();

   private:
    std::queue<task_t> queue_;
    std::condition_variable prodCond_;
    std::mutex mut_;
    bool done_ = false;

   private:
    bool empty_() const noexcept;
    bool isDone_() const noexcept;
};

/**
 * @brief Creates a task that can be executed in a multithreaded environment.
 *
 * This function template takes a C++ function or callable object `F` as an argument
 * and binds any additional arguments passed to it. It returns a pair consisting of
 * a task ready to be executed and a future to obtain the result.
 *
 * @tparam F The type of the function to be invoked. Must be a C++ function.
 * @tparam Args The types of additional arguments to be passed to the function.
 * @param f A callable object (C++ function) that represents the task.
 * @param args Additional arguments to be passed to the function when it is executed.
 * @return A `std::pair` containing the created task (`task_t`) and a `std::future` for the result.
 *
 * @pre The template parameter `F` must be a valid C++ function.
 */
template <typename F, typename... Args>
auto createTask(F f, Args&&... args) {
    std::packaged_task<std::remove_pointer_t<F>> packagedF{f};
    auto fut = packagedF.get_future();
    task_t tsk = {[packagedF = std::move(packagedF),
                   args = std::make_tuple(args...)]() mutable {
        std::apply([packagedF = std::move(packagedF)](
                       auto&&... args) mutable { packagedF(args...); },
                   std::move(args));
        return 0;
    }};
    return std::make_pair(std::move(tsk), std::move(fut));
}

}  // namespace thread_queue

#endif  // INCLUDE_QUEUE_POOL_HPP
