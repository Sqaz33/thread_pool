#ifndef INCLUDE_QUEUE_POOL_HPP
#define INCLUDE_QUEUE_POOL_HPP

#include <functional>
#include <tuple>
#include <type_traits>
#include <future>
#include <utility>
#include <queue>

namespace thread_queue {

    using task_t = std::move_only_function<int()>;

    // lock based queue
    class UnboundedThreadQueue {

    private:
        std::queue<task_t> queue_;
        
    };

    template <typename F, typename... Args>
    auto createTask(F f, Args&&... args) {
        std::packaged_task<std::remove_pointer_t<F>> pf(f);
        auto fut = pf.get_future();
        task_t tsk = {
            [f = std::move(pf), args = std::make_tuple(args...)]() {
                std::apply(
                    [f = std::move(f)](Args&&... args) { f(args...); },
                    std::move(args)
                ),
                return 0;
            }
        };
        return std::make_pair(std::move(tsk), std::move(fut));
    }


} // namespace thread_queue


#endif // INCLUDE_QUEUE_POOL_HPP