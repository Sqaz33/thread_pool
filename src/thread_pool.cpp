#include "../include/thread_pool.hpp"

#include <thread>
#include <utility>


namespace thread_pool {

taskID initID() noexcept { return 0; }
taskID getNextId(const taskID& ID) noexcept { return ID + 1; }

} // namespace thread_pool