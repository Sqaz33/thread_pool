#include "../include/thread_pool.hpp"

namespace task_id {

taskID_t initID() noexcept {
    return 0;
}
taskID_t getNextId(const taskID_t& ID) noexcept {
    return ID + 1;
}

}  // namespace task_id
