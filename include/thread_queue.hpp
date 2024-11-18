#ifndef INCLUDE_QUEUE_POOL_HPP
#define INCLUDE_QUEUE_POOL_HPP

#include <functional>
#include <future>
#include <queue>
#include <tuple>
#include <type_traits>
#include <utility>

namespace thread_queue {

using task_t = std::move_only_function<int()>;

// lock based queue
class UnboundedThreadQueue {
   public:
    void pushTask(task_t&& tsk);
    void popTask(task_t& tsk);
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