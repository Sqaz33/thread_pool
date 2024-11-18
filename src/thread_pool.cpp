#include "../include/thread_pool.hpp"

namespace impl__ {

taskID initID() noexcept {
    return 0;
}
taskID getNextId(const taskID& ID) noexcept {
    return ID + 1;
}

}  // namespace impl__