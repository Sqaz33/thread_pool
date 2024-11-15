#include "../include/thread_queue.hpp"

#include <mutex>

namespace thread_queue {

void UnboundedThreadQueue::pushTask(task_t&& tsk) {
    std::lock_guard<std::mutex> lk{mut_};
    queue_.push(std::move(tsk));
    prodCond_.notify_one();
}

void UnboundedThreadQueue::popTask(task_t& tsk) {
    std::unique_lock<std::mutex> lk{mut_};
    prodCond_.wait(lk, [this] { return !empty_() || isDone_(); }); 
    if (isDone_()) return;
    tsk = std::move(queue_.front());
    queue_.pop();
}

void UnboundedThreadQueue::setDone() {
    done_ = true;
    prodCond_.notify_all();
}

bool UnboundedThreadQueue::empty_() const noexcept { return queue_.empty(); }
bool UnboundedThreadQueue::isDone_() const noexcept { return done_; }

} // namespace thread_queue